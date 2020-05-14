#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <stddef.h>
#include <sys/cdefs.h>

#define RAND_MAX 32767

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((__noreturn__))
void abort(void);

void srand(unsigned int seed);
int rand();

void *calloc(size_t num, size_t size);
void *malloc(size_t size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
