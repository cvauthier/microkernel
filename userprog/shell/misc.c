#include "misc.h"

#include <stdlib.h>
#include <string.h>

char *working_dir()
{
	int size = 100;
	char *cwd = (char*) malloc(size*sizeof(char));
	
	while (cwd && !getcwd(cwd, size))
	{
		size *= 2;
		free(cwd);
		cwd = (char*) malloc(size*sizeof(char));
	}

	return cwd;
}

int isalpha(char c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

int isnum(char c)
{
	return ('0' <= c && c <= '9');
}

char *copy_str(const char *s)
{
	char *res = (char*) malloc((strlen(s)+1)*sizeof(char));
	strcpy(res, s);
	return res;
}

char *str_of_int(int n)
{
	int nb_chars = 0, m = n;
	if (!n) nb_chars = 1;
	else
	{
		while (m)
		{
			nb_chars++;
			m /= 10;
		}
		nb_chars += (n < 0);
	}

	char *s = (char*) calloc(nb_chars+1,sizeof(char));
	int i = nb_chars-1;
	m = n;
	while (m)
	{
		s[i--] = '0'+(m%10);
		m /= 10;
	}
	if (n < 0) s[0] = '-';
	else if (!n) s[0] = '0';
	return s;
}

char *dynarray_to_str(dynarray_t *arr)
{
	int i;
	char *s = (char*) malloc((arr->size+1)*sizeof(char));
	for (i = 0 ; i < arr->size ; i++)
		s[i] = (char) arr->array[i];
	s[i] = 0;
	return s;
}

