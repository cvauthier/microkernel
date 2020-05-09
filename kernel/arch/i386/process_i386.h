#ifndef PROC_I386_H
#define PROC_I386_H

#include <stdint.h>

#define KERNEL_STACK_SIZE 4096

#define PUSHA_EAX -1
#define PUSHA_ECX -2
#define PUSHA_EDX -3
#define PUSHA_EBX -4
#define PUSHA_ESI -7
#define PUSHA_EDI -8

struct __attribute__((__packed__)) proc_data_t
{
	paddr_t pd_physical_addr;
	stackint_t *kernel_stack_addr;
	/*uint32_t code_begin;
	uint32_t code_end;
	uint32_t heap_begin;
	uint32_t heap_end;
	uint32_t stack_top;
	uint32_t stack_bottom;*/
	uint32_t eip;
	uint32_t esp;
};
typedef struct proc_data_t proc_data_t;

extern void jmp_user_proc(uint32_t *stack);
extern void switch_proc(proc_data_t *prev, proc_data_t *next);
extern void switch_initial(proc_data_t *init);
extern void kernel_proc_start();

proc_data_t *new_proc_data();

void syscall_handler(uint32_t *regs);

void reschedule();

#endif


