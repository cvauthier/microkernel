#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <virux_lib.h>

#include "parser.h"

void free_simple_cmd(simple_cmd_t *cmd)
{
	for (int i = 0 ; i < cmd->args->size ; i++)
	{
		char *arg = (char*) cmd->args->array[i];
		if (arg)
			free(arg);
	}
	free_dynarray(cmd->args);
	free(cmd);
}

char *working_dir()
{
	int size = 100;
	char *cwd = (char*) malloc(size*sizeof(char));
	
	while (cwd && !getcwd(cwd, size))
	{
		size *= 2;
		free(cwd);
		cwd = (char*) malloc(size*sizeof(char));
	}

	return cwd;
}

int exec_simple_cmd(simple_cmd_t *cmd)
{
	if (!cmd->args->size)
		return 0;

	int code = 0;
	if (strcmp((char*) cmd->args->array[0], "cd") == 0)
	{
		if (cmd->args->size != 2)
		{
			printf("Error : cd must be applied to one (and only one) argument !\n");
			return 1;
		}
		const char *path = (const char*) cmd->args->array[1];
		if (chdir(path) < 0)
		{
			printf("Invalid directory : %s\n", path);
			return 1;
		}
	}
	else
	{
		dynarray_push(cmd->args, 0);
		int pid = fork();
		if (!pid)
		{
			exec((char*) cmd->args->array[0], (char**) cmd->args->array);
			printf("Command failed\n");
			exit(1);
		}
	
		int pid2 = -1;
		while (pid2 != pid)
			wait(&pid2, &code);
		printf("Exited with code %d\n", code);
	}

	return code;
}

int main(int argc, char **argv)
{
	printf("--- Virux shell v0.0\n");
	int cont = 1;

	while (cont)
	{
		char *cwd = working_dir();
		printf("%s$ ", cwd);
		free(cwd);

		simple_cmd_t *cmd = parse_simple_cmd(&cont);
		exec_simple_cmd(cmd);
		free_simple_cmd(cmd);
	}
	return 0;
}

