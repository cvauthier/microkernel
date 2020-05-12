.section .text

.globl except0
.globl except1
.globl except2
.globl except3
.globl except4
.globl except5
.globl except6
.globl except7
.globl except8
.globl except9
.globl except10
.globl except11
.globl except12
.globl except13
.globl except14
.globl except16
.globl except17
.globl except18
.globl except19
.globl inter128
.globl irq0
.globl irq1
.globl irq2
.globl irq3
.globl irq4
.globl irq5
.globl irq6
.globl irq7
.globl irq8
.globl irq9
.globl irq10
.globl irq11
.globl irq12
.globl irq13
.globl irq14
.globl irq15

except0:
	cli
	pushal
	call default_handler
	popal
	iret

except1:
	cli
	pushal
	call default_handler
	popal
	iret

except2:
	cli
	pushal
	call default_handler
	popal
	iret

except3:
	cli
	pushal
	call default_handler
	popal
	iret

except4:
	cli
	pushal
	call default_handler
	popal
	iret

except5:
	cli
	pushal
	call default_handler
	popal
	iret

except6:
	cli
	pushal
	call default_handler
	popal
	iret

except7:
	cli
	pushal
	call default_handler
	popal
	iret

except8:
	cli
	add $4,%esp
	pushal
	call default_handler
	popal
	iret

except9:
	cli
	pushal
	call default_handler
	popal
	iret

except10:
	cli
	add $4,%esp
	pushal
	call default_handler
	popal
	iret

except11:
	cli
	pushal
	call default_handler
	popal
	iret

except12:
	cli
	pushal
	call default_handler
	popal
	iret

except13:
	cli
	pushal
	call default_handler
	popal
	iret

except14:
	cli
	add $4,%esp
	pushal
	call page_fault_handler
	popal
	iret

except16:
	cli
	pushal
	call default_handler
	popal
	iret

except17:
	cli
	pushal
	call default_handler
	popal
	iret

except18:
	cli
	pushal
	call default_handler
	popal
	iret

except19:
	cli
	pushal
	call default_handler
	popal
	iret

#include <kernel/syscall_codes.h>

inter128:
	cli
	pushal
	mov %esp, %ebp
	
	mov 28(%ebp), %eax
	cmp $NB_SYSCALLS, %eax
	jge .invalid_call
	cmp $0, %eax
	jl .invalid_call
	
	mov syscalls_addr(,%eax,4), %eax
	
	# On empile ebx, ecx, edx
	push 20(%ebp)
	push 24(%ebp)
	push 16(%ebp)
	call *%eax
	jmp .end_call

.invalid_call:
	mov $-1, %eax
.end_call:
	mov %eax, 28(%ebp)
	mov %ebp, %esp
	popal
	iret

irq0:
	cli
	pushal
	mov $0, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq1:
	cli
	pushal
	mov $1, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq2:
	cli
	pushal
	mov $2, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq3:
	cli
	pushal
	mov $3, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq4:
	cli
	pushal
	mov $4, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq5:
	cli
	pushal
	mov $5, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq6:
	cli
	pushal
	mov $6, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq7:
	cli
	pushal
	mov $7, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq8:
	cli
	pushal
	mov $8, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq9:
	cli
	pushal
	mov $9, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq10:
	cli
	pushal
	mov $10, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq11:
	cli
	pushal
	mov $11, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq12:
	cli
	pushal
	mov $12, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq13:
	cli
	pushal
	mov $13, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq14:
	cli
	pushal
	mov $14, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq15:
	cli
	pushal
	mov $15, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

