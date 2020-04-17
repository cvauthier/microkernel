#include <kernel/process.h>

#include "memory_i386.h"
#include "process_i386.h"

proc_data_t *fork_proc_data(proc_data_t *src, int child_pid)
{
	proc_data_t *dst = (proc_data_t*) malloc(sizeof(proc_data_t));
	
	if (!(dst->pd_physical_addr = alloc_physical_page()))
	{
		free(dst);
		return 0;
	}
	
	dst->kernel_stack_addr = (uint32_t*) (malloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE);

	uint32_t *pd_src = &temp_page_1;
	uint32_t *pd_dst = &temp_page_2;
	add_pt_entry(pd_src, src->pd_physical_addr);
	add_pt_entry(pd_dst, dst->pd_physical_addr);
	memset(pd_dst, 0, 4096);

	for (int i = 0 ; i < 1024 ; i++)
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
			memcpy(page_dst, page_src, 4096);
		}
	}

	return dst;
}

void wait_end_proc_data(proc_data_t *data, int child_pid)
{
	int ofs = (data->kernel_stack_addr[-1] == 0x23) ? 0 : STACK_R0_OFS;
	data->kernel_stack_addr[STACK_EAX] = child_pid;
}

void free_proc_data(proc_data_t *proc)
{
	uint32_t *pd_temp = &temp_page_1;
	add_pt_entry(pd_temp, proc->pd_physical_addr);
	for (int i = 0 ; i < 1024 ; i++)
	{
		if (!(pd_temp[i] & 1))
			continue;
		
		uint32_t *pt_temp = &temp_page_2;
		add_pt_entry(pt_temp, pd_temp[i] & 0xFFFFF000);

		for (int j = 0 ; j < 1024 ; j++)
		{
			if (pt_temp[j] & 1)
				free_physical_page(pt_temp[j] & 0xFFFFF000);
		}
		free_physical_page(pd_temp[i] & 0xFFFFF000);
	}
	free_physical_page(proc->pd_physical_addr);
	
	free(((void*)proc->kernel_stack_addr)-KERNEL_STACK_SIZE);
	free(proc);
}

void syscall_handler()
{
	proc_data_t *data = proc_list[cur_pid]->arch_data;
	uint32_t *stack = data6>kernel_stack_addr;
	int ofs = (stack[-1] == 0x23) ? 0 : STACK_R0_OFS; 

	int *eax = (int*) (data->kernel_stack_addr+STACK_EAX+ofs);
	int resched = 0;

	switch (*eax)
	{
		case Wait:
			*eax = syscall_wait();
			if (*eax < 0)
				resched = 1;
			break;
		case Fork:
			*eax = syscall_fork();	
			break;
		case Exit:
			syscall_exit(data->kernel_stack_addr[STACK_EBX+ofs]);
			resched = 1;
			break;
		default:
			*eax = -1;
	}

	if (resched)
		reschedule();
}

void reschedule()
{
	//int prev_pid = cur_pid;
	schedule();

	uint32_t *stack = proc_list[cur_pid]->arch_data->kernel_stack_addr;
	int ofs = (stack[-1] == 0x23) ? 0 : STACK_R0_OFS;

	load_pd(proc_list[cur_pid]->arch_data->pd_physical_addr);
	set_tss_stack(stack);
	jump_user_proc(stack+STACK_BOTTOM+ofs);
}


