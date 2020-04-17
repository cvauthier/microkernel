.section .bss, "aw", @nobits
.globl temp_page_1
.globl temp_page_2
.globl temp_page_3
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
.globl jmp_user_proc # void jmp_user_proc(uint32_t *stack)

jmp_user_proc:
	mov 4(%esp), %esp
	popal
	push %eax
	mov $0x23, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	pop %eax
	iret

