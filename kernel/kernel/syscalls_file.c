#include <kernel/syscalls.h>

#include <kernel/process.h>
#include <kernel/memory.h>
#include <kernel/filesystem.h>

#include <stdlib.h>
#include <string.h>

/* Implémentation des appels liés aux fichiers : open, read, write, seek, close, dup, dup2 */

file_descr_t *access_file(int fd, process_t *p)
{
	if (fd < 0 || fd >= p->files->size || !p->files->array[fd])
		return 0;
	return (file_descr_t*) p->files->array[fd];
}

int syscall_open(const char *path)
{
	char *actual_path = concat_dirs(proc_list[cur_pid]->cwd, path);
	if (!actual_path) return -1;

	file_descr_t *f = open_rd(actual_path);
	free(actual_path);
	if (!f) return -1;
	
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

int syscall_dup(int oldfd)
{
	process_t *p = proc_list[cur_pid];
	file_descr_t *f;
	
	if (!(f = access_file(oldfd, p)))
		return -1;
	f->owners++;
	dynarray_push(p->files, (void*)f);
	return p->files->size-1;
}

int syscall_dup2(int oldfd, int newfd)
{
	process_t *p = proc_list[cur_pid];
	file_descr_t *f1,*f2;
	
	if (!(f1 = access_file(oldfd, p)))
		return -1;
	if (newfd < 0 || newfd >= p->files->size)
		return -1;
	if (newfd == oldfd)
		return newfd;

	f2 = (file_descr_t*) p->files->array[newfd];
	if (f2)
	{
		f2->owners--;
		if (!f2->owners)
			f2->close(f2);
	}

	f1->owners++;
	p->files->array[newfd] = (void*) f1;
	return newfd;
}

