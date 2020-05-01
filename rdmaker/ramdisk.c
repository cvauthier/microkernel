#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ramdisk.h"

int actual_directory_size(FileNode *node)
{
	return (2+node->nb_childs)*32;
}

int prepare_nodes(FileNode *node, int next_inode)
{
	node->inode = next_inode;
	node->size = (node->nb_childs < 0) ? node->size : actual_directory_size(node);
	next_inode += 1 + ((node->size+BLOCK_SIZE-1)/BLOCK_SIZE);
	for (int i = 0 ; i < node->nb_childs ; i++)
		next_inode = prepare_nodes(node->childs[i], next_inode);
	return next_inode;
}

void write_int(uint8_t *ptr, uint32_t n)
{
	for (int i = 0 ; i < 4 ; i++)
	{
		ptr[3-i] = n&0xFF;
		n >>= 8;
	}
}

void write_node(FileNode *node, FileNode *parent, uint8_t *disk, FILE *tarball)
{
	int ofs = node->inode*BLOCK_SIZE;

	write_int(disk+ofs, (node->nb_childs < 0) ? 0 : 1); // kind
	write_int(disk+ofs+4, 1); // nlink
	write_int(disk+ofs+8, node->size); // size

	int j = 0;
	for (int i = 0 ; i < node->size ; i += BLOCK_SIZE)
	{
		write_int(disk+ofs+12+4*j, node->inode+1+j);
		j++;
	}

	if (node->nb_childs < 0)
	{
		fseek(tarball, node->pos, SEEK_SET);
		fread(disk+ofs+BLOCK_SIZE, 1, node->size, tarball);
	}
	else
	{
		ofs += BLOCK_SIZE;
		disk[ofs] = disk[ofs+32] = disk[ofs+33] = '.';
		write_int(disk+ofs+28, node->inode);
		write_int(disk+ofs+32+28, parent->inode);

		ofs += 64;

		for (int i = 0 ; i < node->nb_childs ; i++)
		{
			for (int j = 0 ; j < 28 && node->childs[i]->name[j] ; j++)
				disk[ofs+j] = node->childs[i]->name[j];
			write_int(disk+ofs+28, node->childs[i]->inode);
			write_node(node->childs[i], node, disk, tarball);
			ofs += 32;
		}
	}
}

uint8_t *convert_node_to_ramdisk(FileNode *root, FILE *tarball)
{
	uint8_t *disk = (uint8_t*) calloc(DISK_SIZE+4,sizeof(uint8_t));

	int next_inode = prepare_nodes(root, 1);
	// superblock
	write_int(disk, NB_BLOCKS-1); // inode_nb
	write_int(disk+4, 0); // last_free_inode
	write_int(disk+8, next_inode); // free_block_list
	write_int(disk+12, NB_BLOCKS-next_inode); // free_block_nb
	write_int(disk+16, root->inode);
	memcpy(disk+BLOCK_SIZE-4, "sfs0", 4);

	write_node(root, root, disk, tarball);

	while (next_inode < NB_BLOCKS)
	{
		int ofs = next_inode*BLOCK_SIZE;
		int ofs2 = ofs;
		int rem = BLOCK_SIZE/4-1;
		while (rem > 0 && ++next_inode < NB_BLOCKS)
		{
			ofs2 += 4;
			rem--;
			write_int(disk+ofs2, next_inode);
		}
		if (next_inode < NB_BLOCKS)
			write_int(disk+ofs, next_inode);
	}

	write_int(disk+DISK_SIZE, BLOCK_SIZE);

	return disk;
}


