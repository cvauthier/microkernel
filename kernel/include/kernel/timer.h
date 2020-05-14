#ifndef TIMER_H
#define TIMER_H

unsigned int total_time;

void clock_init();
void timeout(void (*func)(void*), void *arg, int ms);

#endif
