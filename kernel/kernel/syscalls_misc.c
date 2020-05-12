#include <kernel/syscalls.h>

#include <kernel/process.h>
#include <kernel/memory.h>
#include <kernel/filesystem.h>

#include <stdlib.h>
#include <string.h>

/* Implémentation des appels sbrk, getcwd, chdir */

void *syscall_sbrk(int incr)
{
	process_t *p = proc_list[cur_pid];

	if (incr == 0)
		return (void*) p->heap_end;
	
	if (incr > 0)
	{
		vaddr_t inc = (vaddr_t) incr;
		if (inc > p->stack_bottom-p->heap_end)
			return (void*) p->heap_end;
		
		vaddr_t new_end = p->heap_end+inc;
		new_end = (new_end+PAGE_SIZE-1)/PAGE_SIZE*PAGE_SIZE;
		while (p->heap_end < new_end)
		{
			paddr_t page = alloc_physical_page();
			if (!page)
				break;
			add_page(p->heap_end, page);

			pte_t *pte = pte_of_addr(cur_pt_addr(p->heap_end), p->heap_end);
			pte_noexec(pte);
			pte_mkwrite(pte);
			pte_mkread(pte);

			p->heap_end += PAGE_SIZE;
		}
	}
	else
	{
		vaddr_t dec = (vaddr_t) (-incr);
		if (dec > p->heap_end-p->heap_begin)
			return (void*) p->heap_end;
		
		vaddr_t new_end = p->heap_end - dec;
		while (p->heap_end > new_end)
		{
			p->heap_end -= PAGE_SIZE;

			pte_t *pte = pte_of_addr(cur_pt_addr(p->heap_end), p->heap_end);
			pte_set_present(pte, 0);
			free_physical_page(pte_addr(pte));
		}
	}

	return (void*) p->heap_end;
}

char *syscall_getcwd(char *buf, size_t size)
{
	char *cwd = proc_list[cur_pid]->cwd;
	if (strlen(cwd) >= size)
		return 0;
	
	strcpy(buf, cwd);
	return buf;
}

int syscall_chdir(const char *path)
{
	char *actual_path = concat_dirs(proc_list[cur_pid]->cwd, path);
	if (!actual_path)
		return -1;
	
	file_descr_t *fd = open_rd(actual_path);
	int res = -1;

	if (fd)
	{
		if (fd->type == FileType_Directory)
		{
			free(proc_list[cur_pid]->cwd);
			proc_list[cur_pid]->cwd = actual_path;
			res = 0;
		}
		else
			free(actual_path);
		close_rd(fd);
	}
	
	return res;
}

int syscall_invalid()
{
	return -1;
}


