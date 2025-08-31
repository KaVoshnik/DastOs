// kernel.c
#define VIDEO_MEMORY 0xB8000    // Адрес видеопамяти в текстовом режиме
#define SCREEN_WIDTH 80         // Ширина экрана в символах
#define SCREEN_HEIGHT 25        // Высота экрана в строках
#define MAX_INPUT 80            // Максимальная длина буфера ввода

// Буфер для хранения введённых символов
char input_buffer[MAX_INPUT];
int input_pos = 0;

// Функция очистки экрана
void clear_screen() {
    char *video_memory = (char *)VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i += 2) {
        video_memory[i] = ' ';          // Символ
        video_memory[i + 1] = 0x07;     // Атрибут (белый на чёрном)
    }
}

// Функция вывода строки на экран
void print_string(const char *str) {
    char *video_memory = (char *)VIDEO_MEMORY + (input_pos / SCREEN_WIDTH + 1) * SCREEN_WIDTH * 2;
    while (*str && (video_memory < (char *)VIDEO_MEMORY + SCREEN_WIDTH * SCREEN_HEIGHT * 2)) {
        *video_memory++ = *str++;
        *video_memory++ = 0x07;     // Атрибут
    }
}

// Простая таблица преобразования скан-кодов в ASCII (только базовые клавиши)
const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// Обработчик прерываний клавиатуры
void keyboard_handler() {
    unsigned char scancode;
    asm volatile ("inb $0x60, %0" : "=a"(scancode)); // Читаем скан-код из порта 0x60
    if (scancode & 0x80) return; // Игнорируем отпускание клавиш
    char c = scancode_to_ascii[scancode & 0x7F];
    if (c && input_pos < MAX_INPUT - 1) {
        input_buffer[input_pos++] = c;
        input_buffer[input_pos] = 0; // Завершающий нуль
        print_string(&c); // Выводим символ
    }
}

// Основная функция ядра
void kernel_main() {
    clear_screen();
    print_string("MyOS v0.1 - Kernel Loaded Successfully!\n");

    input_buffer[0] = 0;
    input_pos = 0;

    asm volatile ("sti"); // Включаем прерывания
    while (1) {
        asm volatile ("hlt"); // Снижаем потребление CPU
    }
}