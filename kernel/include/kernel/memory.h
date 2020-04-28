#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

#include <multiboot.h>

#define MAX_PAGES (4<<20)

int memory;
int nb_pages;

void memory_setup(multiboot_info_t* mbd);

uint32_t *get_heap_begin();
void map_page(void *page_addr);


#endif
