#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <virux_lib.h>

typedef struct
{
	dynarray_t *args;
} simple_cmd_t;

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

char *parse_token(int *eol, int *eof, int *error)
{
	int c = ' ';

	while (c == ' ' || c == '\t')
	{
		c = getchar();
		if (c < 0)
		{
			*eof = *eol = 1;
			return 0;
		}
		if (c == '\n')
		{
			*eol = 1;
			return 0;
		}
	}

	dynarray_t *str = create_dynarray();
	
	if (c == '"')
	{
		c = getchar();
		while (c != '"')
		{
			if (c == '\\')
			{
				c = getchar();
				c = (c == '"') ? '"' : (c == '\\') ? '\\' : (c == 'n') ? '\n' : (c == 't') ? '\t' : c;
			}

			if (c < 0 || c == '\n')
			{
				*eol = 1;
				if (c < 0) *eof = 1;
				*error = 1;
				printf("Syntax error : unclosed quote\n");
				free_dynarray(str);
				return 0;
			}

			dynarray_push(str, (void*) c);
			c = getchar();
		}
	}
	else
	{
		do
		{
			dynarray_push(str, (void*) c);
			c = getchar();
		} while (c != ' ' && c != '\t' && c != '\n' && c >= 0);
		if (c == '\n')
			*eol = 1;
		if (c < 0)
			*eof = *eol = 1;
	}

	char *tok = (char*) malloc((str->size+1)*sizeof(char));
	for (int i = 0 ; i < str->size ; i++)
		tok[i] = (char) str->array[i];
	tok[str->size] = 0;
	free_dynarray(str);
	
	return tok;
}

simple_cmd_t *parse_simple_cmd(int *eof)
{
	simple_cmd_t *cmd = (simple_cmd_t*) calloc(1, sizeof(simple_cmd_t));
	cmd->args = create_dynarray();

	int eol = 0, error = 0;

	while (!eol)
	{
		char *arg = parse_token(&eol, eof, &error);
		if (arg)
		{
			dynarray_push(cmd->args, arg);
		}
		if (error)
		{
			free_simple_cmd(cmd);
			return 0;
		}
	}

	return cmd;
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

