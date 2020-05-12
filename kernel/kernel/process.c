#include <kernel/process.h>

#include <stdlib.h>
#include <string.h>

static queue_t *process_queues[MAX_PRIORITY+1];

int kernel_proc(int (*start)(void*), void *arg)
{
	int pid = new_pid();

	process_t *p = new_proc(); 
	if (!p)
		return 1;
	
	p->parent_pid = (cur_pid == -1) ? pid : cur_pid;
	setup_start_point(p, start, arg);

	proc_list[pid] = p;
	make_runnable(pid);
	return 0;
}

process_t *new_proc()
{
	process_t *p = (process_t*) calloc(1, sizeof(process_t));
	if (!p)
		return 0;

	if ( !( p->files = create_dynarray() ) || 
			 !( p->hw_context = create_hw_context() ) ||
			 !( p->pd_addr = alloc_physical_page() ) ||
			 !( p->kernel_stack = (stackint_t*) (malloc(KERNEL_STACK_SIZE)+KERNEL_STACK_SIZE) ) ||
			 !( p->cwd = (char*) calloc(2,sizeof(char))) )
	{
		free_proc(p);
		return 0;
	}

	p->cwd[0] = '/';

	pde_t *pde = (pde_t*) temp_map(p->pd_addr, 0);
	memset(pde, 0, PD_SIZE);

	for (int i = FIRST_KERNEL_PDE ; i < NB_PDE-1 ; i++)
		pde[i] = (cur_pd_addr())[i];
	pd_rec_map(pde, p->pd_addr);
	
	return p;
}

void free_proc_userspace(process_t *proc)
{
	pde_t *pd_temp = (pde_t*) temp_map(proc->pd_addr, 0); 
	
	for (int i = 0 ; i < FIRST_KERNEL_PDE ; i++, pd_temp++)
	{
		if (!pde_is_present(pd_temp))
			continue;
	
		pte_t *pt_temp = (pte_t*) temp_map(pde_addr(pd_temp), 1);

		for (int j = 0 ; j < NB_PTE ; j++)
		{
			if (pte_is_present(pt_temp))
				free_physical_page(pte_addr(pt_temp));
			pt_temp++;
		}
		free_physical_page(pde_addr(pd_temp));
	}
}

void free_proc(process_t *proc)
{
	if (proc->pd_addr)
	{
		free_proc_userspace(proc);
		free_physical_page(proc->pd_addr);
	}

	if (proc->kernel_stack)
		free(((void*)proc->kernel_stack)-KERNEL_STACK_SIZE);
	if (proc->hw_context)
		free_hw_context(proc->hw_context);
	
	if (proc->files)
	{
		for (int i = 0 ; i < proc->files->size ; i++)
		{
			file_descr_t *fd = (file_descr_t*) proc->files->array[i];
			if (fd)
			{
				fd->owners--;
				if (!fd->owners)
					fd->close(fd);
			}
		}
		free_dynarray(proc->files);
	}

	if (proc->cwd)
		free(proc->cwd);
	
	free(proc);
}

int new_pid()
{
	int i = (cur_pid+1)%NB_MAX_PROC;
	while (i != cur_pid && proc_list[i] != 0)
	{
		i++;
		if (i >= NB_MAX_PROC)
			i = 0;
	}
	return (i == cur_pid) ? -1 : i;
}

void scheduler_init()
{
	cur_pid = -1;
	for (int i = 0 ; i <= MAX_PRIORITY ; i++)
		process_queues[i] = create_queue();
}

void make_runnable(int pid)
{
	proc_list[pid]->state = Runnable;

	int p = proc_list[pid]->priority;
	queue_push(process_queues[p], (void*) pid);
}

void schedule()
{
	for (int i = MAX_PRIORITY ; i >= 0 ; i--)
	{
		queue_t *q = process_queues[i];
		int pid = -1;

		while (!queue_empty(q))
		{
			pid = (int) queue_first(q);
			if (proc_list[pid] && proc_list[pid]->state == Runnable)
				break;
			queue_pop(q);
		}

		if (!queue_empty(q))
		{
			cur_pid = pid;
			proc_list[pid]->ms_left = DEFAULT_TIME;
			queue_pop(q);
			queue_push(q, (void*) pid);
			return;
		}
	}

	proc_list[IDLE_PID]->ms_left = DEFAULT_TIME;
	cur_pid = IDLE_PID;
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
	
	if (prev_pid != cur_pid)
		context_switch(proc_list[prev_pid], proc_list[cur_pid]);
}

