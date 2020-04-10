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
	pushal
	cld
	call default_handler
	popal
	iret

except1:
	pushal
	cld
	call default_handler
	popal
	iret

except2:
	pushal
	cld
	call default_handler
	popal
	iret

except3:
	pushal
	cld
	call default_handler
	popal
	iret

except4:
	pushal
	cld
	call default_handler
	popal
	iret

except5:
	pushal
	cld
	call default_handler
	popal
	iret

except6:
	pushal
	cld
	call default_handler
	popal
	iret

except7:
	pushal
	cld
	call default_handler
	popal
	iret

except8:
	add 4,%esp
	pushal
	cld
	call default_handler
	popal
	iret

except9:
	pushal
	cld
	call default_handler
	popal
	iret

except10:
	add 4,%esp
	pushal
	cld
	call default_handler
	popal
	iret

except11:
	pushal
	cld
	call default_handler
	popal
	iret

except12:
	pushal
	cld
	call default_handler
	popal
	iret

except13:
	pushal
	cld
	call default_handler
	popal
	iret

except14:
	add 4,%esp
	pushal
	cld
	call page_fault_handler
	popal
	iret

except16:
	pushal
	cld
	call default_handler
	popal
	iret

except17:
	pushal
	cld
	call default_handler
	popal
	iret

except18:
	pushal
	cld
	call default_handler
	popal
	iret

except19:
	pushal
	cld
	call default_handler
	popal
	iret

irq0:
	pushal
	mov 0, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq1:
	pushal
	mov 1, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq2:
	pushal
	mov 2, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq3:
	pushal
	mov 3, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq4:
	pushal
	mov 4, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq5:
	pushal
	mov 5, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq6:
	pushal
	mov 6, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq7:
	pushal
	mov 7, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq8:
	pushal
	mov 8, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq9:
	pushal
	mov 9, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq10:
	pushal
	mov 10, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq11:
	pushal
	mov 11, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq12:
	pushal
	mov 12, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq13:
	pushal
	mov 13, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq14:
	pushal
	mov 14, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

irq15:
	pushal
	mov 15, %eax
	push %eax
	call irq_handler
	pop %eax
	popal
	iret

