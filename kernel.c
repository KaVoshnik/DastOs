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

void busy_wait(unsigned int cycles) {
    volatile unsigned int counter; // 'volatile' предотвращает оптимизацию компилятором
    for (counter = 0; counter < cycles; counter++) {
        // Пустой цикл для ожидания
        __asm__ volatile ("nop"); // Ассемблерная команда "no operation"
    }
}

// Функция для обновления символа в правом нижнем углу экрана как индикатор жизни
void update_indicator() {
    // Адрес правого нижнего угла экрана в текстовом режиме VGA (80x25)
    // Последний символ: позиция (79, 24)
    // Адрес в видеопамяти = (строка * 80 + столбец) * 2
    // Для (79, 24): адрес = (24 * 80 + 79) * 2 = (1920 + 79) * 2 = 1999 * 2 = 3998
    // Базовый адрес видеопамяти 0xB8000
    char *video_memory = (char*)VIDEO_MEMORY;
    int pos = (24 * 80 + 79) * 2; // Позиция для последнего символа
    
    static int state = 0; // Статическая переменная для хранения состояния
    char symbols[] = {'|', '/', '-', '\\'}; // Символы для анимации
    char colors[] = {0x07, 0x02, 0x04, 0x01}; // Разные цвета (светло-серый, зеленый, красный, синий)
    
    video_memory[pos] = symbols[state % 4]; // Меняем символ
    video_memory[pos + 1] = colors[state % 4]; // Меняем цвет
    
    state++; // Переходим к следующему состоянию
}


// Главная функция ядра
void kernel_main() {
    clear_screen();
    print_string("MyOS v0.1 - C Kernel Loaded Successfully!");
    
    while (1) {
        
        update_indicator();
        busy_wait(10000000);
        
        // Добавить больше действий здесь в будущем
        
        // ВАЖНО: Убедиться, что мы не выйдем за пределы функции.
    }
    // Не должно быть кода после while(1) в этой функции
    
    // Альтернативно, если вы хотите быть абсолютно уверенным, что
    // выполнение не пойдет дальше (хотя while(1) уже этого не допустит),
    // можно добавить jmp на метку внутри цикла в ассемблерной вставке,
    // но для простоты лучше оставить C-код.
}