#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <virux_lib.h>

#include <kernel/memory.h>
#include <kernel/filesystem.h>

#include "memory_i386.h"
#include "interrupts.h"

uint64_t kernel_gdt[6];
uint32_t kernel_tss[TSS_SIZE/4];

paddr_t *page_stack;
vaddr_t heap_begin;

vaddr_t get_heap_begin()
{
	return heap_begin;
}

pde_t *add_pde(pde_t *pd_first, vaddr_t vaddr, paddr_t paddr)
{
	pde_t *e = pde_of_addr(pd_first, vaddr);
	*e = (paddr & 0xFFFFF000) | 0x7; // Present + ReadWrite + User/Supervisor
	return e;
}

pte_t *add_pte(pte_t *pt_first, vaddr_t vaddr, paddr_t paddr)
{
	pte_t *e = pte_of_addr(pt_first, vaddr);
	*e = (paddr & 0xFFFFF000) | 0x3; // Present + ReadWrite
	return e;
}

pte_t *add_page(vaddr_t vaddr, paddr_t paddr)
{
	pde_t *pde = pde_of_addr(cur_pd_addr(), vaddr);

	if (!(*pde & 1))
	{
		paddr_t p = alloc_physical_page();
		if (!p)
			return 0;
		add_pde(cur_pd_addr(), vaddr, p);
		memset(cur_pt_addr(vaddr), 0, PT_SIZE);
	}

	pte_t *pte = pte_of_addr(cur_pt_addr(vaddr), vaddr);
	*pte = (paddr & 0xFFFFF000) | 0x3;
	return pte;
}

pte_t *map_page(void *page_addr)
{
	paddr_t p = alloc_physical_page();
	if (!p)
		return 0;
	return add_page((vaddr_t) page_addr, p);
}

paddr_t alloc_physical_page()
{
	return *(page_stack--);
}

void free_physical_page(paddr_t page)
{
	page_stack++;
	*page_stack = page;
}

vaddr_t temp_map(paddr_t paddr, int i)
{
	vaddr_t temp = (vaddr_t) (((uint8_t*) &temp_page_1) + i*PAGE_SIZE);
	add_page(temp, paddr);
	tlb_flush();
	return temp;
}

void add_gdt_descriptor(uint8_t *target, uint32_t limit, uint32_t base, uint8_t type)
{
  if (limit > 65536) 
	{
  	limit >>= 12;
  	target[6] = 0xC0;
  } 
	else 
	{
   	target[6] = 0x40;
  }
    
	target[0] = limit & 0xFF;
	target[1] = (limit >> 8) & 0xFF;
	target[6] |= (limit >> 16) & 0xF;
    
  target[2] = base & 0xFF;
	target[3] = (base >> 8) & 0xFF;
	target[4] = (base >> 16) & 0xFF;
  target[7] = (base >> 24) & 0xFF;
 
  target[5] = type;
}

void set_tss_stack(stackint_t *stack)
{
	kernel_tss[TSS_ESP0] = (uint32_t) stack;
}

vaddr_t temp_map_chunk(paddr_t addr, size_t chunk_size)
{
	paddr_t p = (addr/PAGE_SIZE)*PAGE_SIZE;

	vaddr_t v = temp_map(p, 0);
	if (addr+chunk_size >= p+PAGE_SIZE)
		temp_map(p+PAGE_SIZE, 1);
	
	return v+(addr-p);
}

void memory_setup(multiboot_info_t* mbd)
{
	/* Initialisation de la GDT, de l'IDT */

	uint8_t *p = (uint8_t*) kernel_gdt;
	add_gdt_descriptor(p, 0, 0, 0); // Null segment
	add_gdt_descriptor(p+8, 0xFFFFFFFF, 0, GDT_PRESENT | GDT_DPL_0 | GDT_CODE_DATA | GDT_EXEC | GDT_RW);
	add_gdt_descriptor(p+16,0xFFFFFFFF, 0, GDT_PRESENT | GDT_DPL_0 | GDT_CODE_DATA | GDT_RW);
	add_gdt_descriptor(p+24,0xFFFFFFFF, 0, GDT_PRESENT | GDT_DPL_3 | GDT_CODE_DATA | GDT_EXEC | GDT_RW);
	add_gdt_descriptor(p+32,0xFFFFFFFF, 0, GDT_PRESENT | GDT_DPL_3 | GDT_CODE_DATA | GDT_RW);

	memset(kernel_tss, 0, TSS_SIZE);
	kernel_tss[TSS_SS0] = 0x10; // kernel data segment
	kernel_tss[TSS_ESP0] = 0;
	add_gdt_descriptor(p+40, TSS_SIZE-1, (uint32_t) kernel_tss, GDT_PRESENT | GDT_DPL_3 | GDT_EXEC | GDT_AC); 

	load_gdt((vaddr_t) kernel_gdt, 6*8);
	tss_flush();

	/* Récupération de la quantité de mémoire totale 
		 On doit d'abord mapper l'addresse de mbd */

	mbd = (multiboot_info_t*) temp_map_chunk((paddr_t) mbd, sizeof(multiboot_info_t));
	memory = mbd->mem_upper<<10;
	nb_pages = (memory+PAGE_SIZE-1)/PAGE_SIZE;

	/* Récupération de l'adresse physique du ramdisk */
	paddr_t rd_begin = 0, rd_end = 0;
	paddr_t rd_first_page = 0, rd_last_page = 0;
	
	if (mbd->flags&0x4 && mbd->mods_count > 0)
	{
		uint32_t *modstruct = (uint32_t*) temp_map_chunk((paddr_t) mbd->mods_addr, 2*sizeof(uint32_t));
		rd_begin = modstruct[0];
		rd_end = modstruct[1];
		rd_first_page = rd_begin/PAGE_SIZE*PAGE_SIZE;
		rd_last_page = (rd_end-1)/PAGE_SIZE*PAGE_SIZE;
	}

	/* On commence par copier le ramdisk juste après le kernel */

	paddr_t next_page = get_paddr((vaddr_t) &_kernel_end);
	next_page = (1+(next_page/PAGE_SIZE))*PAGE_SIZE;
	
	if (rd_end)
	{
		rd_begin -= rd_first_page;
		rd_end -= rd_last_page;
		paddr_t rd_size = rd_last_page-rd_first_page+PAGE_SIZE;
		
		if (next_page < rd_first_page)
		{
			for (paddr_t i = 0 ; i < rd_size ; i+=PAGE_SIZE)
				memcpy((void*) temp_map(next_page+i, 0), (void*) temp_map(rd_first_page+i, 1), PAGE_SIZE);
		}
		else if (next_page > rd_first_page)
		{
			paddr_t p = next_page+rd_size-PAGE_SIZE;
			for (paddr_t i = 0 ; i < rd_size ; i+=PAGE_SIZE)
				memcpy((void*) temp_map(p-i, 0), (void*) temp_map(rd_last_page-i, 1), PAGE_SIZE);
		}

		rd_first_page = next_page;
		next_page += rd_size;
		rd_last_page = next_page-PAGE_SIZE;
		rd_begin += rd_first_page;
		rd_end += rd_last_page;
	}

	/* On réserve un peu de mémoire pour mettre en place un allocateur de pages physiques
		 (fonctionnant avec une pile) */

	int needed_memory = ((memory/PAGE_SIZE) + 1) * sizeof(paddr_t);
	int needed_pd_entries = (needed_memory/(NB_PTE*PAGE_SIZE))+1;

	vaddr_t vaddr = (vaddr_t) &_4M_aligned_after_kernel_end;

	for (int i = 0 ; i < needed_pd_entries ; i++)
	{
		add_pde(cur_pd_addr(), vaddr, next_page);
		next_page += PAGE_SIZE;
		
		for (int j = 0 ; j < NB_PTE ; j++)
		{
			add_pte(cur_pt_addr(vaddr), vaddr, next_page);
			next_page += PAGE_SIZE;
			vaddr += PAGE_SIZE;
		}
	}

	next_page /= PAGE_SIZE;

	page_stack = (paddr_t*) &_4M_aligned_after_kernel_end;
	*(page_stack++) = 0;
	for (unsigned int i = nb_pages-1+(1<<8) /*Tenir compte du 1er MB*/ ; i >= next_page ; i--)
		*(page_stack++) = i*PAGE_SIZE;
	page_stack--;

	heap_begin = vaddr;

	/* Interruptions */

	interrupts_setup();

	/* Chargement du ramdisk : on transfère le ramdisk dans le tas et on libère les pages physiques qu'il occupait */

	ramdisk = (uint8_t*) malloc(sizeof(uint8_t)*(rd_end-rd_begin));
	disk_size = rd_end-rd_begin-4;

	int i = 0;
	while (rd_begin < rd_end)
	{
		uint8_t *tmp = (uint8_t*) temp_map(rd_first_page, 0);
		temp_map(rd_first_page+PAGE_SIZE, 1);
		tmp += (rd_begin-rd_first_page);
		
		memcpy(ramdisk+i, tmp, (rd_end-rd_begin > PAGE_SIZE) ? PAGE_SIZE : (rd_end-rd_begin));
		free_physical_page(rd_first_page);

		rd_first_page += PAGE_SIZE;
		i += PAGE_SIZE;
		rd_begin += PAGE_SIZE;
	}
	while (rd_first_page <= rd_last_page)
	{
		free_physical_page(rd_first_page);
		rd_first_page += PAGE_SIZE;
	}

	nb_inodes = read_bigendian_int(ramdisk);
	last_free_inode = read_bigendian_int(ramdisk+4);
	free_block_list = read_bigendian_int(ramdisk+8);
	free_block_nb = read_bigendian_int(ramdisk+12);
	root_inode = read_bigendian_int(ramdisk+16);
	block_size = read_bigendian_int(ramdisk+disk_size);
}

