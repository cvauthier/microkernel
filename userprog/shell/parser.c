#include "parser.h"

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

