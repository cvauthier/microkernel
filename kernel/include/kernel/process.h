#ifndef PROC_H
#define PROC_H

/* Dépendant de l'architecture, implémenté dans le répertoire arch/ */

struct proc_data_t; // Données dépendant de l'architecture
typedef struct proc_data_t proc_data_t;

proc_data_t *fork_proc_data(proc_data_t *src, int child_pid);
void wait_end_proc_data(proc_data_t *data, int child_pid);
void free_proc_data(proc_data_t *proc);

/* Indépendant */

#define NB_MAX_PROC 256
#define MAX_PRIORITY 15 // 0 <= p <= 15

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

void schedule(); // Met à jour cur_pid

int syscall_wait(); // Renvoie le PID à récupérer (négatif si le processus est bloqué)
int syscall_fork(); // Renvoie le PID du processus fils (négatif en cas d'échec)
void syscall_exit(int code);


#endif
