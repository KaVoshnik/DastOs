// Полнофункциональное ядро с поддержкой клавиатуры

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

// IDT структуры
struct idt_entry {
    uint16_t base_lo, sel;
    uint8_t always0, flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Глобальные переменные
struct idt_entry idt[256];
struct idt_ptr idtp;
int terminal_row = 0, terminal_column = 0;

// Scancode таблица
const char scancode_to_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,'*',0,' '
};

// Порты
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Управление курсором
void update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// Терминал
void terminal_clear(void) {
    uint16_t* video = (uint16_t*)VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video[i] = 0x0720; // Пробел с белым текстом
    }
    terminal_row = terminal_column = 0;
    update_cursor(terminal_column, terminal_row);
}

void terminal_putchar(char c) {
    uint16_t* video = (uint16_t*)VGA_MEMORY;

    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            // Простая прокрутка экрана
            for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
                video[i] = video[i + VGA_WIDTH];
            }
            for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
                video[i] = 0x0720;
            }
            terminal_row = VGA_HEIGHT - 1;
        }
        update_cursor(terminal_column, terminal_row);
        return;
    }

    if (c == '\b' && terminal_column > 0) {
        terminal_column--;
        video[terminal_row * VGA_WIDTH + terminal_column] = 0x0720;
        update_cursor(terminal_column, terminal_row);
        return;
    }

    if (c >= 32 && c <= 126) { // Только печатные символы
        video[terminal_row * VGA_WIDTH + terminal_column] = (0x07 << 8) | c;

        if (++terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row >= VGA_HEIGHT) {
                // Простая прокрутка экрана
                for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
                    video[i] = video[i + VGA_WIDTH];
                }
                for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
                    video[i] = 0x0720;
                }
                terminal_row = VGA_HEIGHT - 1;
            }
        }
        update_cursor(terminal_column, terminal_row);
    }
}

void terminal_writestring(const char* data) {
    while (*data) terminal_putchar(*data++);
}

// IDT функции
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

extern void idt_flush(void);
extern void irq1_handler(void);
extern void exception_handler(void);

// Шелл - буфер команд и состояние
#define COMMAND_BUFFER_SIZE 256
char command_buffer[COMMAND_BUFFER_SIZE];
int command_length = 0;
int shell_ready = 0;

void handle_exception(void) {
    terminal_writestring("EXCEPTION OCCURRED! System halted.\n");
    while(1) asm volatile("hlt");
}

// Функция полной инициализации системы для совместимости с GRUB
void initialize_system(void) {
    // ПОЛНОЕ отключение прерываний
    asm volatile("cli");
    
    // Сброс клавиатурного контроллера
    // Ждем пока контроллер будет готов
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
    outb(0x64, 0xAD); // Отключаем клавиатуру
    
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
    outb(0x64, 0xA7); // Отключаем мышь
    
    // Очищаем буфер клавиатуры
    while (inb(KEYBOARD_STATUS_PORT) & 0x01) {
        inb(KEYBOARD_DATA_PORT);
    }
    
    // ПОЛНЫЙ сброс PIC
    // Маскируем все прерывания на время инициализации
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
    
    // Инициализация главного PIC
    outb(0x20, 0x11); // ICW1: Инициализация + требуется ICW4
    outb(0x21, 0x20); // ICW2: Смещение прерываний на 0x20 (32)
    outb(0x21, 0x04); // ICW3: Подчиненный PIC на IRQ2
    outb(0x21, 0x01); // ICW4: Режим 8086
    
    // Инициализация подчиненного PIC
    outb(0xA0, 0x11); // ICW1: Инициализация + требуется ICW4
    outb(0xA1, 0x28); // ICW2: Смещение прерываний на 0x28 (40)
    outb(0xA1, 0x02); // ICW3: Подключен к главному PIC через IRQ2
    outb(0xA1, 0x01); // ICW4: Режим 8086
    
    // Включаем клавиатуру обратно
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
    outb(0x64, 0xAE); // Включаем клавиатуру
    
    // Включаем только клавиатурное прерывание
    outb(0x21, 0xFD); // Маска: все заблокированы кроме IRQ1
    outb(0xA1, 0xFF); // Маска: все заблокированы на подчиненном PIC
}

// Функции шелла
void shell_prompt(void) {
    terminal_writestring("MyOS> ");
}

void command_clear(void) {
    terminal_clear();
    terminal_writestring("MyOS Shell v1.0\n");
    terminal_writestring("===============\n\n");
}

void command_help(void) {
    terminal_writestring("Available commands:\n");
    terminal_writestring("  help    - Show this help message\n");
    terminal_writestring("  clear   - Clear the screen\n");
    terminal_writestring("  about   - Show system information\n");
    terminal_writestring("  reboot  - Restart the system\n");
    terminal_writestring("\n");
}

void command_about(void) {
    terminal_writestring("MyOS - Simple Operating System\n");
    terminal_writestring("Version: 0.2\n");
    terminal_writestring("Features: Keyboard input, Basic shell\n");
    terminal_writestring("Written in: C and Assembly\n");
    terminal_writestring("Written by: Kavoshnik");
    terminal_writestring("\n");
}

void command_reboot(void) {
    terminal_writestring("Rebooting system...\n");
    // Простая перезагрузка через клавиатурный контроллер
    outb(0x64, 0xFE);
    while(1) asm volatile("hlt");
}

// Простое сравнение строк
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

void execute_command(const char* command) {
    if (strcmp(command, "") == 0) {
        // Пустая команда
        return;
    } else if (strcmp(command, "help") == 0) {
        command_help();
    } else if (strcmp(command, "clear") == 0) {
        command_clear();
    } else if (strcmp(command, "about") == 0) {
        command_about();
    } else if (strcmp(command, "reboot") == 0) {
        command_reboot();
    } else {
        terminal_writestring("Unknown command: ");
        terminal_writestring(command);
        terminal_writestring("\n");
        terminal_writestring("Type 'help' for available commands.\n");
    }
}

// Обработчик клавиатуры с поддержкой шелла
void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Проверяем, что это нажатие клавиши (не отпускание)
    if (!(scancode & 0x80) && scancode < 128) {
        char ascii = scancode_to_ascii[scancode];
        
        if (ascii) {
            if (ascii == '\n' || ascii == '\r') { // Enter
                terminal_putchar('\n');
                if (shell_ready) {
                    command_buffer[command_length] = '\0';
                    execute_command(command_buffer);
                    command_length = 0;
                    shell_prompt();
                }
            } else if (ascii == '\b') { // Backspace
                if (command_length > 0) {
                    command_length--;
                    // Простая реализация backspace - стираем символ
                    terminal_putchar('\b');
                    terminal_putchar(' ');
                    terminal_putchar('\b');
                }
            } else if (ascii >= 32 && ascii <= 126) { // Печатные символы
                if (command_length < COMMAND_BUFFER_SIZE - 1) {
                    command_buffer[command_length++] = ascii;
                    terminal_putchar(ascii);
                }
            }
        }
    }
    
    // Отправляем сигнал завершения прерывания
    outb(PIC1_COMMAND, PIC_EOI);
}

// Главная функция
void kernel_main(void) {
    terminal_clear();
    terminal_writestring("MyOS v0.2!\n");
    terminal_writestring("==========\n\n");

    // Настройка IDT
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)&idt;

    // Инициализируем все записи IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Устанавливаем обработчики исключений (0-31)
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)exception_handler, 0x08, 0x8E);
    }

    // Устанавливаем обработчик клавиатуры (IRQ1 = прерывание 33)
    idt_set_gate(33, (uint32_t)irq1_handler, 0x08, 0x8E);
    idt_flush();

    // ПОЛНАЯ инициализация системы для работы с GRUB
    initialize_system();

    // Включаем прерывания
    asm volatile("sti");

    // Инициализация шелла
    command_clear();
    terminal_writestring("Welcome to MyOS!\n");
    terminal_writestring("Type 'help' for available commands.\n\n");
    
    shell_ready = 1;
    shell_prompt();

    while (1) asm volatile("hlt");
}