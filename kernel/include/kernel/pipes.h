#ifndef PIPES_H
#define PIPES_H

#include <kernel/filesystem.h>

#include <stdint.h>
#include <stddef.h>

#define NB_MAX_PIPES 100
#define PIPE_DEFAULT_SIZE 4096

void init_pipe_system();

void create_pipe(file_descr_t *fd[2]);
void close_pipe(file_descr_t *fd);

int32_t read_pipe(file_descr_t *fd, void *ptr, int32_t count);
int32_t write_pipe(file_descr_t *fd, void *ptr, int32_t count);

#endif
