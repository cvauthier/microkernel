#include <stdio.h>
#include <string.h>

#include <virux_lib.h>

void show_use()
{
	printf("Use : less [-h lines] filename\n");
}

int main(int argc, char **argv)
{
	if (argc != 2 && argc != 4)
	{
		printf("Wrong number of arguments\n", argv[1]);
		show_use();
		return 1;
	}

	int lines = 30;
	char *filename = 0;
	if (argc == 2)
	{
		filename = argv[1];
	}
	else
	{
		if (strcmp(argv[1], "-h"))
		{
			printf("Unknown option \"%s\"\n", argv[1]);
			show_use();
			return 1;
		}
		lines = 0;
		char *nb = argv[2];
		while (*nb)
		{
			if (*nb < '0' || *nb > '9')
			{
				printf("Expected a number : %d\n", argv[2]);
				show_use();
				return 1;
			}
			lines = lines*10+(*nb-'0');
			nb++;
		}
		filename = argv[3];
	}

	FILE *f = fopen(filename, "r");
	if (!f)
	{
		printf("Could not open file %s\n", filename);
		return 1;
	}

	while (lines && !feof(f))
	{
		char buffer[50];
		if (fgets(buffer, 50, f))
		{
			printf("%s", buffer);
			if (strchr(buffer, '\n'))
				lines--;
		}
		else
		{
			if (ferror(f))
			{
				printf("\nError reading the file\n");
				return 1;
			}
		}
	}

	return 0;
}

