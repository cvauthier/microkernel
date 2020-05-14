#include <stdlib.h>
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

static unsigned int next = 0;

void srand(unsigned int seed)
{
	next = seed;
}

int rand()
{
	next = next * 1103515245 + 12345;
	return (int) (next/65536) % (RAND_MAX+1);
}

