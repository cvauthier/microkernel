#include <kernel/process.h>

#include <stdlib.h>

void free_proc(process_t *proc)
{
	free_proc_data(proc->arch_data);
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
				free_proc(proc_list[i]);
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
	proc_list[child_pid]->files = create_dynarray();
	proc_list[child_pid]->arch_data = new_data;

	for (int i = 0 ; i < proc_list[cur_pid]->files->size ; i++)
	{
		file_descr_t *fd = (file_descr_t*) proc_list[cur_pid]->files->array[i];
		if (fd)
			fd->owners++;
		dynarray_push(proc_list[child_pid]->files, fd);
	}

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

file_descr_t *access_file(int fd, process_t *p)
{
	if (fd < 0 || fd >= p->files->size || !p->files->array[fd])
		return 0;
	return (file_descr_t*) p->files->array[fd];
}

int syscall_open(const char *path)
{
	file_descr_t *f = open_rd(path);
	if (!f)
		return -1;
	f->owners++;
	dynarray_push(proc_list[cur_pid]->files, (void*)f);
	return proc_list[cur_pid]->files->size-1;
}

int32_t syscall_write(int fd, void *ptr, int32_t count)
{
	process_t *p = proc_list[cur_pid];
	file_descr_t *f;
	if (!(f = access_file(fd, p)))
		return 0;
	return f->write(f, ptr, count);
}

int32_t syscall_read(int fd, void *ptr, int32_t count)
{
	process_t *p = proc_list[cur_pid];
	file_descr_t *f;
	if (!(f = access_file(fd, p)))
		return 0;
	return f->read(f, ptr, count);
}

uint32_t syscall_seek(int fd, int32_t ofs, int flag)
{
	process_t *p = proc_list[cur_pid];
	file_descr_t *f;
	if (!(f = access_file(fd, p)))
		return 0;
	return f->seek(f, ofs, flag);
}

void syscall_close(int fd)
{
	process_t *p = proc_list[cur_pid];
	file_descr_t *f;
	if (!(f = access_file(fd, p)))
		return;
	f->owners--;
	if (!f->owners)
		f->close(f);
	p->files->array[fd] = 0;
}

