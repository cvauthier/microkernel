#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__is_libk)
#include <kernel/memory.h>
#else
#include <virustd.h>
#endif

static uint8_t *heap_begin = 0;
static uint8_t *heap_end   = 0;
static int heap_init_failed = 0;

/* 
	Fonctions de manipulation des blocs de mémoire constituant le tas 
	Structure d'un bloc : 
	- en-tête de 4 octets : bits 31-2 : taille du bloc (multiple de 4), bit 1 : bloc précédent libre ? , bit 0 : bloc libre ?
	- contenu du block, avec éventuellement un remplissage pour avoir un multiple de 4 octets
	- si le bloc n'est pas libre, duplication de l'en-tête si le bloc est libre
*/

#define BLOCK_FREE 0x1
#define BLOCK_PREV_FREE 0x2
#define BLOCK_FIRST 0x4 // Premier bloc du tas
#define BLOCK_LAST 0x8 // Dernier bloc du tas

#define MIN_BLOCK_SIZE 8

typedef struct
{
	size_t size;
	uint8_t *addr;
	int flags;
} mem_block;

size_t get_block_size(size_t s) // Taille d'un bloc nécessaire pour allouer une mémoire de s
{
	return 4 + ((s+3)/4*4);
}

void read_block_at(mem_block *blk, uint8_t *ptr)
{
	uint32_t *header = (uint32_t*) ptr;

	blk->size = *header & 0xFFFFFFFC;
	blk->addr = ptr;
	blk->flags = *header & 0x3;
	
	if (ptr+blk->size == heap_end) blk->flags |= BLOCK_LAST;
	if (ptr == heap_begin)				 blk->flags |= BLOCK_FIRST;
}

void next_block(mem_block *dest, mem_block *blk)
{
	read_block_at(dest, blk->addr + blk->size);
}

/*
Le but de cette fonction est de permettre de parcourir le tas avec une boucle du type

starting_block(&blk);
while (!(blk.flags & BLOCK_LAST))
{
	next_block(&blk, &blk);
	...
}
*/
void starting_block(mem_block *blk)
{
	blk->size = 0;
	blk->addr = heap_begin;
	blk->flags = 0;
}

void write_block(mem_block *blk)
{
	uint32_t *header = (uint32_t*) blk->addr;
	*header = (blk->size & 0xFFFFFFFC) | (blk->flags & 0x3);
	if (blk->flags & BLOCK_FREE)
		*((uint32_t*)(blk->addr+blk->size-4)) = *header;
}

void *block_content_addr(mem_block *blk)
{
	return (void*) (blk->addr+4);
}

void allocate_block(mem_block *blk, size_t size)
{
	if (!(blk->flags & BLOCK_FREE) || blk->size < size)
		return;
	
	if (blk->size < size+MIN_BLOCK_SIZE)
	{
		if (!(blk->flags & BLOCK_LAST))
		{
			mem_block blk2;
			next_block(&blk2, blk);
			blk2.flags ^= BLOCK_PREV_FREE;
			write_block(&blk2);
		}
	}
	else
	{
		mem_block blk2;
		blk2.addr = blk->addr+size;
		blk2.size = blk->size-size;
		blk2.flags = BLOCK_FREE;
		if (blk->flags & BLOCK_LAST)
		{
			blk2.flags |= BLOCK_LAST;
			blk->flags ^= BLOCK_LAST;
		}
		blk->size = size;
		write_block(&blk2);
	}
	blk->flags ^= BLOCK_FREE;
	write_block(blk);
}

/* Fonctions de manipulation du tas */

int expand_heap(int incr)
{
#ifdef __is_libk
	while (incr > 0)
	{
		if (!map_page((void*) heap_end))
			return 1;
		heap_end += PAGE_SIZE;
		incr -= PAGE_SIZE;
	}
#else
	uint8_t *prev_end = heap_end;
	heap_end = (uint8_t*) sbrk(incr);
	if (heap_end - prev_end < incr)	
		return 1;
#endif
	return 0;
}

void init_heap()
{
#ifdef __is_libk
	heap_begin = heap_end = (uint8_t*) get_heap_begin(); 
#else
	heap_begin = heap_end = (uint8_t*) sbrk(0);
#endif
	if ((heap_init_failed = expand_heap(4096)))
		return;
	// Premier bloc libre
	*((uint32_t*) heap_begin) = *((uint32_t*) (heap_end-4)) = (heap_end-heap_begin) | BLOCK_FREE;
}

void *malloc(size_t size)
{
	if (!size)
		return 0;
	if (!heap_begin || heap_init_failed)
	{
		init_heap();
		if (heap_init_failed)
			return 0;
	}

	size = get_block_size(size);
	
	mem_block cur;
	starting_block(&cur);

	while (!(cur.flags & BLOCK_LAST))
	{
		next_block(&cur, &cur);
		if ((cur.flags & BLOCK_FREE) && cur.size >= size)
		{
			allocate_block(&cur, size);
			return block_content_addr(&cur);
		}
	}

	int fail = 0;
	if (cur.flags & BLOCK_FREE)
	{
		fail = expand_heap(size-cur.size);
		cur.size = heap_end-cur.addr;
		write_block(&cur);
	}
	else
	{
		fail = expand_heap(size);
		size_t added_size = heap_end-(cur.addr+cur.size);
		
		if (added_size < MIN_BLOCK_SIZE)
		{
			/* Echec de l'expansion du tas, et en plus on n'a même pas de quoi créer un bloc libre. 
				On augmente alors la taille du dernier bloc (alloué) */
			cur.size = heap_end-cur.addr;
			write_block(&cur);
			return 0;
		}

		cur.flags ^= BLOCK_LAST;
		write_block(&cur);
		
		cur.addr += cur.size;
		cur.size = added_size;
		cur.flags = BLOCK_FREE | BLOCK_LAST;
		write_block(&cur);
	}

	if (fail)
		return 0;
	
	allocate_block(&cur, size);
	return block_content_addr(&cur);
}

void fuse_with_next(mem_block *blk, mem_block *next)
{
	mem_block res;
	res.addr = blk->addr;
	res.size = blk->size+next->size;
	res.flags = (blk->flags & (BLOCK_FIRST | BLOCK_PREV_FREE)) | (next->flags & BLOCK_LAST) | BLOCK_FREE;
	write_block(&res);
}

void free(void *ptr)
{
	mem_block blk, blk2;
	read_block_at(&blk, ((uint8_t*)ptr)-4);

	blk.flags |= BLOCK_FREE;
	write_block(&blk);

	if (!(blk.flags & BLOCK_LAST))
	{
		next_block(&blk2, &blk);
		if (blk2.flags & BLOCK_FREE)
			fuse_with_next(&blk, &blk2);
	}
	if (blk.flags & BLOCK_PREV_FREE)
	{
		read_block_at(&blk2, blk.addr-4);
		blk2.addr = blk2.addr+4-blk2.size;
		fuse_with_next(&blk2, &blk);
	}
}

void *calloc(size_t num, size_t size)
{
	size *= num;
	char *res = (char*) malloc(size);
	for (size_t i = 0 ; i < size ; i++)
		res[i] = 0;
	return (void*) res;
}

