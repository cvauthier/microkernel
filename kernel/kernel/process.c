#include <kernel/process.h>

#include <stdlib.h>

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

void schedule()
{
	int i = (cur_pid+1)%NB_MAX_PROC;
	while (i != cur_pid)
	{
		if (proc_list[i] != 0 && proc_list[i]->state == Runnable)
		{
			proc_list[i]->ms_left = DEFAULT_TIME; 
			cur_pid = i;
			return;
		}
		i++;
		if (i >= NB_MAX_PROC)
			i = 0;
	}
}

int syscall_wait(int *pid, int *code)
{
	int children = 0;
	for (int i = 0 ; i < NB_MAX_PROC ; i++)
	{
		if (i != cur_pid && proc_list[i] != 0 && proc_list[i]->parent_pid == cur_pid)
		{
			children++;
			if (proc_list[i]->state == Zombie)
			{
				*pid = i;
				*code = proc_list[i]->exit_code;
				free_proc_data(proc_list[i]->arch_data);
				free(proc_list[i]);
				proc_list[i] = 0;
				return 1;
			}
		}
	}
	if (children)
	{
		proc_list[cur_pid]->state = Waiting;
		return -1;
	}
	return 0;
}

int syscall_fork()
{
	int child_pid = new_pid();
	if (child_pid < 0)
		return child_pid;

	proc_data_t *new_data = fork_proc_data(proc_list[cur_pid]->arch_data);
	if (!new_data)
		return -1;
	
	proc_list[child_pid] = (process_t*) malloc(sizeof(process_t));
	proc_list[child_pid]->parent_pid = cur_pid;
	proc_list[child_pid]->state = Runnable;
	proc_list[child_pid]->priority = proc_list[cur_pid]->priority;
	proc_list[child_pid]->arch_data = new_data;

	return child_pid;
}

void syscall_exit(int code)
{
	proc_list[cur_pid]->exit_code = code;
	proc_list[cur_pid]->state = Zombie;
	
	int parent = proc_list[cur_pid]->parent_pid;
	if (proc_list[parent] == 0)
		parent = proc_list[cur_pid]->parent_pid = ZOMBIE_SLAYER_PID;
	
	if (proc_list[parent]->state == Waiting)
		proc_list[parent]->state = Runnable;
}

