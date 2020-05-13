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

void install_std_streams()
{
	file_descr_t *term = current_terminal();
	for (int i = 0 ; i < 3 ; i++)
		dynarray_push(proc_list[cur_pid]->files, (void*) term);
	term->owners+=3;
}

int shell_starter(__attribute__((unused)) void *unused)
{
	install_std_streams();
	exec("bin/shell", 0);
	printf("Failed\n");
	return 1;
}

int shell_spawner(__attribute__((unused)) void *unused)
{
	install_std_streams();

	int code = 0;
	while (!code)
	{
		int pid;
		kernel_proc(shell_starter, 0);
		wait(&pid, &code);
	}

	printf("Error : can't load the shell\n");
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
	kernel_proc(shell_spawner, 0);

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

