#ifndef PROC_I386_H
#define PROC_I386_H

#include <stdint.h>

#define PUSHA_EAX -1
#define PUSHA_ECX -2
#define PUSHA_EDX -3
#define PUSHA_EBX -4
#define PUSHA_ESI -7
#define PUSHA_EDI -8

struct __attribute__((__packed__)) hw_context_t
{
	uint32_t eip;
	uint32_t esp;
};
typedef struct hw_context_t hw_context_t;

//extern void jmp_user_proc(uint32_t *stack);

extern void switch_proc(hw_context_t *prev, hw_context_t *next);
extern void switch_initial(hw_context_t *init);

extern void kernel_proc_start();
extern void forked_proc_start();

void syscall_handler(uint32_t *regs);

#endif


