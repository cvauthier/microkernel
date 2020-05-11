#include <stdio.h>

int main(int argc, char **argv)
{
	char path[100] = "/usr/include/stdio.h";
	
	printf("What file to open ?\n");

	if (fgets(path, 100, stdin))
	{
		char *c = strchr(path, '\n');
		if (c)
			path[c-path] = 0;
	}

	printf("Trying to open %s...\n",path);
	
	FILE *f = fopen(path, "r");
	if (!f)
		printf("Failed\n");
	else
	{
		printf("Success :\n");
		char buffer[50];
		for (int i = 0 ; i < 10 ; i++)
		{
			fgets(buffer, 50, f);
			printf("%s", buffer);
		}
		printf("\n");
	}

	return 0;
}

