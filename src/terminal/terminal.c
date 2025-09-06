#include "../../include/terminal.h"
#include "../../include/utils.h"

// Глобальные переменные терминала
int terminal_row = 0, terminal_column = 0;

// Функции для управления VGA курсором
void update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    
    outb(VGA_CURSOR_COMMAND_PORT, 0x0F);
    outb(VGA_CURSOR_DATA_PORT, (uint8_t)(pos & 0xFF));
    outb(VGA_CURSOR_COMMAND_PORT, 0x0E);
    outb(VGA_CURSOR_DATA_PORT, (uint8_t)((pos >> 8) & 0xFF));
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(VGA_CURSOR_COMMAND_PORT, 0x0A);
    outb(VGA_CURSOR_DATA_PORT, (inb(VGA_CURSOR_DATA_PORT) & 0xC0) | cursor_start);
    
    outb(VGA_CURSOR_COMMAND_PORT, 0x0B);
    outb(VGA_CURSOR_DATA_PORT, (inb(VGA_CURSOR_DATA_PORT) & 0xE0) | cursor_end);
}

void disable_cursor() {
    outb(VGA_CURSOR_COMMAND_PORT, 0x0A);
    outb(VGA_CURSOR_DATA_PORT, 0x20);
}

void terminal_clear(void) {
    uint16_t* video_memory = (uint16_t*)VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_memory[i] = 0x0720; // Пробел с белым текстом на черном фоне
    }
    terminal_row = 0;
    terminal_column = 0;
    update_cursor(terminal_column, terminal_row);
}

void terminal_putchar(char c) {
    uint16_t* video_memory = (uint16_t*)VGA_MEMORY;
    
    if (c == '\n') {
        terminal_row++;
        terminal_column = 0;
    } else if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            video_memory[terminal_row * VGA_WIDTH + terminal_column] = 0x0720;
        }
        update_cursor(terminal_column, terminal_row);
        return;
    } else {
        video_memory[terminal_row * VGA_WIDTH + terminal_column] = (0x07 << 8) | c;
        terminal_column++;
    }
    
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }
    
    if (terminal_row >= VGA_HEIGHT) {
        // Прокрутка экрана вверх
        for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            video_memory[i] = video_memory[i + VGA_WIDTH];
        }
        for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
            video_memory[i] = 0x0720;
        }
        terminal_row = VGA_HEIGHT - 1;
    }
    
    update_cursor(terminal_column, terminal_row);
}

void terminal_writestring(const char* data) {
    while (*data) {
        terminal_putchar(*data++);
    }
}

void print_number(uint32_t num) {
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    
    char buffer[12];
    int i = 0;
    
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}

void print_hex(uint32_t num) {
    terminal_writestring("0x");
    
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    
    char buffer[9];
    int i = 0;
    
    while (num > 0) {
        int digit = num % 16;
        buffer[i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        num /= 16;
    }
    
    while (--i >= 0) {
        terminal_putchar(buffer[i]);
    }
}