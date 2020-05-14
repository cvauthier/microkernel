#ifndef SYSCALLS_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/syscall_codes.h>

// Appels systèmes liés à l'ordonnanceur

int syscall_wait(int *pid, int *code); // Renvoie le PID à récupérer (négatif si le processus est bloqué)
int syscall_fork(); // Renvoie le PID du processus fils (négatif en cas d'échec)
void syscall_exit(int code);

// Appels systèmes liés aux fichiers

int syscall_open(const char *path, int flags);
int32_t syscall_write(int fd, void *ptr, int32_t count);
int32_t syscall_read(int fd, void *ptr, int32_t count);
uint32_t syscall_seek(int fd, int32_t ofs, int flag);
void syscall_close(int fd);

int syscall_dup(int oldfd);
int syscall_dup2(int oldfd, int newfd);

// Appel exec

void syscall_exec(const char *path, char **argv); // Exécute un fichier elf

// Autres appels

void *syscall_sbrk(int incr);
char *syscall_getcwd(char *buf, size_t size);
int syscall_chdir(const char *path);
int syscall_pipe(int *pipefds, int flags);
unsigned int syscall_gettime();

int syscall_invalid();

#endif
