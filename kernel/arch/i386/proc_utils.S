.section .text
.globl jmp_user_proc # void jmp_user_proc(uint32_t *stack)
.globl switch_proc # void switch(proc_data_t *prev, proc_data_t *next)
.globl switch_initial # void switch_initial(proc_data_t *init)
.globl kernel_proc_start # void kernel_proc_start()
.globl fork_setup # int fork_setup(proc_data_t *original, proc_data_t *child, int child_pid)

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

switch_proc:
	push %ebp
	mov %esp, %ebp
	mov 8(%esp), %eax # prev
	mov 12(%esp), %edx # next
	pushfl								# Sauvegarde eflags,ebp,etc.
	pushal
	push %ebp
	lea 1f, %ecx
	mov %ecx, 8(%eax) 		# 1f -> prev->eip
	mov %esp, 12(%eax) 		# esp -> prev->esp
	mov 12(%edx), %esp 		# next->esp -> esp
	jmp *8(%edx) 					# Saut à next->eip
1:pop %ebp
	popal
	popfl
	mov %ebp, %esp
	pop %ebp
	ret

switch_initial:
	mov 4(%esp), %edx
	mov 12(%edx), %esp 		# init->esp -> esp
	jmp *8(%edx) 					# Saut à init->eip

kernel_proc_start:
	sti
	pop %eax
	call *%eax
	push %eax
	call exit

