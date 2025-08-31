// Функции для работы с видеоадаптером
#define VIDEO_MEMORY 0xB8000

void clear_screen() {
    char *video_memory = (char*)VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x07;
    }
}

void print_string(const char *str) {
    char *video_memory = (char*)VIDEO_MEMORY;
    while (*str) {
        *video_memory++ = *str++;
        *video_memory++ = 0x07;
    }
}

// Главная функция ядра
void kernel_main() {
    clear_screen();
    print_string("MyOS v0.1 - C Kernel Loaded Successfully!");
    
    // Бесконечный цикл
    while (1) {
        // Здесь может быть код планировщика и обработки прерываний
    }
}