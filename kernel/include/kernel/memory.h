#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

void *kernel_heap_begin;

void init_memory();

void map_page(void *page_addr);

#endif
