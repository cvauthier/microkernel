#include <stdio.h>
#include <stdlib.h>

#include <kernel/tty.h>
#include <kernel/memory.h>

extern void test_main();
extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

extern int xkcd;

void kernel_main(void) 
{
	memory_setup();
	terminal_initialize();
	
	printf("Hello, kernel World!\n\n");
	test_main();

	for (;;)
		asm("hlt");
}

