#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>

uint8_t *ramdisk;
uint32_t nb_inodes;
uint32_t root_inode;
uint32_t block_size;
uint32_t disk_size;
uint32_t last_free_inode;
uint32_t free_block_nb;
uint32_t free_block_list;

#define FILE_EOF 0x1

#define SEEKFD_CUR 0
#define SEEKFD_BEGIN 1
#define SEEKFD_END 2

struct file_descr_t
{
	uint32_t inode;
	uint32_t pos;
	uint32_t size;
	uint8_t flags;
};
typedef struct file_descr_t file_descr_t;

void rd_update_superblock();

file_descr_t *open_inode(uint32_t inode);
file_descr_t *open_fd(const char *path);
void close_fd(file_descr_t *fd);

size_t read_fd(file_descr_t *fd, void *ptr, size_t count);
size_t write_fd(file_descr_t *fd, void *ptr, size_t count);
uint32_t seek_fd(file_descr_t *fd, int32_t ofs, int flag);

#endif
