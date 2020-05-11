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
			 !( p->kernel_stack = (stackint_t*) (malloc(KERNEL_STACK_SIZE)+KERNEL_STACK_SIZE) ) )
	{
		free_proc(p);
		return 0;
	}

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

	process_t *p = new_proc();
	if (!p)
		return -1;
	
	process_t *forked = proc_list[cur_pid];

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

static uint16_t regroup_int16(uint8_t *ints, int endian)
{
	return (endian == 1) ? ints[0] + (ints[1]<<8) : ints[1] + (ints[0]<<8);
}

static uint32_t regroup_int32(uint8_t *ints, int endian)
{
	return (endian == 1) ? ints[0] + (ints[1]<<8) + (ints[2]<<16) + (ints[3]<<24) : ints[3] + (ints[2]<<8) + (ints[1]<<16) + (ints[0]<<24);
}

static void install_segment(file_descr_t *fd, vaddr_t addr, uint32_t filepos, uint32_t filesz, uint32_t memsz, int exec)
{
	uint8_t c;
	fd->seek(fd, filepos, SEEKFD_BEGIN);

	vaddr_t addr0 = addr;

	while (memsz)
	{
		pte_t *pte = map_page((void*)addr);
		if (exec)
		{
			pte_nowrite(pte);
			pte_mkexec(pte);
		}
		else
		{
			pte_noexec(pte);
			pte_mkwrite(pte);
		}
		pte_mkread(pte);
		tlb_flush();

		memset((void*)addr, 0, PAGE_SIZE);
		addr += PAGE_SIZE;
		memsz -= PAGE_SIZE;
	}

	uint8_t *p = (uint8_t*) addr0;
	while (filesz)
	{
		fd->read(fd, &c, 1);
		*(p++) = c;
		filesz--;
	}
}

void syscall_exec(const char *path)
{
	file_descr_t *fd = open_rd(path);
	if (!fd)
		return;

	uint8_t header[52];
	uint8_t text_hdr[32];
	uint8_t data_hdr[32];
	vaddr_t entry_point;

	uint32_t text_pos, data_pos, text_fsz, data_fsz, text_msz, data_msz;
	vaddr_t text_addr, data_addr;
	
	int endian;

	if (fd->read(fd, header, 52) != 52) return;
	if (header[0] != 0x7F || header[1] != 'E' || header[2] != 'L' || header[3] != 'F') return;
	if (header[4] != 1) return; // Pas 32-bit
	
	endian = (header[5] == 1);

	if (regroup_int16(header+16, endian) != 2) return; // Pas un fichier objet exécutable
	if (regroup_int16(header+18, endian) != 3) return; // Pas x86
	if (regroup_int16(header+44, endian) < 2) return; // Il faut qu'il y ait au moins deux Program Header (le 1er pour le code, le 2e pour les données)

	entry_point = (vaddr_t) regroup_int32(header+24, endian);

	fd->seek(fd, regroup_int32(header+28,endian), SEEKFD_BEGIN); // Début de la Program Header Table

	if (fd->read(fd, text_hdr, 32) != 32) return;
	if (fd->read(fd, data_hdr, 32) != 32) return;

	if (regroup_int32(text_hdr, endian) != 1 || regroup_int32(data_hdr, endian) != 1) return; // Le segment doit être de type "load"

	if (regroup_int32(text_hdr+24, endian) != 5 || regroup_int32(data_hdr+24, endian) != 6) return; // Les permissions doivent être respectivement r-x et rw- 

	text_pos = regroup_int32(text_hdr+4, endian);
	text_addr = (vaddr_t) regroup_int32(text_hdr+8, endian);
	text_fsz = regroup_int32(text_hdr+16, endian);
	text_msz = regroup_int32(text_hdr+20, endian);
	
	data_pos = regroup_int32(data_hdr+4, endian);
	data_addr = (vaddr_t) regroup_int32(data_hdr+8, endian);
	data_fsz = regroup_int32(data_hdr+16, endian);
	data_msz = regroup_int32(data_hdr+20, endian);

	text_msz = (text_msz+PAGE_SIZE-1)/PAGE_SIZE*PAGE_SIZE;
	data_msz = (data_msz+PAGE_SIZE-1)/PAGE_SIZE*PAGE_SIZE;

	if (text_addr != 0x08048000) return;
	if (text_addr + text_msz > data_addr) return;
	if (text_addr%PAGE_SIZE || data_addr%PAGE_SIZE) return;
	
	process_t *p = proc_list[cur_pid];
	free_proc_userspace(p);	

	// TODO : s'assurer qu'il y a assez de mémoire

	p->code_begin = text_addr;
	p->code_end = text_addr+text_msz; 
	p->data_begin = data_addr;
	p->data_end = p->heap_begin = p->heap_end = data_addr + data_msz;
	p->stack_top = (vaddr_t) KERNEL_BEGIN;
	p->stack_bottom = p->stack_top - 4*PAGE_SIZE; 

	install_segment(fd, text_addr, text_pos, text_fsz, text_msz, 1);
	install_segment(fd, data_addr, data_pos, data_fsz, data_msz, 0);
	install_segment(fd, p->stack_bottom, 0, 0, 4*PAGE_SIZE, 0);

	jump_to_ring3((void (*)()) entry_point, ((stackint_t*)p->stack_top)-1);	
}

