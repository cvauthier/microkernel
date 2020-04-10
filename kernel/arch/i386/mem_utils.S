.section .text
.globl load_gdt
.globl load_idt
.globl tss_flush

gdtr:
.word 0 # GDT size
.long 0 # Base
 
load_gdt:
	mov 4(%esp), %eax
	mov %eax, gdtr+2
	mov 8(%esp), %eax
	mov %ax, gdtr
	lgdt gdtr
	jmp $0x08,$reload_CS
reload_CS:
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	ret

.word 255
idt: .long 0 # Address

load_idt:
	mov 4(%esp), %eax
	mov %eax, idt
	lidt idt
	ret

tss_flush:
	mov 0x2B, %ax # 0x2B = 0x28 (index) | 0x3 (RPL)
	ltr %ax
	ret

