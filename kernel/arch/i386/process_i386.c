#include <kernel/process.h>
#include <kernel/filesystem.h>
#include <kernel/memory.h>

#include <stdlib.h>
#include <string.h>

#include "memory_i386.h"
#include "process_i386.h"

void free_userspace(proc_data_t *proc)
{
	pde_t *pd_temp = (pde_t*) temp_map(proc->pd_physical_addr, 0); 
	
	for (int i = 0 ; i < FIRST_KERNEL_PDE ; i++)
	{
		if (!pde_present(pd_temp))
			continue;
		
		pte_t *pt_temp = (pte_t*) temp_map(pde_addr(pd_temp), 1);

		for (int j = 0 ; j < NB_PTE ; j++)
		{
			if (pte_present(pt_temp))
				free_physical_page(pte_addr(pt_temp));
			pt_temp++;
		}
		free_physical_page(pde_addr(pd_temp));
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
	
	data->kernel_stack_addr = (stackint_t*) (malloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE);

	pde_t *pd_data = (pde_t*) temp_map(data->pd_physical_addr, 0);

	memset(pd_data, 0, PD_SIZE);
	for (int i = FIRST_KERNEL_PDE ; i < NB_PDE-1 ; i++)
		pd_data[i] = (cur_pd_addr())[i];
	
	pd_rec_map(pd_data, data->pd_physical_addr);

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

	make_runnable(pid);
}

proc_data_t *fork_proc_data(proc_data_t *src)
{
	proc_data_t *dst;
	
	if (!(dst = new_proc_data()))
		return 0;

	memcpy(((uint8_t*)dst->kernel_stack_addr)-KERNEL_STACK_SIZE, ((uint8_t*)src->kernel_stack_addr)-KERNEL_STACK_SIZE, KERNEL_STACK_SIZE);

	pde_t *pd_src = (pde_t*) temp_map(src->pd_physical_addr, 0);
	pde_t *pd_dst = (pde_t*) temp_map(dst->pd_physical_addr, 1);
	memset(pd_dst, 0, PD_SIZE);

	for (int i = 0 ; i < FIRST_KERNEL_PDE ; i++)
	{
		if (!pde_present(pd_src))
			continue;

		paddr_t page = alloc_physical_page();
		if (!page)
		{
			free_proc_data(dst);
			return 0;
		}

		*pd_dst = *pd_src;
		pde_set_addr(pd_dst, page);

		pte_t *pt_src = (pte_t*) temp_map(pde_addr(pd_src), 2);
		pte_t *pt_dst = (pte_t*) temp_map(page, 3);
		memset(pt_dst, 0, PT_SIZE);

		for (int j = 0 ; j < NB_PTE ; j++)
		{
			if (!pte_present(pt_src))
				continue;

			page = alloc_physical_page();
			if (!page)
			{
				free_proc_data(dst);
				return 0;
			}
			*pt_dst = *pt_src;
			pte_set_addr(pt_dst, page);
			
			memcpy((void*) temp_map(page, 5), (void*) temp_map(pte_addr(pt_src),4), PAGE_SIZE);
			pt_src++;
			pt_dst++;
		}

		pd_src++;
		pd_dst++;
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
			*((int32_t*)eax) = syscall_write(*ebx, (void*) *ecx, *((int32_t*)edx));
			break;
		case Syscall_Read:
			*((int32_t*)eax) = syscall_read(*ebx, (void*) *ecx, *((int32_t*)edx));
			break;
		case Syscall_Seek:
			*((uint32_t*)eax) = syscall_seek(*ebx, *((int32_t*)ecx), *edx);
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

