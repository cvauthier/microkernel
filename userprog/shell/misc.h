#ifndef MISC_H
#define MISC_H

#include <virux_lib.h>

char *working_dir();

int isalpha(char c);
int isnum(char c);

char *copy_str(const char *s);

char *str_of_int(int n);

char *dynarray_to_str(dynarray_t *arr);

#endif
