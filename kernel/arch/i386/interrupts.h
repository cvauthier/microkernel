#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stddef.h>
#include <stdint.h>

#define IDT_PRESENT 0x80
#define IDT_DPL_3 0x60
#define IDT_DPL_0 0
#define IDT_TASK_GATE 0x5
#define IDT_INT_GATE 0xE
#define IDT_TRAP_GATE 0xF

void add_idt_descriptor(uint8_t *target, uint32_t base, uint16_t selector, uint8_t type);

void interrupts_setup();

#endif
