#ifndef VIRUSTD_H
#define VIRUSTD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int wait(int *pid, int *code);
int fork();
void exit(int code);

int open(const char *path);
void close(int fd);
int32_t write(int fd, void *ptr, int32_t count);
int32_t read(int fd, void *buffer, int32_t count);
uint32_t seek(int fd, int32_t pos, int flag); 

void *sbrk(int incr);

void exec(const char *path);

char *getcwd(char *buf, size_t size);
int chdir(const char *path);

#ifdef __cplusplus
}
#endif

#endif
