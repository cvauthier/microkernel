#include <stdio.h>
#include <stdlib.h>

#include <virustd.h>

#include <kernel/tty.h>
#include <kernel/memory.h>
#include <kernel/timer.h>
#include <kernel/process.h>

#include <multiboot.h>

extern void test_main();

int slayer(__attribute__((unused)) void *unused)
{
	int foo, bar;
	while (1)
		wait(&foo, &bar);
	return 0;
}

int idle(__attribute__((unused)) void *unused)
{
	scheduling_on = 1;
	printf("Hello kernel world !\n");
	test_main();
	
	kernel_proc(slayer, 0);

	while (1);
	
	return 0;
}

void kernel_main(multiboot_info_t* mbd, unsigned int magic)
{
	memory_setup(mbd);
	clock_init();
	terminal_initialize();
	
	cur_pid = -1;

	kernel_proc(idle, 0);
	reschedule();
}

