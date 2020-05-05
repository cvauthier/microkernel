#ifndef _STRING_H
#define _STRING_H 1

#include <sys/cdefs.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);

char *strcpy (char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t num);
const char *strchr(const char *str, int c);
size_t strlen(const char*);
int strcmp(const char *lhs, const char *rhs);

#ifdef __cplusplus
}
#endif

#endif
