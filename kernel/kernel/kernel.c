#include <stdio.h>
#include <stdlib.h>

#include <virustd.h>

#include <kernel/tty.h>
#include <kernel/memory.h>
#include <kernel/timer.h>
#include <kernel/process.h>
#include <kernel/filesystem.h>
#include <kernel/keyboard.h>

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

	char path[100] = "/usr/include/stdio.h";
	size_t nbread;

	printf("What file to open ?\n");

	file_descr_t *term = current_terminal();
	while (!(nbread = term->read(term, path, 99)));
	path[nbread-1] = 0;

	printf("Trying to open %s...\n",path);
	int fd = open(path);
	if (fd < 0)
		printf("Failed\n");
	else
	{
		char buffer[51];
		buffer[50] = 0;
		for (int i = 0 ; i < 4 ; i++)
		{
			read(fd, (void*) buffer, 50);
			printf("%s", buffer);
		}
		printf("\n");
	}

	kernel_proc(slayer, 0);

	while (1);
	
	return 0;
}

void kernel_main(multiboot_info_t* mbd, unsigned int magic)
{
	terminal_basic_init();
	memory_setup(mbd);
	init_keyboard();
	terminal_init();

	clock_init();
	
	cur_pid = -1;

	kernel_proc(idle, 0);
	reschedule();
}

