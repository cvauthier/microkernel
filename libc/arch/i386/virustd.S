.section .text

.globl wait # int wait()
.globl fork # int fork()
.globl exit # void exit(int code)

wait:
	mov $0, %eax
	int $0x80
	ret

fork:
	mov $1, %eax
	int $0x80
	ret

exit:
	mov 4(%esp), %ebx
	mov $2, %eax
	int $0x80


