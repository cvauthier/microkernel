.section .bss, "aw", @nobits
.globl temp_page_1
.globl temp_page_2
.globl temp_page_3
.globl temp_page_4
.globl temp_page_5
.globl temp_page_6
.align 4096
temp_page_1:
.skip 4096
temp_page_2:
.skip 4096
temp_page_3:
.skip 4096
temp_page_4:
.skip 4096
temp_page_5:
.skip 4096
temp_page_6:
.skip 4096

.section .text
.globl load_gdt # void load_gdt(vaddr_t gdt_addr, uint16_t size)
.globl load_idt # void load_idt(vaddr_t idt_addr)
.globl tss_flush # void tss_flush()
.globl load_pd # void load_pd(paddr_t pd_addr)
.globl tlb_flush # void tlb_flush()
.globl outb # void outb(uint16_t port, uint8_t val)
.globl inb # uint8_t inb(uint16_t port)
.globl get_cr2 # uint32_t get_cr2()

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

idt:
.word 256*8-1
.long 0 # Address

load_idt:
	mov 4(%esp), %eax
	mov %eax, idt+2
	lidt idt
	ret

tss_flush:
	mov $0x2B, %ax # 0x2B = 0x28 (index) | 0x3 (RPL)
	ltr %ax
	ret

load_pd:
	mov 4(%esp), %eax
	mov %eax, %cr3
	ret

tlb_flush:
	mov %cr3,%eax
	mov %eax,%cr3
	ret

outb:
	mov 4(%esp), %edx
	mov 8(%esp), %eax
	outb %al,%dx
	ret

inb:
	xor %eax, %eax
	mov 4(%esp), %edx
	inb %dx,%al
	ret

get_cr2:
	mov %cr2, %eax
	ret


