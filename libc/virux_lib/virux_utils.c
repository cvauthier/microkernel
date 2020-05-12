#include <virux_lib.h>

#include <stdlib.h>
#include <string.h>

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
	return (dynarray_t*) calloc(1,sizeof(dynarray_t));
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

struct queue_elt_t
{
	void *elt;
	struct queue_elt_t *next;
	struct queue_elt_t *prev;
};
typedef struct queue_elt_t queue_elt_t;

struct queue_t
{
	queue_elt_t *first;
	queue_elt_t *last;
	int size;
};

queue_t *create_queue()
{
	return (queue_t*) calloc(1,sizeof(queue_t));
}

int queue_empty(queue_t *q)
{
	return (q->size == 0);
}

void *queue_first(queue_t *q)
{
	return q->first->elt;
}

void queue_push(queue_t *q, void *x)
{
	queue_elt_t *elt = (queue_elt_t*) calloc(1,sizeof(queue_elt_t));
	elt->elt = x;
	if (q->size)
	{
		elt->prev = q->last;
		q->last->next = elt;
		q->last = elt;
	}
	else
	{
		q->last = q->first = elt;
	}
	q->size++;
}

void *queue_pop(queue_t *q)
{
	void *x = q->first->elt;
	if (q->size == 1)
	{
		free(q->first);
		q->first = q->last = 0;
	}
	else
	{
		queue_elt_t *next = q->first->next;
		free(q->first);
		q->first = next;
		q->first->prev = 0;
	}
	q->size--;
	return x;
}

void free_queue(queue_t *q)
{
	for (int i = 0 ; i < q->size ; i++)
	{
		queue_elt_t *next = q->first->next;
		free(q->first);
		q->first = next;
	}
	free(q);
}

// On suppose que dir1 est de la forme "/D1/D2/.../Dn/"
char *concat_dirs(const char *dir1, const char *dir2)
{
	char *res;
	int i;

	if (dir2[0] == '/')
	{
		// Chemin absolu
		if (!(res = (char*) calloc(strlen(dir2)+4, sizeof(char))))
			return 0;
		res[0] = '/';
		i = 0;
	}
	else
	{
		// Chemin relatif
		int n = strlen(dir1);
		if (!(res = (char*) calloc(n+strlen(dir2)+2, sizeof(char))))
			return 0;
		strcpy(res, dir1);
		i = n-1;
		if (res[i] != '/')
			res[++i] = '/';
	}

	while (*dir2)
	{
		if (*dir2 != '/')
		{
			if (*dir2 == '.' && (dir2[1] == '/' || !dir2[1]))
			{
				dir2+=2;
				continue;
			}
			if (*dir2 == '.' && dir2[1] == '.' && (dir2[2] == '/' || !dir2[2]))
			{
				dir2+=3;
				while (i > 0 && res[--i] != '/');
				continue;
			}
			while (*dir2 != '/' && *dir2)
				res[++i] = *(dir2++);
			if (*dir2 == '/')
				res[++i] = '/';
			dir2--;
		}
		dir2++;
	}
	res[i+1] = 0;
	return res;
}

