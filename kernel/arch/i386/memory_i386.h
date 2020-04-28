#ifndef MEMORY_I386_H
#define MEMORY_I386_H

#include <stddef.h>
#include <stdint.h>

#define GDT_PRESENT 0x80
#define GDT_DPL_3 0x60
#define GDT_DPL_0 0
#define GDT_CODE_DATA 0x10
#define GDT_TSS 0
#define GDT_EXEC 0x08
#define GDT_DC 0x04
#define GDT_RW 0x02
#define GDT_AC 0x01

#define TSS_SIZE 104
#define TSS_SS0 0x08
#define TSS_ESP0 0x04

#define PD_ADDR ((uint32_t*) 0xFFFFF000)
#define PT_ADDR(n) ((uint32_t*) (0xFFC00000 | ((n)<<12)))

extern uint32_t _kernel_end;
extern uint32_t _4M_aligned_after_kernel_end;
extern uint32_t tss_stack_top;

// Gestion des PD (Page Directory) et PT (Page Table)

extern void load_pd(uint32_t pd_physical_addr);
extern void tlb_flush();

uint32_t get_physical_addr(void *virtual_addr);
void add_pd_entry(int index, uint32_t pt_physical_addr);
void add_pt_entry(void *virtual_addr, uint32_t physical_addr); // Suppose que la PT correspondante a été ajoutée
void add_page(void *virtual_addr, uint32_t physical_addr);

// Gestion de la GDT

void add_gdt_descriptor(uint8_t *target, uint32_t limit, uint32_t base, uint8_t type);
extern void load_gdt(void *gdt_addr, uint16_t size);

void set_tss_stack(uint32_t *stack);
extern void tss_flush();

// Gestion de l'IDT

extern void load_idt(void *idt_addr);

// Ports

extern void outb(uint16_t port, uint8_t val);
extern uint8_t inb(uint16_t port);

// Gestion de la mémoire

uint32_t alloc_physical_page();
void free_physical_page(uint32_t page);

// Quelques pages temporaires

extern uint32_t temp_page_1;
extern uint32_t temp_page_2;
extern uint32_t temp_page_3;
extern uint32_t temp_page_4;
extern uint32_t temp_page_5;
extern uint32_t temp_page_6;

#endif
