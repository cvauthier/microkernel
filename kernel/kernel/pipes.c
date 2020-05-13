#include <kernel/pipes.h>

#include <kernel/process.h>

#include <stdlib.h>
#include <virux_lib.h>

typedef struct
{
	uint8_t *buffer;
	int size;
	int start_read;
	int start_write;
	file_descr_t *fd_in;
	file_descr_t *fd_out;
} pipe_t;

static dynarray_t *pipes;

void init_pipe_system()
{
	pipes = create_dynarray();
}

static uint32_t do_nothing()
{
	return 0;
}

void create_pipe(file_descr_t *fd[2])
{
	fd[0] = fd[1] = 0;

	int index = 0;
	while (index < pipes->size && pipes->array[index]) index++;
	if (index == pipes->size)
	{
		if (index == NB_MAX_PIPES)
			return;
		dynarray_push(pipes, 0);
	}

	file_descr_t *fd_in = (file_descr_t*) calloc(1, sizeof(file_descr_t));
	file_descr_t *fd_out = (file_descr_t*) calloc(1, sizeof(file_descr_t));
	pipe_t *p = (pipe_t*) calloc(1, sizeof(pipe_t));

	p->buffer = (uint8_t*) calloc(PIPE_DEFAULT_SIZE, sizeof(uint8_t));
	p->size = PIPE_DEFAULT_SIZE;
	p->fd_in = fd_in;
	p->fd_out = fd_out;

	fd_in->inode = fd_out->inode = (uint32_t) index;
	fd_in->type = fd_out->type = FileType_Pipe; 
	fd_in->flags = FILE_WRITE;
	fd_out->flags = FILE_READ;

	fd_in->res = create_resource();
	fd_out->res = create_resource();

	fd_in->close = fd_out->close = close_pipe;
	fd_in->read = fd_out->read = read_pipe;
	fd_in->write = fd_out->write = write_pipe;
	fd_in->seek = fd_out->seek = (uint32_t(*)(file_descr_t*, int32_t, int)) do_nothing;

	pipes->array[index] = (void*) p;
	fd[0] = fd_in;
	fd[1] = fd_out;
}

void close_pipe(file_descr_t *fd)
{
	int i = (int) fd->inode;
	pipe_t *p = (pipe_t*) pipes->array[i];

	free_resource(fd->res);
	if (p->fd_in == fd) 
	{
		p->fd_in = 0;
		if (p->fd_out && resource_waiting_event(p->fd_out->res))
			resource_event(p->fd_out->res);
	}
	if (p->fd_out == fd) 
	{
		p->fd_out = 0;
		if (p->fd_in && resource_waiting_event(p->fd_in->res))
			resource_event(p->fd_in->res);
	}
	
	free(fd);

	if (!p->fd_in && !p->fd_out)
	{
		free(p->buffer);
		free(p);
		pipes->array[i] = 0;
	}
}

int32_t read_pipe(file_descr_t *fd, void *ptr, int32_t count)
{
	if (!(fd->flags & FILE_READ) || count <= 0)
		return 0;
	
	pipe_t *p = (pipe_t*) pipes->array[fd->inode];

	resource_request(fd->res);
	
	while (p->fd_in && p->start_write == p->start_read)
		resource_wait_event(fd->res);
	
	if (!p->fd_in && p->start_write == p->start_read)
		return -1;
	
	uint8_t *buffer = (uint8_t*) ptr;
	int32_t count0 = count;

	while (count)
	{
		if (p->start_write == p->start_read)
			break;
		*(buffer++) = p->buffer[p->start_read];
		count--;
		p->start_read = (p->start_read+1)%p->size;
	}

	resource_release(fd->res);

	if (count0 > count && p->fd_in && resource_waiting_event(p->fd_in->res))
		resource_event(p->fd_in->res);

	return count0-count;
}

int32_t write_pipe(file_descr_t *fd, void *ptr, int32_t count)
{
	if (!(fd->flags & FILE_WRITE) || count <= 0)
		return 0;
	
	pipe_t *p = (pipe_t*) pipes->array[fd->inode];
	
	resource_request(fd->res);
	while (p->fd_out && (p->start_write+1)%p->size == p->start_read)
		resource_wait_event(fd->res);
	if (!p->fd_out)
		return count;
	
	uint8_t *buffer = (uint8_t*) ptr;

	int32_t count0 = count;
	while (count)
	{
		if ((p->start_write+1)%p->size == p->start_read)
			break;
		p->buffer[p->start_write] = *(buffer++);
		count--;
		p->start_write = (p->start_write+1)%p->size;
	}

	resource_release(fd->res);

	if (count0 > count && p->fd_out && resource_waiting_event(p->fd_out->res))
		resource_event(p->fd_out->res);

	return count0-count;
}


