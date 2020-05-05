#include <kernel/utility.h>

#include <stdlib.h>

uint32_t read_bigendian_int(uint8_t *ptr)
{
	return (ptr[0]<<24) + (ptr[1]<<16) + (ptr[2]<<8) + ptr[3];
}

void write_bigendian_int(uint8_t *ptr, uint32_t n)
{
	for (int i = 3 ; i >= 0 ; i--)
	{
		ptr[i] = n&0xFF;
		n >>= 8;
	}
}

dynarray_t *create_dynarray()
{
	dynarray_t *arr = (dynarray_t*) calloc(1,sizeof(dynarray_t));
	return arr;
}

void dynarray_push(dynarray_t *arr, void *elem)
{
	if (!arr->max_size)
	{
		arr->max_size = 2;
		arr->array = (void**) calloc(2, sizeof(void*));
	}
	else if (arr->size == arr->max_size)
	{
		void **new_array = (void**) calloc(2*arr->size, sizeof(void*));
		for (int i = 0 ; i < arr->size ; i++)
			new_array[i] = arr->array[i];
		free(arr->array);
		arr->max_size *= 2;
		arr->array = new_array;
	}
	arr->array[arr->size++] = elem;
}

void dynarray_pop(dynarray_t *arr)
{
	if (arr->size)
		arr->size--;
}

void free_dynarray(dynarray_t *arr)
{
	if (arr->array)
		free(arr->array);
	free(arr);
}

