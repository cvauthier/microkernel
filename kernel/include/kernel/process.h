#ifndef PROC_H
#define PROC_H

#include <kernel/memory.h>
#include <kernel/filesystem.h>
#include <virux_lib.h>

/* Dépendant de l'architecture, implémenté dans le répertoire arch/ */

struct hw_context_t;
typedef struct hw_context_t hw_context_t;

struct process_t;
typedef struct process_t process_t;

void context_switch(process_t *prev, process_t *next);
void first_context_switch(process_t *proc);

void setup_fork_switch(process_t *parent, process_t *child);
void setup_start_point(process_t *proc, int (*start)(void*), void *arg);

void jump_to_ring3(void (*start)(), stackint_t *stack);

hw_context_t *create_hw_context();
void free_hw_context(hw_context_t *ctx);

void disable_interrupts();
void enable_interrupts();

/* Indépendant */

#define KERNEL_STACK_SIZE 4096
#define MAX_STACK_SIZE (20<<20)

#define NB_MAX_PROC 64
#define MAX_PRIORITY 10 // 0 <= p <= 10

#define DEFAULT_TIME 5

#define IDLE_PID 0
#define ZOMBIE_SLAYER_PID 1

enum { Runnable, Waiting, Zombie, WaitingResource };

struct process_t
{
	int parent_pid;
	int priority;
	int state;
	int exit_code;
	int ms_left;
	
	dynarray_t *files;
	char *cwd;

	hw_context_t *hw_context;
	paddr_t pd_addr;
	stackint_t *kernel_stack;

	vaddr_t code_begin;
	vaddr_t code_end;
	vaddr_t data_begin;
	vaddr_t data_end;
	vaddr_t heap_begin;
	vaddr_t heap_end;
	vaddr_t stack_top;
	vaddr_t stack_bottom;
};
typedef struct process_t process_t;

/* L'organisation de l'userspace est plus ou moins celle du "flexible layout" de Linux, sans les régions de mémoire anonymes
 (code à partir de 0x08048000, data et bss juste après, puis le tas, et la pile à la fin) */

struct resource_t
{
	queue_t *waiting_queue;
	int waiting_event;
};
typedef struct resource_t resource_t;

int kernel_proc(int (*start)(void*), void *arg);

process_t *new_proc();
void free_proc_userspace(process_t *proc);
void free_proc(process_t *proc);

// Ordonnanceur

process_t *proc_list[NB_MAX_PROC];
int cur_pid;
int scheduling_on;

int new_pid();

void scheduler_init();
void make_runnable(int pid);
void schedule(); // Met à jour cur_pid
void scheduler_tick();
void reschedule(); // Appelle schedule et change de processus

/* Accès concurrent à une ressource
	 Un processus peut demander une resource et est mis dans une file. Le processus en tête est celui qui a la ressource.
	 Il peut demander d'attendre un évènement (par exemple la saisie de texte pour un terminal)
*/

resource_t *create_resource();

void resource_request(resource_t *res);
void resource_wait_event(resource_t *res);
int resource_waiting_event(resource_t *res);
void resource_event(resource_t *res);
void resource_release(resource_t *res);

void free_resource(resource_t *res);

#endif
