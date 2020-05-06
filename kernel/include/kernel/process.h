#ifndef PROC_H
#define PROC_H

#include <kernel/filesystem.h>
#include <kernel/utility.h>

/* Dépendant de l'architecture, implémenté dans le répertoire arch/ */

struct proc_data_t; // Données dépendant de l'architecture
typedef struct proc_data_t proc_data_t;

proc_data_t *fork_proc_data(proc_data_t *src);
void free_proc_data(proc_data_t *proc);

void kernel_proc(int (*start)(void*), void *arg);

void scheduler_tick();
void reschedule();

void syscall_exec(const char *path); // Exécute un fichier elf

/* Indépendant */

#define NB_MAX_PROC 64
#define MAX_PRIORITY 10 // 0 <= p <= 10

#define DEFAULT_TIME 5

#define IDLE_PID 0
#define ZOMBIE_SLAYER_PID 1

enum { Runnable, Waiting, Zombie };
enum { Syscall_Wait, Syscall_Fork, Syscall_Exit, Syscall_Open, Syscall_Close, Syscall_Read, Syscall_Write, Syscall_Seek };

struct process_t
{
	int parent_pid;
	int priority;
	int state;
	int exit_code;
	int ms_left;
	dynarray_t *files;
	proc_data_t *arch_data;
};
typedef struct process_t process_t;

process_t *proc_list[NB_MAX_PROC];
int cur_pid;
int scheduling_on;

void free_proc(process_t *proc);

int new_pid();
void schedule(); // Met à jour cur_pid

int syscall_wait(int *pid, int *code); // Renvoie le PID à récupérer (négatif si le processus est bloqué)
int syscall_fork(); // Renvoie le PID du processus fils (négatif en cas d'échec)
void syscall_exit(int code);
int syscall_open(const char *path);
int32_t syscall_write(int fd, void *ptr, int32_t count);
int32_t syscall_read(int fd, void *ptr, int32_t count);
uint32_t syscall_seek(int fd, int32_t ofs, int flag);
void syscall_close(int fd);

#endif
