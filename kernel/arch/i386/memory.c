#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <kernel/memory.h>

extern uint32_t _kernel_end;
extern void load_gdt(uint64_t *ptr, uint32_t size);
extern void load_idt(uint64_t *ptr);
extern void load_page_directory(uint32_t *ptr);
extern void enable_paging();

uint64_t kernel_gdt[4];
uint64_t kernel_idt[256];
uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t page_table_1[1024] __attribute__((aligned(4096)));
uint32_t page_table_2[1024] __attribute__((aligned(4096)));

uint32_t *stack_top;

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
        target[6] = 0x40;
 
    target[0] = limit & 0xFF;
    target[1] = (limit >> 8) & 0xFF;
    target[6] |= (limit >> 16) & 0xF;
 
    target[2] = base & 0xFF;
    target[3] = (base >> 8) & 0xFF;
    target[4] = (base >> 16) & 0xFF;
    target[7] = (base >> 24) & 0xFF;
 
    target[5] = type;
}

void add_idt_descriptor(uint8_t *target, uint32_t base, uint8_t type)
{
	target[0] = base&0xFF;
	target[1] = (base>>8)&0xFF;
	target[2] = target[3] = target[4] = 0;
	target[5] = type;
	target[6] = (base>>16)&0xFF;
	target[7] = (base>>24);
}

void init_memory()
{
	uint8_t *p = (uint8_t*) kernel_gdt;
	add_gdt_descriptor(p, 0, 0, 0); // Null segment
	add_gdt_descriptor(p+8, 0x3FFFFFFF, 0, 0x9A); // Code segment
	add_gdt_descriptor(p+16,0x3FFFFFFF, 0, 0x92); // Data segment
	load_gdt(kernel_gdt, 3*8);

	load_idt(kernel_idt);

	unsigned int i;
	for (i = 0 ; i < 1024 ; i++)
	{
		page_table_1[i] = (i << 12) | 3; // Write & Present
		page_table_2[i] = ((i+1024) << 12) | 3;
		page_directory[i] = 0x00000002; // Write
	}
	page_directory[0] = ((unsigned int) page_table_1) | 3;
	page_directory[1] = ((unsigned int) page_table_2) | 3;
	page_directory[1023] = ((unsigned int) page_directory) | 3;

	load_page_directory(page_directory);
	enable_paging();

	stack_top = (&_kernel_end) + 128;
	*stack_top = 0;
	for (uint32_t i = 2<<10 ; i < 2<<30 ; i+=4*1024)
		*(++stack_top) = i;
	
	i = (uint32_t) (stack_top+1);
	i = ((i+1023)>>10)<<10;
	kernel_heap_begin = (void*) i;
}

void *alloc_physical_page()
{
	if (*stack_top == 0)
		return (void*) 0;
	
	return (void*) *(stack_top--);
}

void free_physical_page(void *page_ptr)
{
	*(++stack_top) = (uint32_t) page_ptr;
}

void map_page(void *page_addr)
{
	uint32_t *pd = (uint32_t*) (0xFFFFF000 | ((uint32_t) page_addr & 0xFFF));
	if (!(*pd & 0x1))
	{
		// Allocate new page directory
		void *new_phys_page = alloc_physical_page();
		// TODO : Add error if new_phys_page = 0
		*pd |= (uint32_t) new_phys_page & 0xFFFFF000 | 3;
	}

	uint32_t *pt = (uint32_t*) (0xFFC00000 | ((uint32_t) page_addr >> 10) & ~0x3);
	if (!(*pt & 0x1))
	{
		void *new_phys_page = alloc_physical_page();
		// TODO : Add error if new_phys_page = 0
		*pt |= (uint32_t) new_phys_page & 0xFFFFF000 | 3;
	}
}

