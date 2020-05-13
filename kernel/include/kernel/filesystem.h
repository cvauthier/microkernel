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
#define FILE_READ 0x2
#define FILE_WRITE 0x4
#define FILE_CLOEXEC 0x8

typedef enum { FileType_File, FileType_Directory, FileType_Terminal, FileType_Pipe } file_type_t;

struct resource_t;

struct file_descr_t
{
	uint32_t inode;
	uint32_t pos;
	uint32_t size;
	int owners;
	file_type_t type;
	uint8_t flags;

	struct resource_t *res;

	int32_t (*read)(struct file_descr_t*,void*,int32_t);
	int32_t (*write)(struct file_descr_t*,void*,int32_t);
	uint32_t (*seek)(struct file_descr_t*,int32_t,int);
	void (*close)(struct file_descr_t*);
};
typedef struct file_descr_t file_descr_t;

void rd_update_superblock();

file_descr_t *open_inode_rd(uint32_t inode);
file_descr_t *open_rd(const char *path);
void close_rd(file_descr_t *fd);

int32_t read_rd(file_descr_t *fd, void *ptr, int32_t count);
int32_t write_rd(file_descr_t *fd, void *ptr, int32_t count);
uint32_t seek_rd(file_descr_t *fd, int32_t ofs, int flag);

#endif
