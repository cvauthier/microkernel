#include <virux_lib.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#endif

__attribute__((__noreturn__))
void abort(void) 
{
#if defined(__is_libk)
	terminal_clear();
	terminal_writestring("ERROR\n-----\nThe kernel has encountered an unexpected error and cannot continue execution\n");
	while (1);
#else
	exit(6);
#endif
	__builtin_unreachable();
}
