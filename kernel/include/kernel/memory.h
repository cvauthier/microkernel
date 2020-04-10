#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

void memory_setup();

void map_page(void *page_addr);

#endif
