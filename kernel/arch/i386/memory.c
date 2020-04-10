#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <kernel/memory.h>
#include "memory_i386.h"
#include "interrupts.h"

extern uint32_t _kernel_end;
extern uint32_t tss_stack_top;

extern void load_gdt(uint64_t *ptr, uint32_t size);
extern void tlb_flush();
extern void tss_flush();

uint64_t kernel_gdt[6];
uint32_t kernel_tss[TSS_SIZE/4];


uint32_t next_page;

void *get_physical_addr(void *virtual_addr)
{
	uint32_t vaddr = (uint32_t) virtual_addr;
	uint32_t *page_table = (uint32_t*) ((0xFFC00000 | (vaddr >> 10)) & ~0x3);

	return (void*) (((*page_table) & 0xFFFFF000) | (vaddr & 0xFFF));
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

void memory_setup()
{
	uint8_t *p = (uint8_t*) kernel_gdt;
	add_gdt_descriptor(p, 0, 0, 0); // Null segment
	add_gdt_descriptor(p+8, 0xFFFFFFFF, 0, GDT_PRESENT | GDT_DPL_0 | GDT_CODE_DATA | GDT_EXEC | GDT_RW);
	add_gdt_descriptor(p+16,0xFFFFFFFF, 0, GDT_PRESENT | GDT_DPL_0 | GDT_CODE_DATA | GDT_RW);
	add_gdt_descriptor(p+24,0xFFFFFFFF, 0, GDT_PRESENT | GDT_DPL_3 | GDT_CODE_DATA | GDT_EXEC | GDT_RW);
	add_gdt_descriptor(p+32,0xFFFFFFFF, 0, GDT_PRESENT | GDT_DPL_3 | GDT_CODE_DATA | GDT_RW);
	
	memset(kernel_tss, 0, TSS_SIZE);
	kernel_tss[TSS_SS0] = 0x10; // kernel data segment
	kernel_tss[TSS_ESP0] = (uint32_t) &tss_stack_top; // kernel stack
	add_gdt_descriptor(p+40, TSS_SIZE-1, (uint32_t) kernel_tss, GDT_PRESENT | GDT_DPL_3 | GDT_EXEC | GDT_AC); 

	load_gdt(kernel_gdt, 6*8);
	tss_flush();

	interrupts_setup();

	next_page = (uint32_t) get_physical_addr(&_kernel_end); 
	next_page = next_page/4096*4096;
}

void *simple_alloc_physical_page()
{
	next_page += 4096;
	return (void*) next_page;
}

void map_page(void *page_addr)
{
	uint32_t addr = (uint32_t) page_addr;
	uint32_t *pd = (uint32_t*) (0xFFFFF000 | ((addr>>22)<<2) );
	if (!(*pd & 0x1))
	{
		// Allocate new page directory
		void *new_phys_page = simple_alloc_physical_page();
		*pd |= (((uint32_t) new_phys_page) & 0xFFFFF000) | 3;
		tlb_flush();

		uint32_t *pt = (uint32_t*) (0xFFC00000 | (addr>>22)<<12);
		memset(pt, 0, 4096);
	}

	uint32_t *pt = (uint32_t*) (0xFFC00000 | ((addr>>12)<<2));
	if (!(*pt & 0x1))
	{
		void *new_phys_page = simple_alloc_physical_page();
		*pt |= (((uint32_t) new_phys_page) & 0xFFFFF000) | 3;
		tlb_flush();
	}
}

