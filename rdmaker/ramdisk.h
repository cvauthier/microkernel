#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdint.h>

#include "filenode.h"

#define BLOCK_SIZE 512
#define DISK_SIZE (4<<20)
#define NB_BLOCKS (DISK_SIZE/BLOCK_SIZE)

uint8_t *convert_node_to_ramdisk(FileNode *root, FILE *tarball);

#endif
