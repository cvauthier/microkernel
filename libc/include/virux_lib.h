#ifndef VIRUX_LIB_H
#define VIRUX_LIB_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Appels système

int wait(int *pid, int *code);
int fork();
void exit(int code);

int open(const char *path);
void close(int fd);
int32_t write(int fd, void *ptr, int32_t count);
int32_t read(int fd, void *buffer, int32_t count);
uint32_t seek(int fd, int32_t pos, int flag); 

int dup(int oldfd);
int dup2(int oldfd, int newfd);

void *sbrk(int incr);

void exec(const char *path, char **argv);

char *getcwd(char *buf, size_t size);
int chdir(const char *path);

// Utile pour le ramdisk

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

// File

struct queue_t;
typedef struct queue_t queue_t;

queue_t *create_queue();

int queue_empty(queue_t *q);
void *queue_first(queue_t *q);
void queue_push(queue_t *q, void *elt);
void *queue_pop(queue_t *q);

void free_queue(queue_t *q);

// Manipulation de chemins

char *concat_dirs(const char *dir1, const char *dir2);

#ifdef __cplusplus
}
#endif

#endif
