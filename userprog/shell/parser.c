#include "parser.h"
#include "misc.h"

#include <stdio.h>

enum { Tok_String, Tok_Pipe, Tok_And, Tok_Or, Tok_Seq, Tok_LPar, Tok_RPar };

struct token_t
{
	int type;
	char *s;
};

void free_tok(token_t *t)
{
	if (t->type == Tok_String) free(t->s);
	free(t);
}

token_t *create_token(int type, char *s)
{
	token_t *t = (token_t*) malloc(sizeof(token_t));
	t->type = type;
	t->s = s;
	return t;
}

void free_tokenlist(dynarray_t *arr)
{
	for (int i = 0 ; i < arr->size ; i++)
		free_tok((token_t*) arr->array[i]);
	free_dynarray(arr);
}

// Garanti : si *error = 1, alors la valeur de retour est 0
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

	if (c == '(' || c == ')' || c == '=')
	{
		char *res = (char*) malloc(2*sizeof(char));
		res[0] = c;
		res[1] = 0;
		return res;
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
		} while (c != ' ' && c != '\t' && c != '\n' && c != '(' && c != ')' && c != '=' && c >= 0);
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

void process_token(dynarray_t *tlist, char *t)
{
	if (strcmp(t,"&&") == 0)
		dynarray_push(tlist, (void*) create_token(Tok_And, 0));
	else if (strcmp(t,"||") == 0)
		dynarray_push(tlist, (void*) create_token(Tok_Or, 0));
	else if (strcmp(t,";") == 0)
		dynarray_push(tlist, (void*) create_token(Tok_Seq, 0));
	else if (strcmp(t,"|") == 0)
		dynarray_push(tlist, (void*) create_token(Tok_Pipe, 0));
	else if (strcmp(t,"(") == 0)
		dynarray_push(tlist, (void*) create_token(Tok_LPar, 0));
	else if (strcmp(t,")") == 0)
		dynarray_push(tlist, (void*) create_token(Tok_RPar, 0));
	else
		dynarray_push(tlist, (void*) create_token(Tok_String, t));
	if (((token_t*)tlist->array[tlist->size-1])->s == 0)
		free(t);
}

char *str_of_char(char c)
{
	char *res = (char*) calloc(2,sizeof(char));
	res[0] = c;
	return res;
}

// Garanti : si *error = 1, alors la valeur de retour est 0
dynarray_t *tokenize(int *eof, int *error)
{
	dynarray_t *tlist = create_dynarray();
	int c = 0;
	int eol = 0;

	while (!eol)
	{
		c = getchar();
		if (c == '\n' || c == '\t' || c == ' ' || c < 0)
		{
			eol = (c == '\n') || (c < 0);
			*eof = (c < 0);
			continue;
		}

		if (c == '(' || c == ')' || c == '=')
		{
			process_token(tlist, str_of_char(c));
			continue;
		}

		dynarray_t *stack = create_dynarray();
	
		if (c == '"')
		{
			c = getchar();
			while (c != '"')
			{
				if (c < 0 || c == '\n')
				{
					*eof = (c < 0);
					*error = 1;
					printf("Syntax error : unclosed quote\n");
					free_dynarray(stack);
					free_tokenlist(tlist);
					return 0;
				}

				dynarray_push(stack, (void*) c);
				c = getchar();
			}
		}
		else
		{
			do
			{
				dynarray_push(stack, (void*) c);
				c = getchar();
			} while (c != ' ' && c != '\t' && c != '\n' && c != '(' && c != ')' && c != '=' && c >= 0);
		
			eol = (c == '\n') || (c < 0);
			*eof = (c < 0);
		}

		process_token(tlist, dynarray_to_str(stack));
		free_dynarray(stack);
	}
	
	return tlist;
}

#define T(x,i) ((token_t*) x->array[*i])->type
#define S(x,i) ((token_t*) x->array[*i])->s

cmd_t *parse_simple_cmd_rec(dynarray_t *tlist, int *i, int *error)
{
	if (T(tlist,i) == Tok_LPar)
	{
		(*i)++;
		cmd_t *cmd = parse_cmd_rec(tlist, i, error);
		if (*error)
			return 0;
		if (*i == tlist->size || T(tlist,i) != Tok_RPar)
		{
			printf("Syntax error : unclosed parenthesis\n");
			*error = 1;
			free_command(cmd);
			return 0;
		}
		(*i)++;
		return cmd;
	}

	if (*i == tlist->size || T(tlist,i) != Tok_String)
	{
		printf("Syntax error\n");
		*error = 1;
		return 0;
	}

	simple_cmd_t *scmd = (simple_cmd_t*) calloc(1, sizeof(simple_cmd_t));
	scmd->args = create_dynarray();

	while (*i < tlist->size && T(tlist,i) == Tok_String)
	{
		dynarray_push(scmd->args, (void*) copy_str(S(tlist,i)));
		(*i)++;
	}

	cmd_t *cmd = (cmd_t*) calloc(1, sizeof(cmd_t));
	cmd->scmd = scmd;
	cmd->type = SimpleCmd;
	return cmd;
}

cmd_t *parse_cmd_rec(dynarray_t *tlist, int *i, int *error)
{
	cmd_t *cmd = parse_simple_cmd_rec(tlist, i, error);
	if (*error)
		return 0;
	
	if (*i == tlist->size)
		return cmd;
	
	while (*i < tlist->size && T(tlist,i) != Tok_RPar)
	{
		if (T(tlist,i) == Tok_And || T(tlist,i) == Tok_Or || T(tlist,i) == Tok_Pipe || T(tlist,i) == Tok_Seq)
		{
			int type = 0;
			switch (T(tlist,i))
			{
				case Tok_And:  type = And; break;
				case Tok_Or:   type = Or; break;
				case Tok_Pipe: type = Pipe; break;
				case Tok_Seq:  type = Seq; break;
				default: break;
			}
			(*i)++;
			cmd_t *cmd2 = parse_simple_cmd_rec(tlist, i, error);
			if (*error)
			{
				free_command(cmd);
				return 0;
			}

			cmd_t *new_cmd = (cmd_t*) calloc(1,sizeof(cmd_t));
			new_cmd->type = type;
			new_cmd->op1 = cmd;
			new_cmd->op2 = cmd2;
			cmd = new_cmd;
		}
		else
		{
			printf("Syntax error : unexpected token\n");
			*error = 1;
			free_command(cmd);
			return 0;
		}
	}

	return cmd;
}

cmd_t *parse_command(int *eof)
{
	int error = 0;
	dynarray_t *tlist = tokenize(eof, &error);
	cmd_t *cmd;

	if (!error)
	{
		int i = 0;
		cmd = parse_cmd_rec(tlist, &i, &error);
		free_tokenlist(tlist);
	}
	if (error)
	{
		cmd = (cmd_t*) calloc(1,sizeof(cmd_t));
		cmd->type = None;
	}

	return cmd; 		
}


