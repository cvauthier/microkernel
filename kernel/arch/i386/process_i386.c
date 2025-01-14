#include <kernel/process.h>
#include <kernel/filesystem.h>
#include <kernel/memory.h>
#include <kernel/syscalls.h>

#include <stdlib.h>
#include <string.h>

#include "memory_i386.h"
#include "process_i386.h"

void prepare_switch(process_t *p)
{
	// Les pages du kernel doivent rester les mêmes d'un processus à l'autre
	pde_t *pde = (pde_t*) temp_map(p->pd_addr, 0);
	for (int i = FIRST_KERNEL_PDE ; i < NB_PDE-1 ; i++)
		pde[i] = (cur_pd_addr())[i];

	load_pd(p->pd_addr);
	set_tss_stack(p->kernel_stack);
}

void context_switch(process_t *prev, process_t *next)
{
	prepare_switch(next);
	switch_proc(prev->hw_context, next->hw_context);
}

void first_context_switch(process_t *proc)
{
	prepare_switch(proc);
	switch_initial(proc->hw_context);
}

void setup_fork_switch(process_t *parent, process_t *child)
{
	/* On suppose ici que child a été créé par un appel à fork par parent alors qu'il s'exécutait en ring 3. 
		 La pile contient alors dans l'ordre SS, ESP, EFLAGS, CS, EIP, et les 8 registres empilés par PUSHA
		 
		 Le processus reprendra à forked_proc_start, qui fait un POPA puis un IRET */

	for (int i = 1 ; i <= 13 ; i++)
		child->kernel_stack[-i] = parent->kernel_stack[-i];

	child->kernel_stack[-5+PUSHA_EAX] = 0; // Fork renvoie 0
	child->hw_context->eip = (uint32_t) forked_proc_start;
	child->hw_context->esp = (uint32_t) (child->kernel_stack-13);
}

void setup_start_point(process_t *proc, int (*start)(void*), void *arg)
{
	// kernel_proc_start dépile start et arg, et fait syscall_exit(start(arg))

	proc->hw_context->eip = (uint32_t) kernel_proc_start;
	proc->hw_context->esp = (uint32_t) (proc->kernel_stack-2);
	proc->kernel_stack[-1] = (uint32_t) arg;
	proc->kernel_stack[-2] = (uint32_t) start;
}

hw_context_t *create_hw_context()
{
	return (hw_context_t*) calloc(1, sizeof(hw_context_t));
}

void free_hw_context(hw_context_t *ctx)
{
	free(ctx);
}

