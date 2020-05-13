.section .text
.globl jump_to_ring3 # void jump_to_ring3(void (*start)(), stackint_t *stack)
.globl switch_proc # void switch(hw_context_t *prev, hw_context_t *next)
.globl switch_initial # void switch_initial(hw_context_t *init)
.globl kernel_proc_start # void kernel_proc_start()
.globl forked_proc_start # void forked_proc_start();
.globl enable_interrupts # void enable_interrupts();
.globl disable_interrupts # void disable_interrupts();

jump_to_ring3:
	cli
	mov 4(%esp), %ebx # start
	mov 8(%esp), %ecx # stack
	push $0x23 # User SS
	push %ecx  # User stack
	pushf
	pop %edx
	or $0x0200, %edx # On active les interruptions
	push %edx
	push $0x1B # User CS
	push %ebx # EIP à dépiler
	mov $0x23, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	iret

switch_proc:
	push %ebp
	mov %esp, %ebp
	mov 8(%esp), %eax # prev
	mov 12(%esp), %edx # next
	pushfl								# Sauvegarde eflags,ebp,etc.
	pushal
	push %ebp
	lea 1f, %ecx
	mov %ecx, (%eax) 		# 1f -> prev->eip
	mov %esp, 4(%eax) 		# esp -> prev->esp
	mov 4(%edx), %esp 		# next->esp -> esp
	jmp *(%edx) 					# Saut à next->eip
1:pop %ebp
	popal
	popfl
	mov %ebp, %esp
	pop %ebp
	ret

switch_initial:
	mov 4(%esp), %edx
	mov 4(%edx), %esp 		# init->esp -> esp
	jmp *(%edx) 					# Saut à init->eip

kernel_proc_start:
	sti
	pop %eax
	call *%eax
	push %eax
	call exit

forked_proc_start:
	popal
	iret

disable_interrupts:
	cli
	ret

enable_interrupts:
	sti
	ret

