#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <virux_lib.h>

#include <kernel/tty.h>
#include <kernel/memory.h>
#include <kernel/timer.h>
#include <kernel/process.h>
#include <kernel/filesystem.h>
#include <kernel/keyboard.h>

#include <kernel/multiboot.h>

// extern void test_main();

void install_std_streams()
{
	file_descr_t *term = current_terminal();
	for (int i = 0 ; i < 3 ; i++)
		dynarray_push(proc_list[cur_pid]->files, (void*) term);
	term->owners+=3;
}

int some_task(__attribute__((unused)) void *unused)
{
	install_std_streams();

	if (chdir("bin") < 0)
		printf("Failed to chdir to bin\n");

	printf("Trying to execute test...\n");
	exec("test", 0);
	printf("Failed\n");

	return 0;
}

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

	kernel_proc(slayer, 0);
	kernel_proc(some_task, 0);

	while (1);
	return 0;
}

void kernel_main(multiboot_info_t* mbd, __attribute__((unused)) unsigned int magic)
{
	terminal_basic_init();
	memory_setup(mbd);
	init_keyboard();
	terminal_init();
	clock_init();
	scheduler_init();	

	kernel_proc(idle, 0);
	schedule();
	first_context_switch(proc_list[cur_pid]);
}

