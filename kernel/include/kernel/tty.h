#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>
#include <kernel/keyboard.h>
#include <kernel/filesystem.h>

void terminal_basic_init(); // Pour pouvoir faire des printf le plus tôt possible

void terminal_clear();
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);

void terminal_init(); // Nécessite un gestionnaire de mémoire

file_descr_t *new_terminal();
file_descr_t *current_terminal();

int32_t term_obj_write(file_descr_t *fd, void *ptr, int32_t count);
int32_t term_obj_read(file_descr_t *fd, void *ptr, int32_t count);
void term_obj_close(file_descr_t *fd);

void terminal_kb_event(kb_event_t evt);

#endif
