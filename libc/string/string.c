#include <string.h>

char *strcpy (char *dest, const char *src)
{
	while (*src)
		*(dest++) = *(src++);
	return dest;
}

char *strncpy(char *dest, const char *src, size_t num)
{
	size_t i = 0;
	while (i < num && src[i])
	{
		dest[i] = src[i];
		i++;
	}
	while (i < num)
		dest[i++] = 0;
	return dest;
}

const char *strchr(const char *str, int c)
{
	while (*str)
	{
		if (*str == c)
			return str;
		str++;
	}
	return 0;
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

int strcmp(const char *lhs, const char *rhs)
{
	size_t i = 0;
	while (rhs[i])
	{
		if (rhs[i] != lhs[i])
			return (lhs[i]<rhs[i]) ? -1 : 1;
		i++;
	}
	return lhs[i] ? 1 : 0;
}

