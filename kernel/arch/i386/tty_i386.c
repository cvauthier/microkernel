#include <kernel/tty.h>

#include "vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint16_t* const VGA_MEMORY = (uint16_t*) 0xC03FF000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

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

void terminal_show_cursor()
{
	uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos>>8) & 0xFF));
}

int terminal_width()
{
	return VGA_WIDTH;
}

int terminal_height()
{
	return VGA_HEIGHT;
}


