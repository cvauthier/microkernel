#ifndef EVAL_H
#define EVAL_H

#include <stdlib.h>
#include <string.h>
#include <virux_lib.h>

typedef struct
{
	dynarray_t *args;
} simple_cmd_t;

enum { SimpleCmd, Seq, And, Or, Pipe, None };

struct cmd_t
{
	int type;
	struct cmd_t *op1;
	struct cmd_t *op2;
	simple_cmd_t *scmd;
};
typedef struct cmd_t cmd_t;

extern dynarray_t *var_list;

void free_simple_command(simple_cmd_t *cmd);
void free_command(cmd_t *cmd);

const char *getvar(const char *name);
void setvar(const char *name, const char *val);

char *eval_str(const char *str);

int eval_simple_command(simple_cmd_t *cmd);
int eval_command(cmd_t *cmd);

#endif
