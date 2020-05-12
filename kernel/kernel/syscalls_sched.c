#include <kernel/syscalls.h>

#include <kernel/process.h>
#include <kernel/memory.h>

#include <stdlib.h>
#include <string.h>

/* Implémentation des appels liés à l'ordonnanceur : wait, fork, exit */

int syscall_wait(int *pid, int *code)
{
	int children = 1;
	while (children)
	{
		children = 0;
		for (int i = 0 ; i < NB_MAX_PROC ; i++)
		{
			if (i != cur_pid && proc_list[i] != 0 && proc_list[i]->parent_pid == cur_pid)
			{
				children++;
				if (proc_list[i]->state == Zombie)
				{
					*pid = i;
					*code = proc_list[i]->exit_code;
					free_proc(proc_list[i]);
					proc_list[i] = 0;
					return 1;
				}
			}
		}
		if (children)
		{
			proc_list[cur_pid]->state = Waiting;
			reschedule();
		}
	}
	return 0;
}

int syscall_fork()
{
	int child_pid = new_pid();
	if (child_pid < 0)
		return child_pid;

	process_t *p = new_proc();
	if (!p)
		return -1;
	
	process_t *forked = proc_list[cur_pid];

	free(p->cwd);
	if (!(p->cwd = (char*) calloc(strlen(forked->cwd),sizeof(char))))
	{
		free_proc(p);
		return -1;
	}
	strcpy(p->cwd, forked->cwd);

	memcpy(((uint8_t*)p->kernel_stack)-KERNEL_STACK_SIZE, ((uint8_t*)forked->kernel_stack)-KERNEL_STACK_SIZE, KERNEL_STACK_SIZE);

	pde_t *pd_src = (pde_t*) temp_map(forked->pd_addr, 0);
	pde_t *pd_dst = (pde_t*) temp_map(p->pd_addr, 1);
	memset(pd_dst, 0, PD_SIZE*FIRST_KERNEL_PDE/NB_PDE);

	for (int i = 0 ; i < FIRST_KERNEL_PDE ; i++, pd_src++, pd_dst++ )
	{
		if (!pde_is_present(pd_src))
			continue;

		paddr_t page = alloc_physical_page();
		if (!page)
		{
			free_proc(p);
			return -1;
		}

		*pd_dst = *pd_src;
		pde_set_addr(pd_dst, page);

		pte_t *pt_src = (pte_t*) temp_map(pde_addr(pd_src), 2);
		pte_t *pt_dst = (pte_t*) temp_map(page, 3);
		memset(pt_dst, 0, PT_SIZE);

		for (int j = 0 ; j < NB_PTE ; j++, pt_src++, pt_dst++)
		{
			if (!pte_is_present(pt_src))
				continue;

			page = alloc_physical_page();
			if (!page)
			{
				free_proc(p);
				return -1;
			}
			*pt_dst = *pt_src;
			pte_set_addr(pt_dst, page);
			
			memcpy((void*) temp_map(page, 5), (void*) temp_map(pte_addr(pt_src),4), PAGE_SIZE);
		}
	}

	p->parent_pid = cur_pid;
	p->priority = forked->priority;
	
	p->code_begin = forked->code_begin;
	p->code_end = forked->code_end;
	p->heap_begin = forked->heap_begin;
	p->heap_end = forked->heap_end;
	p->data_begin = forked->data_begin;
	p->data_end = forked->data_end;
	p->stack_top = forked->stack_top;
	p->stack_bottom = forked->stack_bottom;

	for (int i = 0 ; i < forked->files->size ; i++)
	{
		file_descr_t *fd = (file_descr_t*) forked->files->array[i];
		if (fd)
			fd->owners++;
		dynarray_push(p->files, fd); // TODO : s'assurer qu'il ne manque pas de mémoire
	}

	proc_list[child_pid] = p;
	setup_fork_switch(forked, p);
	make_runnable(child_pid);

	return child_pid;
}

void syscall_exit(int code)
{
	proc_list[cur_pid]->exit_code = code;
	proc_list[cur_pid]->state = Zombie;

	for (int i = 0 ; i < NB_MAX_PROC ; i++)
	{
		if (proc_list[i] && i != cur_pid && proc_list[i]->parent_pid == cur_pid)
			proc_list[i]->parent_pid = ZOMBIE_SLAYER_PID; 
	}

	int parent = proc_list[cur_pid]->parent_pid;
	if (proc_list[parent]->state == Waiting)
		make_runnable(parent);

	reschedule();
}

