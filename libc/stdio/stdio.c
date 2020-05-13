#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <virux_lib.h>

#define FILE_EOF 1
#define FILE_ERROR 2

struct FILE
{
	int fd;
	int flags;
};

FILE *stdout = &(FILE){0,0};
FILE *stdin = &(FILE){1,0};
FILE *stderr = &(FILE){2,0};

void fclose(FILE *stream)
{
	close(stream->fd);
	free(stream);
}

FILE *fopen(const char *filename, __attribute__((unused)) const char *mode)
{
	int fd = open(filename, 0);
	if (fd < 0)
		return 0;

	FILE *f = (FILE*) calloc(1,sizeof(FILE));
	f->fd = fd;
	return f;
}

static bool print(FILE *stream, const char* data, size_t length) 
{
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (fputc(bytes[i], stream) == EOF)
			return false;
	return true;
}
 
int fprintf(FILE *stream, const char *restrict format, ...)
{
	va_list parameters;
	va_start(parameters, format);
	int written = vfprintf(stream, format, parameters);
	va_end(parameters);
	return written;
}

int printf(const char* restrict format, ...) 
{
	va_list parameters;
	va_start(parameters, format);
	int written = vfprintf(stdout, format, parameters);
	va_end(parameters);
	return written;
}

int vfprintf(FILE *stream, const char *format, va_list parameters)
{
	int written = 0;

	while (*format != '\0') 
	{
		size_t maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') 
		{
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) 
			{
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(stream, format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;

		if (*format == 'c') 
		{
			format++;
			char c = (char) va_arg(parameters, int /* char promotes to int */);
			if (!maxrem) 
			{
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(stream, &c, sizeof(c)))
				return -1;
			written++;
		} 
		else if (*format == 's') 
		{
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(stream, str, len))
				return -1;
			written += len;
		} 
		else if (*format == 'd') 
		{ 
			format++;
			int32_t x = va_arg(parameters, int32_t);
			int n = 0;
			int32_t y = 0;
			char c;

			if (x < 0)
			{
				c = '-';
				if (!print(stream, &c,1))
					return -1;
				written++;
				x = -x;
			}

			while (x)
			{
				y = 10*y+x%10;
				x /= 10;
				n++;
			}
			c = '0';
			if (!y)
			{
				if (!print(stream, &c,1))
					return -1;
				written++;
			}
			else
			{
				for (int i = 0 ; i < n ; i++)
				{
					c = (char) ('0'+y%10);
					y /= 10;
					if (!print(stream, &c,1))
						return -1;
					written++;
				}
			}
		}	
		else 
		{
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) 
			{
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(stream, format, len))
				return -1;
			written += len;
			format += len;
		}
	}
	return written;
}

int vprintf(const char *format, va_list parameters)
{
	return vfprintf(stdout, format, parameters);
}

int fgetc(FILE *stream)
{
	char c;
	int32_t res = read(stream->fd, &c, sizeof(c));
	if (res <= 0)
	{
		stream->flags |= (res == 0) ? FILE_EOF : FILE_ERROR;
		return EOF;
	}
	return c;
}

char *fgets(char *str, int num, FILE *stream)
{
	int i;
	for (i = 0 ; i < num-1 ; i++)
	{
		int ic = fgetc(stream);
		if (ic < 0)
		{
			str[i] = 0;
			return 0;
		}
		str[i] = (char) ic;
		if (ic == '\n')
		{
			i++;
			break;
		}
	}
	str[i] = 0;
	return str;
}

int fputc(int ic, FILE *stream)
{
	char c = (char) ic;
	if (write(stream->fd, &c, sizeof(c)) < 0)
	{
		stream->flags |= FILE_ERROR;
		return EOF;
	}
	return ic;
}

int getchar()
{
	return fgetc(stdin);
}

int putchar(int ic) 
{
	return fputc(ic, stdout);
}

int puts(const char* string) 
{
	return fprintf(stdout, "%s\n", string);
}

int feof(FILE *stream)
{
	return stream->flags & FILE_EOF;
}

int ferror(FILE *stream)
{
	return stream->flags & FILE_ERROR;
}

