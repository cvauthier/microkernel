#ifndef PARSER_H
#define PARSER_H

#include <virux_lib.h>

typedef struct
{
	dynarray_t *args;
} simple_cmd_t;

char *parse_token(int *eol, int *eof, int *error);
simple_cmd_t *parse_simple_cmd(int *eof);

#endif
