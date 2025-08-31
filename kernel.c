// kernel.c - Kernel with simple shell for DastOs
// Compile with: gcc -ffreestanding -m32 -c kernel.c -o kernel.o
// Link with: ld -m elf_i386 -Ttext 0x7E00 --oformat binary kernel.o -o kernel.bin

#include <stddef.h>

// Simple VGA text mode functions
#define VIDEO_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

static char* video_ptr = (char*)VIDEO_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;

void clear_screen() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i += 2) {
        *((char*)VIDEO_MEMORY + i) = ' ';
        *((char*)VIDEO_MEMORY + i + 1) = 0x07;  // Light grey on black
    }
    cursor_x = 0;
    cursor_y = 0;
    video_ptr = (char*)VIDEO_MEMORY;
}

void print_char(char c, char color) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        *video_ptr++ = c;
        *video_ptr++ = color;
        cursor_x++;
    }

    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= SCREEN_HEIGHT) {
        // Simple scroll (shift up)
        for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i++) {
            *((char*)VIDEO_MEMORY + i) = *((char*)VIDEO_MEMORY + i + SCREEN_WIDTH * 2);
        }
        for (int i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i < SCREEN_HEIGHT * SCREEN_WIDTH * 2; i += 2) {
            *((char*)VIDEO_MEMORY + i) = ' ';
            *((char*)VIDEO_MEMORY + i + 1) = 0x07;
        }
        cursor_y = SCREEN_HEIGHT - 1;
    }

    video_ptr = (char*)VIDEO_MEMORY + (cursor_y * SCREEN_WIDTH + cursor_x) * 2;
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str++, 0x07);  // Light grey on black
    }
}

// Simple keyboard input (polling)
char get_char() {
    char c = 0;
    while (c == 0) {
        asm volatile (
            "inb $0x60, %%al\n"
            "movb %%al, %0"
            : "=r" (c)
        );
    }
    return c;
}

// Simple shell
void shell() {
    char command[256];
    int idx = 0;

    print_string("DastOs Shell > ");

    while (1) {
        char c = get_char();
        if (c == 0x0D) {  // Enter key (ASCII 13)
            command[idx] = '\0';
            print_char('\n', 0x07);

            // Process command
            if (strcmp(command, "help") == 0) {
                print_string("Available commands: help, clear, exit\n");
            } else if (strcmp(command, "clear") == 0) {
                clear_screen();
            } else if (strcmp(command, "exit") == 0) {
                print_string("Shutting down...\n");
                while (1);  // Infinite loop as shutdown
            } else {
                print_string("Unknown command: ");
                print_string(command);
                print_char('\n', 0x07);
            }

            idx = 0;
            print_string("DastOs Shell > ");
        } else if (c == 0x08) {  // Backspace
            if (idx > 0) {
                idx--;
                cursor_x--;
                video_ptr -= 2;
                print_char(' ', 0x07);
                video_ptr -= 2;  // Move back again
            }
        } else if (c >= ' ' && c <= '~') {  // Printable chars
            command[idx++] = c;
            print_char(c, 0x07);
        }
    }
}

// Kernel entry point
void kernel_main() {
    clear_screen();
    print_string("Welcome to DastOs!\n");
    shell();
}

// Simple strcmp implementation
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}