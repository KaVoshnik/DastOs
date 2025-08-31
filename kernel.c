#define VIDEO_MEMORY 0xB8000
#define MAX_INPUT 256

// Структура для позиции курсора
int cursor_x = 0;
int cursor_y = 0;
char input_buffer[MAX_INPUT];
int input_index = 0;

void clear_screen() {
    char *video_memory = (char*)VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x07;
    }
    cursor_x = 0;
    cursor_y = 0;
}

void print_char(char c, char attr) {
    char *video_memory = (char*)VIDEO_MEMORY;
    int position = (cursor_y * 80 + cursor_x) * 2;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        video_memory[position] = c;
        video_memory[position + 1] = attr;
        cursor_x++;
    }
    
    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= 25) {
        // Прокрутка экрана
        for (int i = 0; i < 80 * 24 * 2; i++) {
            video_memory[i] = video_memory[i + 80 * 2];
        }
        for (int i = 80 * 24 * 2; i < 80 * 25 * 2; i += 2) {
            video_memory[i] = ' ';
            video_memory[i + 1] = 0x07;
        }
        cursor_y = 24;
    }
}

void print_string(const char *str) {
    while (*str) {
        print_char(*str, 0x07);
        str++;
    }
}

void print_newline() {
    print_char('\n', 0x07);
}

void print_prompt() {
    print_string("> ");
}

// Функция сравнения строк
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Обработка команд
void process_command(char *input) {
    if (strcmp(input, "clear") == 0) {
        clear_screen();
        print_prompt();
    } else if (strcmp(input, "help") == 0) {
        print_newline();
        print_string("Available commands:");
        print_newline();
        print_string("  clear - Clear screen");
        print_newline();
        print_string("  help  - Show this help");
        print_newline();
        print_string("  reboot - Reboot system");
        print_newline();
        print_prompt();
    } else if (strcmp(input, "reboot") == 0) {
        print_newline();
        print_string("Rebooting...");
        // Команда перезагрузки через клавиатурный контроллер
        asm volatile ("outb %0, $0x64" : : "a"((char)0xFE));
    } else if (input[0] != 0) {
        print_newline();
        print_string("Unknown command: ");
        print_string(input);
        print_newline();
        print_prompt();
    }
}

// Главная функция ядра
void kernel_main() {
    clear_screen();
    print_string("MyOS v0.1 - Functional Shell");
    print_newline();
    print_string("Type 'help' for available commands");
    print_newline();
    print_prompt();
    
    // Бесконечный цикл
    while (1) {
        asm volatile ("hlt");
    }
}