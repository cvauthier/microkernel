#include <stdio.h>
#include <stdlib.h>

#if defined(__is_libk)
#include <kernel/memory.h>

static void *kernel_heap_end = 0;
#endif

/* Block structure : 
	4-byte header (size (multiple of 4), allocated(1)/free(0) flag
	actual content
	filler (to reach multiple of 4)
*/

void *malloc(size_t size)
{
	size = 4 + (((size+3)>>2)<<2);

#if defined(__is_libk)
	if (!kernel_heap_end)
	{
		kernel_heap_end = kernel_heap_begin+1024;
		map_page(kernel_heap_begin);

		uint32_t *flags = (uint32_t*) kernel_heap_begin;
		*flags = 1024;
	}

	void *previous = 0;
	void *current = kernel_heap_begin;

	while (current != kernel_heap_end)
	{
		uint32_t *flags = (uint32_t*) current;
		size_t block_size = *flags & ~0x3;

		if ((*flags & 1) == 0 && block_size >= size)
		{
			// Free block
			void *result = current+4;
			if (block_size < size + 8)
			{
				*flags = block_size | 1;
			}
			else
			{
				*flags = size | 1;
				current += size;
				flags = (uint32_t*) current;
				*flags = (block_size - size);
			}
			return result;
		}

		previous = current;
		current += block_size;
	}

	uint32_t *flags = (uint32_t*) previous;
	size_t available = (*flags & 1) ? 0 : *flags & ~0x3;
	
	while (available < size)
	{
		available += 1024;
		map_page(kernel_heap_end);
		kernel_heap_end += 1024;
	}

	current = (*flags & 1) ? current : previous;
	flags = (uint32_t*) current;
	void *result = current+4;

	if (available < size + 8)
	{
		*flags = available | 1;
	}
	else
	{
		*flags = size | 1;
		current += size;
		flags = (uint32_t*) current;
		*flags = (available-size);
	}
	return result;
	
#else
	// TODO: Implement malloc in user space
#endif
}

void free(void *ptr)
{
#if defined(__is_libk)
	uint32_t *flags = (uint32_t*) ptr;
	*flags &= ~0x1;
#else
	// TODO: Implement free in user space
#endif
}

