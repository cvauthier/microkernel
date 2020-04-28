#ifndef PROC_H
#define PROC_H

/* Dépendant de l'architecture, implémenté dans le répertoire arch/ */

struct proc_data_t; // Données dépendant de l'architecture
typedef struct proc_data_t proc_data_t;

proc_data_t *fork_proc_data(proc_data_t *src);
void free_proc_data(proc_data_t *proc);

void kernel_proc(int (*start)(void*), void *arg);

void scheduler_tick();
void reschedule();

/* Indépendant */

#define NB_MAX_PROC 256
#define MAX_PRIORITY 15 // 0 <= p <= 15

#define DEFAULT_TIME 5

#define IDLE_PID 0
#define ZOMBIE_SLAYER_PID 1

enum { Free, Waiting, Runnable, Zombie };
enum { Wait, Fork, Exit };

struct process_t
{
	int parent_pid;
	int priority;
	int state;
	int exit_code;
	int ms_left;
	proc_data_t *arch_data;
};
typedef struct process_t process_t;

struct intlist_t
{
	int n;
	struct intlist_t *next;
};

process_t *proc_list[NB_MAX_PROC];
int cur_pid;
int scheduling_on;

int new_pid();

void schedule(); // Met à jour cur_pid

int syscall_wait(int *pid, int *code); // Renvoie le PID à récupérer (négatif si le processus est bloqué)
int syscall_fork(); // Renvoie le PID du processus fils (négatif en cas d'échec)
void syscall_exit(int code);

#endif
