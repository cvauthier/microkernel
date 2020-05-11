.section .text
.global _start
.type _start, @function

_start:
	call _init
	call main
	push %eax
	call exit

.size _start, . - _start
