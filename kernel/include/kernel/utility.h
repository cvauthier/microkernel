#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>

// Sert pour le ramdisk

uint32_t read_bigendian_int(uint8_t *ptr);
void write_bigendian_int(uint8_t *ptr, uint32_t n);

// Tableau dynamique

struct dynarray_t
{
	int size;
	int max_size;
	void **array;
};
typedef struct dynarray_t dynarray_t;

dynarray_t *create_dynarray();

void dynarray_push(dynarray_t *arr, void *elem);
void dynarray_pop(dynarray_t *arr);

void free_dynarray(dynarray_t *arr);

#endif
