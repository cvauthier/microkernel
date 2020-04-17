#ifndef PROC_I386_H
#define PROC_I386_H

#include <stdint.h>

#define KERNEL_STACK_SIZE 4096

#define STACK_SS -1
#define STACK_ESP -2
#define STACK_EFLAGS -3
#define STACK_CS -4
#define STACK_EIP -5
#define STACK_EAX -6
#define STACK_ECX -7
#define STACK_EDX -8
#define STACK_EBX -9
#define STACK_ESI -12
#define STACK_EDI -13
#define STACK_BOTTOM -13

#define STACK_R0_OFS 2

struct proc_data_t
{
	uint32_t pd_physical_addr;
	uint32_t *kernel_stack_addr;
};
typedef struct proc_data_t proc_data_t;

extern uint32_t temp_page_1;
extern uint32_t temp_page_2;
extern uint32_t temp_page_3;
extern uint32_t temp_page_4;
extern uint32_t temp_page_5;
extern uint32_t temp_page_6;

void jmp_user_proc(uint32_t *stack);

void syscall_handler();

void reschedule();

#endif


