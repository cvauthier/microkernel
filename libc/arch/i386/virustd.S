.section .text

.globl wait 
.globl fork
.globl exit
.globl open
.globl close 
.globl write
.globl read 
.globl seek
.globl sbrk 
.globl exec
.globl getcwd 
.globl chdir 
.globl dup 
.globl dup2

#include <kernel/syscall_codes.h>

wait:
	mov $SYSCALL_WAIT, %eax
	jmp syscall

fork:
	mov $SYSCALL_FORK, %eax
	jmp syscall

exit:
	mov 4(%esp), %ebx
	mov $SYSCALL_EXIT, %eax
	int $0x80

open:
	mov $SYSCALL_OPEN, %eax
	jmp syscall

close:
	mov $SYSCALL_CLOSE, %eax
	jmp syscall

write:
	mov $SYSCALL_WRITE, %eax
	jmp syscall

read:
	mov $SYSCALL_READ, %eax
	jmp syscall

seek:
	mov $SYSCALL_SEEK, %eax
	jmp syscall

sbrk:
	mov $SYSCALL_SBRK, %eax
	jmp syscall

exec:
	mov $SYSCALL_EXEC, %eax
	jmp syscall

getcwd:
	mov $SYSCALL_GETCWD, %eax
	jmp syscall

chdir:
	mov $SYSCALL_CHDIR, %eax
	jmp syscall

dup:
	mov $SYSCALL_DUP, %eax
	jmp syscall

dup2:
	mov $SYSCALL_DUP2, %eax
	jmp syscall

syscall:
	push %ebp
	push %ebx
	push %edx
	mov %esp,%ebp
	mov 16(%esp),%ebx
	mov 20(%esp), %ecx
	mov 24(%esp), %edx
	int $0x80
	mov %ebp, %esp
	pop %edx
	pop %ebx
	pop %ebp
	ret


