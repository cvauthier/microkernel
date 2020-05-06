#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>
#include <stdarg.h>

#define EOF (-1)

struct FILE;
typedef struct FILE FILE;

extern FILE *stdout;
extern FILE *stdin;
extern FILE *stderr;

#ifdef __cplusplus
extern "C" {
#endif

void fclose(FILE *stream);
FILE *fopen(const char *filename, const char *mode);

int fprintf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list parameters);
int vprintf(const char *format, va_list parameters);

int fgetc(FILE *stream);
char *fgets(char *str, int num, FILE *stream);
int fputc(int ic, FILE *stream);
int getchar();
int putchar(int ic);
int puts(const char *string);

int feof(FILE *stream);
int ferror(FILE *stream);

#ifdef __cplusplus
}
#endif

#endif
