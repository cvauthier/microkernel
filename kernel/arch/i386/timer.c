#include <stdlib.h>
#include <stdint.h>

#include <kernel/timer.h>
#include <kernel/process.h>

#define PIT_FREQUENCY 1193182

extern void load_idt(uint64_t *ptr);
extern void outb(uint16_t port, uint8_t val);

struct callout_t
{
	void (*func)(void*);
	void *arg;

	int ms_left;
};
typedef struct callout_t callout_t;

struct calloutll_t
{
	struct calloutll_t *next;
	callout_t callout;
};
typedef struct calloutll_t calloutll_t;

static calloutll_t *callout_list;
static int clock_on;

void clock_init()
{
	clock_on = 0;
	outb(0x43, 0x36); 
	/* Mode/command register
			Channel 0 : 0 0
			Access mode : 1 1 (lobyte/hibyte)
			Operating mode : 0 1 1 (square wave generator)
			Binary mode : 0 */
	
	uint16_t count = PIT_FREQUENCY/1000; // pour une milliseconde
	outb(0x40, count&0xFF);
	outb(0x40, (count>>8)&0xFF);

	callout_list = 0;
	clock_on = 1;
	total_time = 0;
}

void clock_tick()
{
	if (!clock_on)
		return;

	total_time++;
	if (callout_list != 0)
	{
		callout_list->callout.ms_left--;
		while (callout_list != 0 && callout_list->callout.ms_left == 0)
		{
			(*callout_list->callout.func)(callout_list->callout.arg);
			calloutll_t *next = callout_list->next;
			free(callout_list);
			callout_list = next;
		}
	}

	scheduler_tick();
}

void timeout(void (*func)(void*), void *arg, int ms)
{
	calloutll_t *new_elt = (calloutll_t*) malloc(sizeof(calloutll_t));
	new_elt->callout.func = func;
	new_elt->callout.arg = arg;

	calloutll_t **top = &callout_list;

	while (*top != 0 && (*top)->callout.ms_left <= ms)
	{
		ms -= (*top)->callout.ms_left;
		top = &((*top)->next);
	}
	new_elt->callout.ms_left = ms;

	new_elt->next = *top;
	*top = new_elt;
	if (new_elt->next != 0)
		new_elt->next->callout.ms_left -= ms;
}


