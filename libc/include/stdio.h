#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>
#include <stdarg.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int vprintf(const char *format, va_list parameters);
int printf(const char *format, ...);
int putchar(int ic);
int puts(const char *string);

#ifdef __cplusplus
}
#endif

#endif
