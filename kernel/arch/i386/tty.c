#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <kernel/tty.h>
#include <kernel/utility.h>

#include "vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint16_t* const VGA_MEMORY = (uint16_t*) 0xC03FF000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// "General Terminal Interface" : https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap11.html

#define TERM_BUFF_SIZE 4096

typedef struct
{
	file_descr_t *fd;
	char *buffer;
	char *charsizes;
	int begin;
	int begin_draw;
	int begin_typed;
	int begin_typing;
	int pos;
	int nb_lines;
	int line_size;
} term_obj_t;

static dynarray_t *term_stack;

static int term_capslock;
static int term_shift;

void terminal_basic_init() 
{
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	terminal_clear();
}

void terminal_clear()
{
	terminal_row = 0;
	terminal_column = 0;
	for (size_t y = 0; y < VGA_HEIGHT; y++) 
	{
		for (size_t x = 0; x < VGA_WIDTH; x++) 
			terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) 
{
	terminal_buffer[y * VGA_WIDTH + x] = vga_entry(c, color);
}

void terminal_putchar(char c) 
{
	unsigned char uc = c;
	if (c != '\n')
		terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH || c == '\n') 
	{
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
		{
			terminal_row--;
			for (size_t y = 0 ; y+1 < VGA_HEIGHT ; y++)
			{
				for (size_t x = 0 ; x < VGA_WIDTH ; x++)
					terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y+1) * VGA_WIDTH + x];
			}
			for (size_t x = 0 ; x < VGA_WIDTH ; x++)
				terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', VGA_COLOR_BLACK);
		}
	}
}

void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}

void inc_index(int *i)
{
	(*i)++;
	if (*i == TERM_BUFF_SIZE)
		*i = 0;
}

void dec_index(int *i)
{
	if (!*i)
		*i = TERM_BUFF_SIZE;
	(*i)--;
}

int term_char_size(int lsize, char c)
{
	int size;
	switch(c)
	{
		case '\n':
			size = VGA_WIDTH;
			break;
		case '\t':
			size = 4;
			break;
		default:
			size = 1;
	}
	return (size+lsize > VGA_WIDTH) ? VGA_WIDTH-lsize : size;
}

void term_push_char(term_obj_t *term, char c)
{
	term->buffer[term->pos] = c;
	term->charsizes[term->pos] = term_char_size(term->line_size, c);
	term->line_size += term->charsizes[term->pos];
		
	inc_index(&term->pos);
	if (term->pos == term->begin) 			 inc_index(&term->begin);
	if (term->pos == term->begin_typed)  term->begin_typed = term->begin;
	if (term->pos == term->begin_typing) term->begin_typing = term->begin;

	if (term->line_size >= VGA_WIDTH)
	{
		term->line_size = 0;
		term->nb_lines++;
		if (term->nb_lines > VGA_HEIGHT)
		{
			term->nb_lines--;
			int lsize = 0;
			while (lsize < VGA_WIDTH)
			{
				lsize += term->charsizes[term->begin_draw];
				inc_index(&term->begin_draw);
			}
		}
	}
}

void term_pop_char(term_obj_t *term)
{
	if (term->pos == term->begin_typing)
		return;
	
	dec_index(&term->pos);
	
	if (term->line_size == 0)
	{
		term->line_size = VGA_WIDTH-term->charsizes[term->pos];
		if (term->nb_lines == VGA_HEIGHT)
		{
			int lsize = 0;
			int i = term->begin_draw;
			while (lsize < VGA_WIDTH)
			{
				if (i == term->begin)
					break;
				dec_index(&i);
				lsize += term->charsizes[i];
			}
			if (lsize == VGA_WIDTH)
			{
				term->begin_draw = i;
				term->nb_lines++;
			}
		}
		term->nb_lines--;
	}
	else
		term->line_size -= term->charsizes[term->pos];
}

void term_obj_draw(term_obj_t *term)
{
	terminal_clear();
	if (term->pos >= term->begin_draw)
		terminal_write(term->buffer+term->begin_draw, term->pos-term->begin_draw);
	else
	{
		terminal_write(term->buffer+term->begin_draw, TERM_BUFF_SIZE-term->begin_draw);
		terminal_write(term->buffer, term->pos);
	}
}

void term_clean_stack()
{
	while (term_stack->size)
	{
		term_obj_t *term = (term_obj_t*) term_stack->array[term_stack->size-1];
		if (term)
			return;
		dynarray_pop(term_stack);
	}
	new_terminal();
}

void terminal_init()
{
	term_capslock = term_shift = 0;
	term_stack = create_dynarray();
	new_terminal();
	dynarray_push(kb_callbacks, (void*) terminal_kb_event);
}

file_descr_t *current_terminal()
{
	term_clean_stack();
	return ((term_obj_t*)term_stack->array[term_stack->size-1])->fd;
}

void do_nothing()
{
}

file_descr_t *new_terminal()
{
	term_obj_t *term = (term_obj_t*) calloc(1, sizeof(term_obj_t));
	file_descr_t *fd = (file_descr_t*) calloc(1, sizeof(file_descr_t));

	term->buffer = (char*) malloc(TERM_BUFF_SIZE*sizeof(char));
	term->charsizes = (char*) malloc(TERM_BUFF_SIZE*sizeof(char));
	term->nb_lines = 1;
	term->line_size = 0;
	term->fd = fd;
	
	fd->inode = (uint32_t) term_stack->size;
	fd->size = TERM_BUFF_SIZE;
	fd->write = term_obj_write;
	fd->read = term_obj_read;
	fd->seek = (uint32_t(*)(file_descr_t*,int32_t,int)) do_nothing;
	fd->close = term_obj_close;
	
	dynarray_push(term_stack, (void*) term);
	return fd;
}

int32_t term_obj_write(file_descr_t *fd, void *ptr, int32_t count)
{
	if (count <= 0)
		return 0;

	term_obj_t *term = term_stack->array[fd->inode];
	char *buffer = (char*) ptr;
	int32_t count0 = count;

	while (count--)
	{
		term_push_char(term, *(buffer++));
	}
	term->begin_typed = term->begin_typing = term->pos;
	term_obj_draw(term);

	return count0;
}

int32_t term_obj_read(file_descr_t *fd, void *ptr, int32_t count)
{
	if (count <= 0)
		return 0;

	term_obj_t *term = term_stack->array[fd->inode];
	char *buffer = (char*) ptr;

	int32_t count0 = count;
	while (count)
	{
		if (term->begin_typed == term->begin_typing)
			break;
		count--;
		*buffer = term->buffer[term->begin_typed];
		inc_index(&term->begin_typed);
		if (*buffer == '\n')
			break;
		buffer++;
	}

	return count0-count;
}

void term_obj_close(file_descr_t *fd)
{
	int i = (int) fd->inode;
	term_obj_t *term = term_stack->array[i];
	free(term->buffer);
	free(term->charsizes);
	free(fd);
	free(term);
	term_stack->array[i] = 0;
}

void terminal_kb_event(kb_event_t evt)
{
	static const char key[10] = {')','!','@','#','$','%','^','&','*','('};

	file_descr_t *fd = current_terminal();
	term_obj_t *term = term_stack->array[fd->inode];

	if (evt.type == Kb_Pressed)
	{
		if (Kb_a <= evt.key && evt.key <= Kb_z)
		{
			char base = (term_capslock != !!term_shift)  ? 'A' : 'a';
			term_push_char(term, evt.key-Kb_a+base);
		}
		else if (Kb_0 <= evt.key && evt.key <= Kb_9)
		{
			int n = evt.key-Kb_0;
			term_push_char(term, term_shift ? key[n] : n+'0');
		}
		else
		{
			switch (evt.key)
			{	
				case Kb_LBra:
					term_push_char(term, term_shift ? '{' : '[');
					break;
				case Kb_RBra:
					term_push_char(term, term_shift ? '}' : ']');
					break;
				case Kb_Scol:
					term_push_char(term, term_shift ? ':' : ';');
					break;
				case Kb_Quote:
					term_push_char(term, term_shift ? '"' : '\'');
					break;
				case Kb_Antislash:
					term_push_char(term, term_shift ? '\\' : '|');
					break;
				case Kb_Comma:
					term_push_char(term, term_shift ? '<' : ',');
					break;
				case Kb_Period:
					term_push_char(term, term_shift ? '>' : '.');
					break;
				case Kb_Slash:
					term_push_char(term, term_shift ? '?' : '/');
					break;
				case Kb_Backtick:
					term_push_char(term, term_shift ? '~' : '`');
					break;
				case Kb_Minus:
					term_push_char(term, term_shift ? '_' : '-');
					break;
				case Kb_Equal:
					term_push_char(term, term_shift ? '+' : '=');
					break;
				case Kb_Enter:
					term_push_char(term, '\n');
					term->begin_typing = term->pos;
					break;
				case Kb_Tab:
					term_push_char(term, '\t');
					break;
				case Kb_Space:
					term_push_char(term, ' ');
					break;
				case Kb_Backspace:
					term_pop_char(term);
					break;
				case Kb_CapsLock:
					term_capslock = !term_capslock;
					break;
				case Kb_LShift:
					term_shift |= 2;
					break;
				case Kb_RShift:
					term_shift |= 1;
					break;
				default:
					break;
			}
		}
		term_obj_draw(term);
	}
	else if (evt.type == Kb_Released)
	{
		switch (evt.key)
		{
			case Kb_LShift:
				term_shift &= 1;
				break;
			case Kb_RShift:
				term_shift &= 2;
				break;
			default:
				break;
		}
	}
}

