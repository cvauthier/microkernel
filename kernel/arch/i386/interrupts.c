#include <string.h>

#include "interrupts.h"

extern void load_idt(uint64_t *ptr);
uint64_t kernel_idt[256];

void add_idt_descriptor(uint8_t *target, uint32_t base, uint16_t selector, uint8_t type)
{
	target[0] = base&0xFF;
	target[1] = (base>>8)&0xFF;
	target[2] = selector&0xFF;
	target[3] = (selector>>8)&0xFF;
	target[4] = 0;
	target[5] = type;
	target[6] = (base>>16)&0xFF;
	target[7] = (base>>24);
}

void interrupts_setup()
{
	memset(kernel_idt, 0, sizeof(kernel_idt));
	load_idt(kernel_idt);
}

