#include <kernel/process.h>
#include <kernel/filesystem.h>

#include <stdlib.h>
#include <string.h>

#include "memory_i386.h"
#include "process_i386.h"

void free_userspace(proc_data_t *proc)
{
	uint32_t *pd_temp = &temp_page_1;
	add_pt_entry(pd_temp, proc->pd_physical_addr);
	tlb_flush();
	for (int i = 0 ; i < 768 ; i++)
	{
		if (!(pd_temp[i] & 1))
			continue;
		
		uint32_t *pt_temp = &temp_page_2;
		add_pt_entry(pt_temp, pd_temp[i] & 0xFFFFF000);
		tlb_flush();

		for (int j = 0 ; j < 1024 ; j++)
		{
			if (pt_temp[j] & 1)
				free_physical_page(pt_temp[j] & 0xFFFFF000);
		}
		free_physical_page(pd_temp[i] & 0xFFFFF000);
	}
}

proc_data_t *new_proc_data()
{
	proc_data_t *data = (proc_data_t*) malloc(sizeof(proc_data_t));
	
	if (!(data->pd_physical_addr = alloc_physical_page()))
	{
		free(data);
		return 0;
	}
	
	data->kernel_stack_addr = (uint32_t*) (malloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE);
	
	uint32_t *pd_data = &temp_page_1;
	add_pt_entry(pd_data, data->pd_physical_addr);
	tlb_flush();

	memset(pd_data, 0, 4096);
	for (int i = 768 ; i < 1023 ; i++)
		pd_data[i] = (PD_ADDR)[i];
	pd_data[1023] = data->pd_physical_addr | 0x3;

	return data;
}

void kernel_proc(int (*start)(void*), void *arg)
{
	proc_data_t *proc = new_proc_data();
	int pid = new_pid();

	proc_list[pid] = (process_t*) malloc(sizeof(process_t));
	proc_list[pid]->parent_pid = (cur_pid == -1) ? pid : cur_pid;

	proc_list[pid]->priority =	0;
	proc_list[pid]->state = Runnable;
	proc_list[pid]->files = create_dynarray();
	proc_list[pid]->arch_data = proc;

	proc->eip = (uint32_t) kernel_proc_start;
	proc->esp = (uint32_t) (proc->kernel_stack_addr-2);
	proc->kernel_stack_addr[-1] = (uint32_t) arg;
	proc->kernel_stack_addr[-2] = (uint32_t) start;
}

proc_data_t *fork_proc_data(proc_data_t *src)
{
	proc_data_t *dst;
	
	if (!(dst = new_proc_data()))
		return 0;

	memcpy(((uint8_t*)dst->kernel_stack_addr)-KERNEL_STACK_SIZE, ((uint8_t*)src->kernel_stack_addr)-KERNEL_STACK_SIZE, KERNEL_STACK_SIZE);

	uint32_t *pd_src = &temp_page_1;
	uint32_t *pd_dst = &temp_page_2;
	add_pt_entry(pd_src, src->pd_physical_addr);
	add_pt_entry(pd_dst, dst->pd_physical_addr);
	tlb_flush();

	for (int i = 0 ; i < 768 ; i++)
	{
		if (!(pd_src[i] & 1))
			continue;
		uint32_t page = alloc_physical_page();
		if (!page)
		{
			free_proc_data(dst);
			return 0;
		}
		pd_dst[i] = (pd_src[i]&0xFFF) | (page & 0xFFFFF000);

		uint32_t *pt_src = &temp_page_3;
		uint32_t *pt_dst = &temp_page_4;
		add_pt_entry(pt_src, pd_src[i] & 0xFFFFF000);
		add_pt_entry(pt_dst, page);
		tlb_flush();
		memset(pt_dst, 0, 4096);

		for (int j = 0 ; j < 1024 ; j++)
		{
			if (!(pt_src[j] & 1))
				continue;

			page = alloc_physical_page();
			if (!page)
			{
				free_proc_data(dst);
				return 0;
			}
			pt_dst[j] = (pt_src[j]&0xFFF) | (page & 0xFFFFF000);
			
			uint32_t *page_src = &temp_page_5;
			uint32_t *page_dst = &temp_page_6;
			add_pt_entry(page_src, pt_src[j] & 0xFFFFF000);
			add_pt_entry(page_dst, page);
			tlb_flush();
			memcpy(page_dst, page_src, 4096);
		}
	}

	return dst;
}

void free_proc_data(proc_data_t *proc)
{
	free_userspace(proc);
	free_physical_page(proc->pd_physical_addr);
	
	free(((void*)proc->kernel_stack_addr)-KERNEL_STACK_SIZE);
	free(proc);
}

void syscall_handler(uint32_t *regs)
{
	int *eax = (int*) (regs+PUSHA_EAX);
	int *ebx = (int*) (regs+PUSHA_EBX);
	int *ecx = (int*) (regs+PUSHA_ECX);
	int *edx = (int*) (regs+PUSHA_EDX);

	switch (*eax)
	{
		case Syscall_Wait:
			*eax = syscall_wait(ebx, ecx);
			while (*eax < 0)
			{
				reschedule();
				*eax = syscall_wait(ebx, ecx);
			}
			break;
		case Syscall_Fork:
			*eax = syscall_fork();
			break;
		case Syscall_Exit:
			syscall_exit(*ebx);
			reschedule();
			break;
		case Syscall_Open:
			*eax = syscall_open((const char*) *ebx);
			break;
		case Syscall_Write:
			*((size_t*)eax) = syscall_write(*ebx, (void*) *ecx, *edx);
			break;
		case Syscall_Read:
			*((size_t*)eax) = syscall_read(*ebx, (void*) *ecx, *edx);
			break;
		case Syscall_Seek:
			*((uint32_t*)eax) = syscall_seek(*ebx, *ecx, *edx);
			break;
		case Syscall_Close:
			syscall_close(*ebx);
			break;
		default:
			*eax = -1;
	}
}

void scheduler_tick()
{
 	if (!scheduling_on)
		return;

	if (!(--proc_list[cur_pid]->ms_left))
		reschedule();
}

void reschedule()
{
	int prev_pid = cur_pid;
	schedule();
	
	if (prev_pid == cur_pid)
		return;

	proc_data_t *next = proc_list[cur_pid]->arch_data;

	load_pd(next->pd_physical_addr);
	set_tss_stack(next->kernel_stack_addr);
	
	if (prev_pid == -1)
		switch_initial(next);
	else
		switch_proc(proc_list[prev_pid]->arch_data, next);
}

void syscall_exec(const char *path)
{
	file_descr_t *fd = open_rd(path);
	if (!fd)
		return;
	
	/* TODO : parse ELF */
}

