// kernel.c - Kernel with simple shell for DastOs
// Compile with: i686-elf-gcc -ffreestanding -m32 -c kernel.c -o kernel.o
// Link with: i686-elf-ld -Ttext 0x1000 --oformat binary kernel.o -o kernel.bin

#define VIDEO_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define WHITE_ON_BLACK 0x07

static char* video_ptr = (char*)VIDEO_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;

// Очистка экрана
void clear_screen() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i += 2) {
        *(video_ptr + i) = ' ';
        *(video_ptr + i + 1) = WHITE_ON_BLACK;
    }
    cursor_x = 0;
    cursor_y = 0;
    video_ptr = (char*)VIDEO_MEMORY;
}

// Вывод символа
void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        *(video_ptr++) = c;
        *(video_ptr++) = WHITE_ON_BLACK;
        cursor_x++;
    }

    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= SCREEN_HEIGHT) {
        // Прокрутка экрана
        for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i++) {
            *(video_ptr + i - SCREEN_WIDTH * 2) = *(video_ptr + i);
        }
        for (int i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i < SCREEN_HEIGHT * SCREEN_WIDTH * 2; i += 2) {
            *(video_ptr + i - SCREEN_WIDTH * 2) = ' ';
            *(video_ptr + i - SCREEN_WIDTH * 2 + 1) = WHITE_ON_BLACK;
        }
        cursor_y--;
        video_ptr -= SCREEN_WIDTH * 2;
    }
}

// Вывод строки
void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

// Чтение символа с клавиатуры (простое опрашивание порта)
char get_char() {
    unsigned char c;
    do {
        asm volatile ("inb $0x60, %0" : "=a"(c));
    } while (c == 0);
    // Базовая конверсия скан-кодов в ASCII (только буквы и некоторые символы)
    if (c >= 0x02 && c <= 0x1A) { // a-z
        return 'a' + (c - 0x02);
    } else if (c == 0x1C) { // Enter
        return '\n';
    } else if (c == 0x0E) { // Backspace
        return '\b';
    }
    return 0;
}

// Простое сравнение строк
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Шелл
void shell() {
    char command[256];
    int idx = 0;

    print_string("DastOs Shell > ");

    while (1) {
        char c = get_char();
        if (c == '\n') {
            command[idx] = '\0';
            print_char('\n');

            if (strcmp(command, "help") == 0) {
                print_string("Commands: help, clear, exit\n");
            } else if (strcmp(command, "clear") == 0) {
                clear_screen();
            } else if (strcmp(command, "exit") == 0) {
                print_string("Shutting down...\n");
                while (1); // Зацикливаемся вместо выключения
            } else if (command[0] != '\0') {
                print_string("Unknown command: ");
                print_string(command);
                print_char('\n');
            }

            idx = 0;
            print_string("DastOs Shell > ");
        } else if (c == '\b') {
            if (idx > 0) {
                idx--;
                cursor_x--;
                video_ptr -= 2;
                print_char(' ');
                video_ptr -= 2;
            }
        } else if (c >= 'a' && c <= 'z') {
            command[idx++] = c;
            print_char(c);
        }
    }
}

// Точка входа ядра
void kernel_main() {
    clear_screen();
    print_string("Welcome to DastOs!\n");
    shell();
}