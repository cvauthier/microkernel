#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filenode.h"
#include "ramdisk.h"

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		printf("Use : rdmaker a.tar\n");
		return 0;
	}
	if (argc != 2)
	{
		printf("Error : too many arguments\n");
		return 1;
	}

	int arglen = strlen(argv[1]);
	if (strcmp(argv[1]+arglen-4,".tar") != 0)
	{
		printf("Error : %s must end with '.tar'\n", argv[1]);
		return 1;
	}

	FILE *tarball = fopen(argv[1], "rb");
	if (!tarball)
	{
		printf("Error : unable to open tarball %s\n", argv[1]);
		return 1;
	}
	
	char *rd_path = (char*) malloc(sizeof(char)*(arglen+1));
	strcpy(rd_path, argv[1]);
	strcpy(rd_path+(arglen-4), ".bin");

	FILE *ramdisk = fopen(rd_path, "wb+");
	if (!ramdisk)
	{
		printf("Error : unable to open dest file %s\n", rd_path);
		return 1;
	}
	free(rd_path);

	FileNode *root = parse_tarball(tarball);
	uint8_t *disk = convert_node_to_ramdisk(root, tarball);
	
	fwrite(disk, 1, DISK_SIZE+4, ramdisk);
	fclose(ramdisk);
	fclose(tarball);

	free_node(root);
	free(disk);
	return 0;
}

