#ifndef PARSER_H
#define PARSER_H

#include <virux_lib.h>

#include "eval.h"

struct token_t;
typedef struct token_t token_t;

void free_tok(token_t *t);
token_t *create_token(int type, char *s);

void free_tokenlist(dynarray_t *arr);
dynarray_t *tokenize(int *eof, int *error);

cmd_t *parse_simple_cmd_rec(dynarray_t *tlist, int *i, int *error);
cmd_t *parse_cmd_rec(dynarray_t *tlist, int *i, int *error);

cmd_t *parse_command(int *eof);

#endif
