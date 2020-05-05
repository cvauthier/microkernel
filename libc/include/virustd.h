#ifndef VIRUSTD_H
#define VIRUSTD_H

#include <stdint.h>

int wait(int *pid, int *code);
int fork();
void exit(int code);

int open(const char *path);
void close(int fd);
size_t write(int fd, void *ptr, size_t count);
size_t read(int fd, void *buffer, size_t count);
uint32_t seek(int fd, int32_t pos, int flag); 

/*
void *sbrk(int incr);
void exec(char *path);

*/

#endif
