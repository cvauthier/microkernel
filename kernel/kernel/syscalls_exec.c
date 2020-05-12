#include <kernel/syscalls.h>

#include <kernel/process.h>
#include <kernel/memory.h>
#include <kernel/filesystem.h>

#include <stdlib.h>
#include <string.h>

/* Implémentation de l'appel exec */

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
	char *actual_path = concat_dirs(proc_list[cur_pid]->cwd, path);
	if (!actual_path) return;
	
	file_descr_t *fd = open_rd(actual_path);
	free(actual_path);
	if (!fd) return;

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

