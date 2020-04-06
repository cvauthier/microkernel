#include <stddef.h>
#include <stdint.h>

void *get_physical_addr(void *virtual_addr);

void add_gdt_descriptor(uint8_t *target, uint32_t limit, uint32_t base, uint8_t type);
void add_idt_descriptor(uint8_t *target, uint32_t base, uint8_t type);

void *simple_alloc_physical_page();


