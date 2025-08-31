// kernel.c
#include <stdint.h>

// Простая реализация VGA-буфера
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
int cursor_x = 0;
int cursor_y = 0;
uint8_t color = 0x0F; // Белый текст на черном фоне

void clear_screen() {
    for (int i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = (color << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}

void put_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= 25) {
            cursor_y = 24;
            // Простая прокрутка
            for (int i = 0; i < 24 * 80; i++) {
                vga_buffer[i] = vga_buffer[i + 80];
            }
            for (int i = 24 * 80; i < 25 * 80; i++) {
                vga_buffer[i] = (color << 8) | ' ';
            }
        }
        return;
    }

    vga_buffer[cursor_y * 80 + cursor_x] = (color << 8) | c;
    cursor_x++;
    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= 25) {
            cursor_y = 24;
            for (int i = 0; i < 24 * 80; i++) {
                vga_buffer[i] = vga_buffer[i + 80];
            }
            for (int i = 24 * 80; i < 25 * 80; i++) {
                vga_buffer[i] = (color << 8) | ' ';
            }
        }
    }
}

void print(const char* str) {
    while (*str) {
        put_char(*str++);
    }
}

void shell() {
    print("DastOS Shell > ");
    // Простая команда: "help"

    // Эмуляция ввода команды (в данном случае просто жестко заданная команда)
    const char* cmd = "help";

    print(cmd);
    print("\n");

    if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p') {
        print("Available commands:\n");
        print(" - help\n");
        print(" - clear\n");
        print(" - exit\n");
    } else if (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r') {
        clear_screen();
    } else if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't') {
        print("Shutting down...\n");
        asm volatile("cli; hlt");
    } else {
        print("Unknown command.\n");
    }
}

void kernel_main() {
    clear_screen();
    print("Welcome to DastOS!\n");
    shell();
}