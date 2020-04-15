#ifndef TIMER_H
#define TIMER_H

void clock_init();

void timeout(void (*func)(void*), void *arg, int ms);

#endif
