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

void *get_physical_addr(void *virtual_addr);

void add_gdt_descriptor(uint8_t *target, uint32_t limit, uint32_t base, uint8_t type);

void *simple_alloc_physical_page();

#endif
