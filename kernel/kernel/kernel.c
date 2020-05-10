#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <virustd.h>

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
}

int some_task(__attribute__((unused)) void *unused)
{
	install_std_streams();

	char path[100] = "/usr/include/stdio.h";
	
	printf("What file to open ?\n");

	if (fgets(path, 100, stdin))
	{
		char *c = strchr(path, '\n');
		if (c)
			path[c-path] = 0;
	}

	printf("Trying to open %s...\n",path);
	
	FILE *f = fopen(path, "r");
	if (!f)
		printf("Failed\n");
	else
	{
		printf("Success :\n");
		char buffer[50];
		for (int i = 0 ; i < 10 ; i++)
		{
			fgets(buffer, 50, f);
			printf("%s", buffer);
		}
		printf("\n");
	}

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

