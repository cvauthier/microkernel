.section .text
.globl load_page_directory
.globl enable_paging
.globl load_gdt

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

load_page_directory:
	mov 4(%esp), %eax
	mov %eax, %cr3
	ret

enable_paging:
	push %ebp
	mov %esp, %ebp
	mov %cr0, %eax
	or $0x80000000, %eax
	mov %eax, %cr0
	mov %ebp, %esp
	pop %ebp
	ret

