#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <kernel/memory.h>
#include <kernel/filesystem.h>
#include <kernel/utility.h>

#include "memory_i386.h"
#include "interrupts.h"

uint64_t kernel_gdt[6];
uint32_t kernel_tss[TSS_SIZE/4];

uint32_t *page_stack;
uint32_t *heap_begin;

uint32_t get_physical_addr(void *virtual_addr)
{
	uint32_t vaddr = (uint32_t) virtual_addr;
	uint32_t *page_table = PT_ADDR(vaddr>>22);
	page_table += (vaddr>>12)&0x3FF;
	return ((*page_table) & 0xFFFFF000) | (vaddr & 0xFFF);
}

void add_pd_entry(int index, uint32_t pt_physical_addr)
{
	uint32_t *pd = PD_ADDR+index;
	*pd = (pt_physical_addr&0xFFFFF000) | 0x3; // Present + ReadWrite
	
	uint32_t *pt = PT_ADDR(index);
	memset(pt, 0, 4096);
}

void add_pt_entry(void *virtual_addr, uint32_t physical_addr)
{
	uint32_t addr = (uint32_t) virtual_addr;

	uint32_t *pt = PT_ADDR(addr>>22) + ((addr>>12)&0x3FF);
	*pt = (physical_addr & 0xFFFFF000) | 0x3;
}

void add_page(void *virtual_addr, uint32_t physical_addr)
{
	uint32_t addr = (uint32_t) virtual_addr;
	uint32_t *pd = PD_ADDR + (addr>>22);

	if (!(*pd & 0x1))
		add_pd_entry(addr>>22,alloc_physical_page());

	uint32_t *pt = PT_ADDR(addr>>22) + ((addr>>12)&0x3FF);
	if (!(*pt & 0x1))
		*pt = (physical_addr & 0xFFFFF000) | 0x3;
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

void set_tss_stack(uint32_t *stack)
{
	kernel_tss[TSS_ESP0] = (uint32_t) stack;
}

uint32_t alloc_physical_page()
{
	return *(page_stack--);
}

void free_physical_page(uint32_t page)
{
	page_stack++;
	*page_stack = page;
}

// Rend accessible temporairement une portion (de taille < 4kB) en utilisant temp_page_1 et temp_page_2
void *temp_map(void *ptr, uint32_t size)
{
	uint32_t tmp = (uint32_t) ptr;
	
	add_pt_entry(&temp_page_1, tmp>>12<<12);
	if ((tmp+size-1)>>12 > tmp>>12)
		add_pt_entry(&temp_page_2, ((tmp>>12)+1)<<12);
	
	tlb_flush();
	return (void*) (((uint8_t*)&temp_page_1) + (tmp-(tmp>>12<<12)));
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
	kernel_tss[TSS_ESP0] = 0; // kernel stack
	add_gdt_descriptor(p+40, TSS_SIZE-1, (uint32_t) kernel_tss, GDT_PRESENT | GDT_DPL_3 | GDT_EXEC | GDT_AC); 

	load_gdt(kernel_gdt, 6*8);
	tss_flush();

	/* Récupération de la quantité de mémoire totale 
		 On doit d'abord mapper l'addresse de mbd */

	mbd = (multiboot_info_t*) temp_map((void*) mbd, sizeof(multiboot_info_t));
	memory = mbd->mem_upper<<10;
	nb_pages = (memory+4095)>>12;

	/* Récupération de l'adresse physique du ramdisk */
	uint32_t rd_begin = 0, rd_end = 0;
	uint32_t rd_first_pt = 0, rd_last_pt = 0;
	
	if (mbd->flags&0x4 && mbd->mods_count > 0)
	{
		uint32_t *modstruct = (uint32_t*) temp_map((void*)mbd->mods_addr, 2*sizeof(uint32_t));
		rd_begin = modstruct[0];
		rd_end = modstruct[1];
		rd_first_pt = rd_begin>>12<<12;
		rd_last_pt = (rd_end-1)>>12<<12;
	}

	/* On réserve un peu de mémoire pour mettre en place un allocateur de pages physiques
		 (fonctionnant avec une pile) */

	uint32_t next_page = get_physical_addr(&_kernel_end); 
	next_page = (1+(next_page>>12))<<12;

	if (next_page == rd_first_pt)
		next_page = rd_last_pt+4096;

	int needed_memory = ((memory>>12) + 1) * 4;
	int needed_pd_entries = (needed_memory>>22)+1;

	uint8_t *vaddr = (uint8_t*) &_4M_aligned_after_kernel_end;

	for (int i = 0 ; i < needed_pd_entries ; i++)
	{
		add_pd_entry(((uint32_t)vaddr)>>22, next_page);
		
		next_page += 4096;
		if (next_page == rd_first_pt)
			next_page = rd_last_pt+4096;
		
		for (int j = 0 ; j < 1024 ; j++)
		{
			add_pt_entry(vaddr, next_page);
			
			next_page += 4096;
			if (next_page == rd_first_pt)
				next_page = rd_last_pt+4096;
			vaddr += 4096;
		}
	}

	next_page >>= 12;
	page_stack = &_4M_aligned_after_kernel_end;
	*(page_stack++) = 0;
	for (unsigned int i = nb_pages-1+(1<<8) /*Tenir compte du 1er MB*/ ; i >= next_page ; i--)
	{
		if (i<<12 == rd_last_pt)
			i = (rd_first_pt>>12)-1;
		*(page_stack++) = i<<12;
	}
	page_stack--;

	heap_begin = (uint32_t*) vaddr;
	
	interrupts_setup();

	/* Chargement du ramdisk */

	ramdisk = (uint8_t*) malloc(sizeof(uint8_t)*(rd_end-rd_begin));
	disk_size = rd_end-rd_begin-4;
	int i = 0;
	while (rd_begin < rd_end)
	{
		uint8_t *tmp = (uint8_t*) temp_map((void*)rd_begin, 4096);
		memcpy(ramdisk+i, tmp, (rd_end-rd_begin > 4096) ? 4096 : (rd_end-rd_begin));
		i += 4096;
		rd_begin += 4096;
	}
	
	while (rd_first_pt <= rd_last_pt)
	{
		free_physical_page(rd_first_pt);
		rd_first_pt += 4096;
	}

	nb_inodes = read_bigendian_int(ramdisk);
	last_free_inode = read_bigendian_int(ramdisk+4);
	free_block_list = read_bigendian_int(ramdisk+8);
	free_block_nb = read_bigendian_int(ramdisk+12);
	root_inode = read_bigendian_int(ramdisk+16);
	block_size = read_bigendian_int(ramdisk+disk_size);
}

uint32_t *get_heap_begin()
{
	return heap_begin;
}

void map_page(void *page_addr)
{
	add_page(page_addr, alloc_physical_page());
}

