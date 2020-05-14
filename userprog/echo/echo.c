#include <stdio.h>
#include <string.h>

#include <virux_lib.h>

void show_use()
{
	printf("Use : echo [-ne] str\n");
}

int main(int argc, char **argv)
{
	int n = 0, e = 0;
	char *str;

	if (argc < 2)
	{
		show_use();
		return 1;
	}
	for (int i = 1 ; i < argc-1 ; i++)
	{
		char *s = argv[i];
		if (*s != '-')
		{
			printf("Syntax error\n");
			show_use();
			return 1;
		}
		s++;
		while (*s)
		{
			if (*s == 'n') n = 1;
			else if (*s == 'e') e = 1;
			else
			{
				printf("Unknown option %c\n", *s);
				return 1;
			}
			s++;
		}
	}
	str = argv[argc-1];

	while (*str)
	{
		if (*str == '\\')
		{
			str++;
			if (*str == 'n') putchar('\n');
			else if (*str == 't') putchar('\t');
			else if (*str == '\\') putchar('\\');
			else if (*str) putchar(*str);
			else str--;
			str++;
		}
		else
			putchar(*(str++));
	}
	
	if (!n) putchar('\n');

	return 0;
}

