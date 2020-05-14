#include "eval.h"
#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_simple_command(simple_cmd_t *cmd)
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

void free_command(cmd_t *cmd)
{
	if (cmd->op1) free_command(cmd->op1);
	if (cmd->op2) free_command(cmd->op2);
	if (cmd->scmd) free_simple_command(cmd->scmd);
	free(cmd);
}

typedef struct { char *name; char *val; } var_t;
dynarray_t *var_list;

var_t *create_var(const char *name, const char *val)
{
	var_t *v = (var_t*) calloc(1,sizeof(var_t));
	v->name = copy_str(name);
	v->val = copy_str(val);
	return v;
}

void free_var(var_t *v)
{
	free(v->name);
	free(v->val);
	free(v);
}

const char *getvar(const char *name)
{
	for (int i = 0 ; i < var_list->size ; i++)
	{
		var_t *v = (var_t*) var_list->array[i];
		if (v && strcmp(v->name, name) == 0)
			return v->val;
	}
	return "";
}

void setvar(const char *name, const char *val)
{
	var_t *v;
	int index = 0;
	while (index < var_list->size)
	{
		v = (var_t*) var_list->array[index];
		if (v && strcmp(v->name, name) == 0)
			break;
		index++;
	}
	if (index == var_list->size)
	{
		if (strcmp(val, "") == 0)
			return;
		v = (var_t*) calloc(1,sizeof(var_t));
		dynarray_push(var_list, (void*) create_var(name, val));
		return;
	}
	if (strcmp(val, "") == 0)
	{
		var_list->array[index] = var_list->array[var_list->size-1];
		free_var(v);
		dynarray_pop(var_list);
		return;
	}
	free(v->val);
	v->val = (char*) malloc((strlen(val)+1)*sizeof(char));
	strcpy(v->val, val);
}

char *eval_str(const char *str)
{
	dynarray_t *stack = create_dynarray();
	
	while (*str)
	{
		char c = *str;
		if (c == '$')
		{
			str++;
			if (*str == '$')
			{
				dynarray_push(stack, (void*) '$');
				str++;
			}
			else
			{
				dynarray_t *var_name = create_dynarray();
				
				int paren = (*str == '(');
				if (paren) str++;
				if (isalpha(*str) || *str=='_')
				{
					while (*str && (isalpha(*str) |  *str == '_'))
						dynarray_push(var_name, (void*) *(str++));
				}
				else if (*str == '?')
					dynarray_push(var_name, (void*) *(str++));
				
				if (paren)
				{
					if (*str != ')')
					{
						printf("Error : unclosed parenthesis\n");
						free_dynarray(stack);
						free_dynarray(var_name);
						return 0;
					}
					str++;
				}

				char *name = dynarray_to_str(var_name);
				free_dynarray(var_name);
				const char *val = getvar(name);
				free(name);
				int i = 0;
				while (val[i])
					dynarray_push(stack, (void*) val[i++]);
			}
		}
		else
		{
			dynarray_push(stack, (void*) c);
			str++;
		}
	}

	char *res = dynarray_to_str(stack);
	free_dynarray(stack);
	return res;
}

int eval_args(simple_cmd_t *cmd)
{
	for (int i = 0 ; i < cmd->args->size ; i++)
	{
		char *arg = (char*) cmd->args->array[i];
		char *arg2 = (void*) eval_str(arg);
		if (!arg2)
			return -1;
		cmd->args->array[i] =	arg2;
		free(arg);
	}
	return 0;
}

int eval_simple_command(simple_cmd_t *cmd)
{
	if (!cmd->args->size)
		return 0;

	int code = 0;
	if (strcmp((char*) cmd->args->array[0], "cd") == 0)
	{
		if (cmd->args->size != 2)
		{
			printf("Error : cd must be applied to one (and only one) argument !\n");
			code = 1;
		}
		else
		{
			if (eval_args(cmd) < 0)
				return 1;
			const char *path = (const char*) cmd->args->array[1];
			if (chdir(path) < 0)
			{
				printf("Invalid directory : %s\n", path);
				code = 1;
			}
		}
	}
	else if (cmd->args->size >= 2 && strcmp((char*) cmd->args->array[1], "=") == 0)
	{
		if (cmd->args->size != 3)
		{
			printf("Error : set statement must be of the form NAME=VAL\n");
			code = 1;
		}
		else
		{
			char *name = (char*) cmd->args->array[0];
			int i = 0;
			while (name[i] && (isalpha(name[i]) || name[i] == '_'))
				i++;

			if (!name[i])
			{
				char *val = eval_str((char*) cmd->args->array[2]);
				if (val)
				{
					setvar(name, val);
					free(val);
				}
			}
			else
			{
				printf("Error : variable name cannot have character %c\n", name[i]);
				code = 1;
			}
		}
	}
	else
	{
		if (eval_args(cmd) < 0)
			return 0;
		dynarray_push(cmd->args, 0);
		int pid = fork();
		if (!pid)
		{
			exec((char*) cmd->args->array[0], (char**) cmd->args->array);
			
			char *arg0 = (char*) cmd->args->array[0];
			int n = strlen(arg0);
			const char *path = getvar("PATH");
			while (*path)
			{
				const char *next = strchr(path, ':');
				next = next ? next : path+strlen(path);
				
				char *new_arg = (char*) malloc((2+(next-path)+n)*sizeof(char));
				memcpy(new_arg,path,next-path);
				new_arg[next-path] = '/';
				strcpy(new_arg+(next-path)+1,arg0);

				cmd->args->array[0] = (void*) new_arg;
				exec(new_arg, (char**) cmd->args->array);

				free(new_arg);
				path = next;
			}
			
			printf("Command failed\n");
			exit(1);
		}
	
		int pid2 = -1;
		while (pid2 != pid)
			wait(&pid2, &code);
	}

	return code;
}

int eval_command(cmd_t *cmd)
{
	int code = 0;
	if (cmd->type == SimpleCmd)
	{
		code = eval_simple_command(cmd->scmd);
	}
	else if (cmd->type == Seq)
	{
		eval_command(cmd->op1);
		code = eval_command(cmd->op2);
	}
	else if (cmd->type == And)
	{
		code = eval_command(cmd->op1);
		code = (code == 0) ? eval_command(cmd->op2) : code;
	}
	else if (cmd->type == Or)
	{
		code = eval_command(cmd->op1);
		code = (code == 0) ? code : eval_command(cmd->op2);
	}
	else if (cmd->type == Pipe)
	{
		int fd[2];
		if (pipe(fd, 0) == 0)
		{
			int pid1 = fork();
			if (!pid1)
			{
				dup2(fd[0], 0); // 0 = stdout
				close(fd[1]);
				close(fd[0]);
				exit(eval_command(cmd->op1));
			}
			int pid2 = fork();
			if (!pid2)
			{
				dup2(fd[1], 1); // 1 = stdin
				close(fd[1]);
				close(fd[0]);
				exit(eval_command(cmd->op2));
			}
			close(fd[0]);
			close(fd[1]);
			for (int i = 0 ; i < 2 ; i++)
			{
				int pid, code2;
				wait(&pid, &code2);
				if (pid == pid2)
					code = code2;
			}
		}
	}

	if (cmd->type != None)
	{
		char *s = str_of_int(code);
		setvar("?", s);
		free(s);
	}
	return code;
}

