#ifndef VIRUSTD_H
#define VIRUSTD_H

int wait(int *pid, int *code);
int fork();
void exit(int code);

/*
TODO :

void *sbrk(int incr);
void exec(char *path);

*/

#endif
