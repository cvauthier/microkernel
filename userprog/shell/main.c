#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <virux_lib.h>

#include "parser.h"
#include "eval.h"
#include "misc.h"

int main(int argc, char **argv)
{
	printf("--- Virux shell v0.0\n");

	var_list = create_dynarray();
	int eof = 0;

	while (!eof)
	{
		char *cwd = working_dir();
		printf("virux:%s$ ", cwd);
		free(cwd);

		cmd_t *cmd = parse_command(&eof);
	
		if (cmd->type == SimpleCmd && cmd->scmd->args->size >= 1)
		{
			simple_cmd_t *scmd = cmd->scmd;
			char *name = (char*) scmd->args->array[0];
			
			if (strcmp(name, "exit") == 0)
			{
				free_command(cmd);
				break;
			}
		}

		int code = eval_command(cmd);
		
		free_command(cmd);
	}

	return 0;
}

