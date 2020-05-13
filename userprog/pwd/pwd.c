#include <stdio.h>
#include <stdlib.h>

#include <virux_lib.h>

int main(int argc, char **argv)
{
	int size = 50;
	char *cwd = (char*) malloc(size*sizeof(char));
	
	while (cwd && !getcwd(cwd, size))
	{
		size *= 2;
		free(cwd);
		cwd = (char*) malloc(size*sizeof(char));
	}

	if (!cwd)
	{
		printf("Could not retrieve current working directory\n");
		return 1;
	}

	printf("%s\n", cwd);
	return 0;
}

