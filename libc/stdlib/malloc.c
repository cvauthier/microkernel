#include <stdio.h>
#include <stdlib.h>

#if defined(__is_libk)
#include <kernel/memory.h>

static uint32_t *kernel_heap_begin = 0x100000;
static uint32_t *kernel_heap_end = 0;
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

		*kernel_heap_begin = 4096;
	}

	uint32_t *prev = 0;
	uint32_t *cur = kernel_heap_begin;

	while (cur != kernel_heap_end)
	{
		size_t block_size = *cur & ~0x3;

		if ((*cur & 1) == 0 && block_size >= size)
		{
			// Free block
			void *result = (void*) (cur+1);
			if (block_size < size + 8)
			{
				*cur = block_size | 1;
			}
			else
			{
				*cur = size | 1;
				cur += size>>2;
				*cur = block_size - size;
			}
			return result;
		}

		prev = cur;
		cur += block_size>>2;
	}

	size_t available = (*prev & 1) ? 0 : *prev & ~0x3;
	
	while (available < size)
	{
		available += 4096;
		map_page(kernel_heap_end);
		kernel_heap_end += 1024;
	}

	cur = (*prev & 1) ? cur : prev;
	void *result = (void*) (cur+1);

	if (available < size + 8)
	{
		*cur = available | 1;
	}
	else
	{
		*cur = size | 1;
		cur += size>>2;
		*cur = (available-size);
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

