#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"

// VGA константы
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_CURSOR_COMMAND_PORT 0x3D4
#define VGA_CURSOR_DATA_PORT 0x3D5

// Функции терминала
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_writestring(const char* data);
void print_number(uint32_t num);
void print_hex(uint32_t num);
void update_cursor(int x, int y);
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor(void);

#endif // TERMINAL_H