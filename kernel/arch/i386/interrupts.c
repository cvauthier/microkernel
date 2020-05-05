#include <string.h>
#include <stdio.h>

#include <kernel/keyboard.h>

#include "interrupts.h"
#include "memory_i386.h"

extern int except0();
extern int except1();
extern int except2();
extern int except3();
extern int except4();
extern int except5();
extern int except6();
extern int except7();
extern int except8();
extern int except9();
extern int except10();
extern int except11();
extern int except12();
extern int except13();
extern int except14();
extern int except16();
extern int except17();
extern int except18();
extern int except19();
extern int inter128();
extern int irq0();
extern int irq1();
extern int irq2();
extern int irq3();
extern int irq4();
extern int irq5();
extern int irq6();
extern int irq7();
extern int irq8();
extern int irq9();
extern int irq10();
extern int irq11();
extern int irq12();
extern int irq13();
extern int irq14();
extern int irq15();

void clock_tick();

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
	
	// Reprogrammation PIC - cf wiki.osdev.org/PIC
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0);
	outb(0xA1, 0);

	// Clavier + PIT
	outb(0x21, 0xFC);
	outb(0xA1, 0xFF);

	uint8_t *p = (uint8_t*) (kernel_idt+32);
	add_idt_descriptor(p     , (uint32_t)irq0  , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+8   , (uint32_t)irq1  , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+16  , (uint32_t)irq2  , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+24  , (uint32_t)irq3  , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+32  , (uint32_t)irq4  , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+40  , (uint32_t)irq5  , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+48  , (uint32_t)irq6  , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+56  , (uint32_t)irq7  , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+64  , (uint32_t)irq8  , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+72  , (uint32_t)irq9  , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+80  , (uint32_t)irq10 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+88  , (uint32_t)irq11 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+104 , (uint32_t)irq13 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+112 , (uint32_t)irq14 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+120 , (uint32_t)irq15 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 

	p = (uint8_t*) kernel_idt;
	add_idt_descriptor(p     , (uint32_t)except0 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+8   , (uint32_t)except1 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+16  , (uint32_t)except2 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+24  , (uint32_t)except3 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+32  , (uint32_t)except4 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+40  , (uint32_t)except5 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+48  , (uint32_t)except6 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+56  , (uint32_t)except7 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+64  , (uint32_t)except8 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+72  , (uint32_t)except9 , 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+80  , (uint32_t)except10, 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+88  , (uint32_t)except11, 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+96  , (uint32_t)except12, 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+104 , (uint32_t)except13, 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+112 , (uint32_t)except14, 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+128 , (uint32_t)except16, 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+136 , (uint32_t)except17, 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+144 , (uint32_t)except18, 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	add_idt_descriptor(p+152 , (uint32_t)except19, 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 
	
	add_idt_descriptor(p+1024 , (uint32_t)inter128, 0x08, IDT_PRESENT | IDT_DPL_0 | IDT_INT_GATE); 

	load_idt(kernel_idt);
}

void page_fault_handler()
{
}

void default_handler()
{
}

void irq_handler(int num)
{
	if (num >= 8)
		outb(0xA0, 0x20);
	outb(0x20, 0x20); // EOI

	if (num == 0)
	{
		clock_tick();
	}
	else if (num == 1)
	{
		kb_receive_scancode((uint8_t) inb(0x60));
	}
	else
	{
		printf("Received IRQ %d", num);
	}
}

