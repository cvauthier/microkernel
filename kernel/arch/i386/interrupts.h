#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stddef.h>
#include <stdint.h>

#include <kernel/syscalls.h>

#define IDT_PRESENT 0x80
#define IDT_DPL_3 0x60
#define IDT_DPL_0 0
#define IDT_TASK_GATE 0x5
#define IDT_INT_GATE 0xE
#define IDT_TRAP_GATE 0xF

uint32_t syscalls_addr[NB_SYSCALLS];

void add_idt_descriptor(uint8_t *target, uint32_t base, uint16_t selector, uint8_t type);
void interrupts_setup();

void page_fault_handler();
void default_handler();
void irq_handler(int num);

#endif
