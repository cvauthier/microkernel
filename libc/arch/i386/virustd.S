.section .text

.globl wait # int wait(int *pid, int *code)
.globl fork # int fork()
.globl exit # void exit(int code)

wait:
	push %ebp
	push %ebx
	mov %esp, %ebp
	mov $0, %eax
	int $0x80
	mov 12(%ebp), %edx # pid -> edx
	mov %ebx, (%edx) # *pid = ebx
	mov 16(%ebp), %edx # code -> edx
	mov %ecx, (%edx) # *code = ecx
	mov %ebp, %esp
	pop %ebx
	pop %ebp
	ret

fork:
	mov $1, %eax
	int $0x80
	ret

exit:
	mov 4(%esp), %ebx
	mov $2, %eax
	int $0x80


