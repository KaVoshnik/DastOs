// kernel.c
#define VIDEO_MEMORY 0xB8000
#define MAX_INPUT 80

char input_buffer[MAX_INPUT];
int input_pos = 0;

void clear_screen() {
    char *video_memory = (char*)VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x07;
    }
}

void print_string(const char *str) {
    char *video_memory = (char*)VIDEO_MEMORY + 160; // Начать со второй строки
    while (*str) {
        *video_memory++ = *str++;
        *video_memory++ = 0x07;
    }
}

const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

void keyboard_handler() {
    unsigned char scancode;
    asm volatile ("inb $0x60, %0" : "=a"(scancode));
    if (scancode & 0x80) return;
    char c = scancode_to_ascii[scancode & 0x7F];
    if (c && input_pos < MAX_INPUT - 1) {
        input_buffer[input_pos++] = c;
        input_buffer[input_pos] = 0;
        print_string(&c);
    }
}

void kernel_main() {
    clear_screen();
    print_string("MyOS v0.1 - C Kernel Loaded Successfully!");
    
    input_buffer[0] = 0;
    input_pos = 0;
    
    asm volatile ("sti");
    while (1) {
        asm volatile ("hlt");
    }
}
