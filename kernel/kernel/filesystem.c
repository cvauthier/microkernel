#include <kernel/filesystem.h>
#include <kernel/utility.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void rd_update_superblock()
{
	write_bigendian_int(ramdisk+4, last_free_inode);
	write_bigendian_int(ramdisk+8, free_block_list);
	write_bigendian_int(ramdisk+12, free_block_nb);
}

uint32_t rd_alloc_block()
{
	if (!free_block_list)
		return 0;
	
	uint32_t res = 0;
	uint32_t ofs = free_block_list*block_size;
	uint32_t ofs2 = ofs+4;
	
	while (ofs2 < ofs+block_size)
	{
		res = read_bigendian_int(ramdisk+ofs2);
		if (res)
		{
			write_bigendian_int(ramdisk+ofs2, 0);
			free_block_nb--;
			break;
		}
		ofs2 += 4;
	}

	if (ofs2 >= ofs+block_size)
	{
		res = free_block_list;
		free_block_list = read_bigendian_int(ramdisk+ofs);
		if (res)
			free_block_nb--;
	}

	rd_update_superblock();
	return res;
}

uint32_t rd_find_in_dir(uint32_t dir_inode, const char *name)
{
	file_descr_t *dir = open_inode_rd(dir_inode);
	uint32_t res = 0;

	char buffer[32];
	while (read_rd(dir, (void*)buffer, 32) > 0)
	{
		uint32_t ino = read_bigendian_int(buffer+28);
		buffer[28] = 0;
		if (strcmp(name, buffer) == 0)
		{
			res = ino;
			break;
		}
	}

	close_rd(dir);
	return res;
}

file_descr_t *open_inode_rd(uint32_t inode)
{
	if (inode > nb_inodes)
		return 0;
	
	uint32_t ofs = inode*block_size;

	uint32_t nlink = read_bigendian_int(ramdisk+ofs+4);
	if (!nlink)
		return 0;

	file_descr_t *fd = (file_descr_t*) calloc(1, sizeof(file_descr_t));
	fd->inode = inode;
	fd->size = read_bigendian_int(ramdisk+ofs+8);
	fd->read = read_rd;
	fd->write = write_rd;
	fd->seek = seek_rd;
	fd->close = close_rd;
	return fd;
}

file_descr_t *open_rd(const char *path)
{
	if (path[0] != '/')
		return 0;
	
	int cur_dir_ino = root_inode;

	const char *subpath = strchr(path+1,'/');
	while (subpath)
	{
		int name_len = subpath-path-1;
		if (name_len)
		{
			char *name = (char*) calloc(name_len+1, sizeof(char));
			strncpy(name, path+1, name_len);
	
			cur_dir_ino = rd_find_in_dir(cur_dir_ino, name);
			if (!cur_dir_ino)
				return 0;
		
			free(name);
		}
		path = subpath;
		subpath = strchr(path+1,'/');
	}

	uint32_t ino = rd_find_in_dir(cur_dir_ino, path+1);
	return ino ? open_inode_rd(ino) : 0;
}

void close_rd(file_descr_t *fd)
{
	free(fd);
}

int32_t read_rd(file_descr_t *fd, void *ptr, int32_t count)
{
	if (count <= 0)
		return 0;

	uint32_t main_ofs = fd->inode*block_size + 12 + 4*(fd->pos/block_size);
	uint32_t block_ofs = read_bigendian_int(ramdisk+main_ofs)*block_size + fd->pos%block_size;
	uint32_t next_pos = (fd->pos/block_size+1)*block_size;

	uint8_t *buffer = (uint8_t*) ptr;
	int32_t count0 = count;

	while (count)
	{
		if (fd->pos == fd->size)
		{
			fd->flags |= FILE_EOF;
			return count0-count;
		}
		if (fd->pos == next_pos)
		{
			next_pos += block_size;
			main_ofs += 4;
			block_ofs = read_bigendian_int(ramdisk+main_ofs)*block_size;
		}

		*buffer = ramdisk[block_ofs];
		buffer++;
		block_ofs++;
		fd->pos++;
		count--;
	}
	
	return count0;
}

int32_t write_rd(file_descr_t *fd, void *ptr, int32_t count)
{
	if (count <= 0)
		return 0;

	uint32_t main_ofs = fd->inode*block_size + 12 + fd->pos/block_size;
	uint32_t block_ofs = read_bigendian_int(ramdisk+main_ofs)*block_size + fd->pos%block_size;
	uint32_t next_pos = (fd->pos+block_size)/block_size*block_size;

	uint8_t *buffer = (uint8_t*) ptr;
	int32_t count0 = count;

	if (fd->size-fd->pos < count)
	{
		if (next_pos >= fd->pos+count)
		{
			fd->size = fd->pos+count;
		}
		else
		{
			count -= (fd->size-fd->pos);
			uint32_t blocklist_ofs = main_ofs+4;
			uint32_t limit = (main_ofs+block_size)/block_size*block_size;
			while (count > 0)
			{
				if (blocklist_ofs >= limit)
					break;
				uint32_t new_block = rd_alloc_block();
				if (!new_block)
					break;
				write_bigendian_int(ramdisk+blocklist_ofs,new_block);
				blocklist_ofs += 4;
				fd->size += block_size;
				count -= block_size;
			}
		}
		write_bigendian_int(ramdisk+fd->inode*block_size+8, fd->size);
	}

	count = count0;

	while (count)
	{
		if (fd->pos == fd->size)
		{
			return count0-count;
		}
		if (fd->pos == next_pos)
		{
			next_pos += block_size;
			main_ofs += 4;
			block_ofs = read_bigendian_int(ramdisk+main_ofs)*block_size;
		}

		ramdisk[block_ofs] = *buffer;
		buffer++;
		block_ofs++;
		fd->pos++;
		count--;
	}
	
	return count0;
}

uint32_t seek_rd(file_descr_t *fd, int32_t ofs, int flag)
{
	if (!fd->size)
		return fd->pos;
		
	uint32_t origin = (flag == SEEKFD_CUR) ? fd->pos : (flag == SEEKFD_BEGIN) ? 0 : fd->size-1;
	uint32_t abs_ofs = (uint32_t) (ofs > 0 ? ofs : -ofs);

	if (ofs > 0)
		fd->pos = (abs_ofs > fd->size-origin) ? fd->size : origin+abs_ofs;
	else
		fd->pos = (origin >= abs_ofs) ? origin-abs_ofs : 0;

	return fd->pos;
}

