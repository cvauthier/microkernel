.section .text

.globl wait # int wait(int *pid, int *code)
.globl fork # int fork()
.globl exit # void exit(int code)
.globl open # int open(const char *path)
.globl close # void close(int fd)
.globl write # ssize_t write(int fd, void *ptr, size_t count)
.globl read # ssize_t read(int fd, void *buffer, size_t count)
.globl seek # uint32_t seek(int fd, int32_t pos, int flag)

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

open:
	push %ebx
	mov $3, %eax
	mov 8(%esp), %ebx
	int $0x80
	pop %ebx
	ret

close:
	push %ebx
	mov $4, %eax
	mov 8(%esp), %ebx
	int $0x80
	pop %ebx
	ret

read:
	push %ebx
	mov $5, %eax
	mov 8(%esp), %ebx
	mov 12(%esp), %ecx
	mov 16(%esp), %edx
	int $0x80
	pop %ebx
	ret

write:
	push %ebx
	mov $6, %eax
	mov 8(%esp), %ebx
	mov 12(%esp), %ecx
	mov 16(%esp), %edx
	int $0x80
	pop %ebx
	ret

seek:
	push %ebx
	mov $7, %eax
	mov 8(%esp), %ebx
	mov 12(%esp), %ecx
	mov 16(%esp), %edx
	int $0x80
	pop %ebx
	ret

