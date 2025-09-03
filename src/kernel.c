// Полнофункциональное ядро с поддержкой клавиатуры и управления памятью

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned int   size_t;

#define NULL ((void*)0)

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

// Управление памятью
#define HEAP_START 0x400000    // Начало кучи (4 MB)
#define HEAP_SIZE  0x100000    // Размер кучи (1 MB)
#define BLOCK_SIZE sizeof(memory_block_t)

// Виртуальная память и пейджинг
#define PAGE_SIZE           4096        // Размер страницы 4KB
#define PAGE_ENTRIES        1024        // Количество записей в таблице страниц
#define PAGE_DIRECTORY_SIZE PAGE_ENTRIES * sizeof(uint32_t)
#define PAGE_TABLE_SIZE     PAGE_ENTRIES * sizeof(uint32_t)

// Флаги для Page Directory Entry и Page Table Entry
#define PAGE_PRESENT     0x001  // Страница присутствует в памяти
#define PAGE_WRITABLE    0x002  // Страница доступна для записи
#define PAGE_USER        0x004  // Страница доступна пользователю
#define PAGE_PWT         0x008  // Page Write Through
#define PAGE_PCD         0x010  // Page Cache Disable
#define PAGE_ACCESSED    0x020  // Страница была accessed
#define PAGE_DIRTY       0x040  // Страница была изменена
#define PAGE_PS          0x080  // Page Size (только для PDE)
#define PAGE_GLOBAL      0x100  // Глобальная страница

// Адреса для размещения структур пейджинга
#define PAGE_DIRECTORY_ADDR  0x300000   // 3MB - Page Directory
#define PAGE_TABLES_ADDR     0x301000   // 3MB+4KB - Page Tables

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

// Структура блока памяти
typedef struct memory_block {
    size_t size;                    // Размер блока
    int is_free;                    // Флаг: свободен ли блок
    struct memory_block* next;      // Указатель на следующий блок
    struct memory_block* prev;      // Указатель на предыдущий блок
} memory_block_t;

// Структуры для виртуальной памяти
typedef uint32_t page_directory_entry_t;   // PDE - 32-битная запись
typedef uint32_t page_table_entry_t;       // PTE - 32-битная запись

// Структура Page Directory
typedef struct {
    page_directory_entry_t entries[PAGE_ENTRIES];
} page_directory_t;

// Структура Page Table
typedef struct {
    page_table_entry_t entries[PAGE_ENTRIES];
} page_table_t;

// Глобальные переменные
struct idt_entry idt[256];
struct idt_ptr idtp;
int terminal_row = 0, terminal_column = 0;

// Переменные управления памятью
memory_block_t* heap_start = NULL;
uint32_t total_memory = 0;
uint32_t free_memory = 0;
uint32_t used_memory = 0;

// Переменные виртуальной памяти
page_directory_t* page_directory = (page_directory_t*)PAGE_DIRECTORY_ADDR;
page_table_t* page_tables = (page_table_t*)PAGE_TABLES_ADDR;
int paging_enabled = 0;

// Предварительные объявления всех функций
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void print_number(uint32_t num);

// Функции для работы с виртуальной памятью (из interrupts.asm)
extern void enable_paging(uint32_t page_directory_addr);
extern void disable_paging(void);
extern uint32_t get_page_fault_address(void);
extern void flush_tlb(void);
extern void page_fault_handler(void);

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

// Функции для работы с памятью
void* memset(void* dest, int val, size_t len) {
    uint8_t* ptr = (uint8_t*)dest;
    while (len-- > 0) {
        *ptr++ = val;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t len) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (len-- > 0) {
        *d++ = *s++;
    }
    return dest;
}

// Инициализация системы управления памятью
void init_memory_management() {
    heap_start = (memory_block_t*)HEAP_START;
    heap_start->size = HEAP_SIZE - BLOCK_SIZE;
    heap_start->is_free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    total_memory = HEAP_SIZE;
    free_memory = HEAP_SIZE - BLOCK_SIZE;
    used_memory = BLOCK_SIZE;
}

// Выделение памяти (простой аллокатор)
void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    // Выравниваем размер по границе 4 байта
    size = (size + 3) & ~3;
    
    memory_block_t* current = heap_start;
    
    // Ищем подходящий свободный блок
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Если блок намного больше нужного, разделяем его
            if (current->size > size + BLOCK_SIZE + 16) {
                memory_block_t* new_block = (memory_block_t*)((uint32_t)current + BLOCK_SIZE + size);
                new_block->size = current->size - size - BLOCK_SIZE;
                new_block->is_free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                
                if (current->next) {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = size;
            }
            
            current->is_free = 0;
            free_memory -= current->size;
            used_memory += current->size;
            
            return (void*)((uint32_t)current + BLOCK_SIZE);
        }
        current = current->next;
    }
    
    return NULL; // Память не найдена
}

// Освобождение памяти
void kfree(void* ptr) {
    if (ptr == NULL) return;
    
    memory_block_t* block = (memory_block_t*)((uint32_t)ptr - BLOCK_SIZE);
    
    if (block->is_free) return; // Уже освобожден
    
    block->is_free = 1;
    free_memory += block->size;
    used_memory -= block->size;
    
    // Объединяем смежные свободные блоки
    if (block->next && block->next->is_free) {
        block->size += block->next->size + BLOCK_SIZE;
        if (block->next->next) {
            block->next->next->prev = block;
        }
        block->next = block->next->next;
    }
    
    if (block->prev && block->prev->is_free) {
        block->prev->size += block->size + BLOCK_SIZE;
        if (block->next) {
            block->next->prev = block->prev;
        }
        block->prev->next = block->next;
    }
}

// Получение информации о памяти
void get_memory_info(uint32_t* total, uint32_t* free, uint32_t* used) {
    *total = total_memory;
    *free = free_memory;
    *used = used_memory;
}

// === ФУНКЦИИ ВИРТУАЛЬНОЙ ПАМЯТИ ===

// Создание записи Page Directory Entry
uint32_t create_pde(uint32_t page_table_addr, uint32_t flags) {
    return (page_table_addr & 0xFFFFF000) | (flags & 0xFFF);
}

// Создание записи Page Table Entry  
uint32_t create_pte(uint32_t physical_addr, uint32_t flags) {
    return (physical_addr & 0xFFFFF000) | (flags & 0xFFF);
}

// Получение адреса Page Table из PDE
uint32_t get_page_table_addr(uint32_t pde) {
    return pde & 0xFFFFF000;
}

// Получение физического адреса из PTE
uint32_t get_physical_addr(uint32_t pte) {
    return pte & 0xFFFFF000;
}

// Инициализация системы виртуальной памяти
void init_virtual_memory(void) {
    terminal_writestring("Initializing virtual memory...\n");
    
    // Очищаем Page Directory
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        page_directory->entries[i] = 0;
    }
    
    // Создаем identity mapping для первых 4MB памяти (ядро)
    // Используем первую Page Table для этого
    page_table_t* kernel_page_table = &page_tables[0];
    
    // Заполняем первую Page Table identity mapping
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        uint32_t physical_addr = i * PAGE_SIZE;  // 0x0, 0x1000, 0x2000...
        kernel_page_table->entries[i] = create_pte(physical_addr, 
            PAGE_PRESENT | PAGE_WRITABLE);
    }
    
    // Записываем первую PDE чтобы указать на первую Page Table
    uint32_t kernel_page_table_addr = (uint32_t)kernel_page_table;
    page_directory->entries[0] = create_pde(kernel_page_table_addr, 
        PAGE_PRESENT | PAGE_WRITABLE);
    
    terminal_writestring("Identity mapping for kernel created\n");
    
    // Создаем mapping для кучи (начиная с 4MB)
    uint32_t heap_start_page = HEAP_START / PAGE_SIZE;  // Номер страницы для 4MB
    uint32_t heap_directory_index = heap_start_page / PAGE_ENTRIES;
    uint32_t heap_page_index = heap_start_page % PAGE_ENTRIES;
    
    // Используем вторую Page Table для кучи
    page_table_t* heap_page_table = &page_tables[1];
    
    // Заполняем Page Table для кучи
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        uint32_t physical_addr = HEAP_START + (i * PAGE_SIZE);
        heap_page_table->entries[i] = create_pte(physical_addr, 
            PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    }
    
    // Записываем PDE для кучи
    uint32_t heap_page_table_addr = (uint32_t)heap_page_table;
    page_directory->entries[heap_directory_index] = create_pde(heap_page_table_addr, 
        PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
        
    terminal_writestring("Heap mapping created\n");
}

// Включение виртуальной памяти
void enable_virtual_memory(void) {
    if (paging_enabled) {
        terminal_writestring("Paging is already enabled\n");
        return;
    }
    
    terminal_writestring("Enabling paging...\n");
    
    // Включаем пейджинг через ассемблерную функцию
    enable_paging((uint32_t)page_directory);
    paging_enabled = 1;
    
    terminal_writestring("Virtual memory enabled successfully!\n");
}

// Отключение виртуальной памяти
void disable_virtual_memory(void) {
    if (!paging_enabled) {
        terminal_writestring("Paging is not enabled\n");
        return;
    }
    
    terminal_writestring("Disabling paging...\n");
    disable_paging();
    paging_enabled = 0;
    terminal_writestring("Virtual memory disabled\n");
}

// Обработчик Page Fault
void handle_page_fault(void) {
    uint32_t fault_address = get_page_fault_address();
    
    terminal_writestring("PAGE FAULT occurred!\n");
    terminal_writestring("Fault address: 0x");
    
    // Выводим адрес в hex формате
    for (int i = 28; i >= 0; i -= 4) {
        uint32_t nibble = (fault_address >> i) & 0xF;
        char hex_char = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        terminal_putchar(hex_char);
    }
    terminal_putchar('\n');
    
    terminal_writestring("System halted due to page fault.\n");
    while(1) asm volatile("hlt");
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

// Вывод чисел
void print_number(uint32_t num) {
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    
    char buffer[12]; // Достаточно для 32-bit числа
    int i = 0;
    
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Выводим цифры в обратном порядке
    while (--i >= 0) {
        terminal_putchar(buffer[i]);
    }
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
    terminal_writestring("MyOS Shell v1.1 with Memory Management\n");
    terminal_writestring("=====================================\n\n");
}

void command_help(void) {
    terminal_writestring("Available commands:\n");
    terminal_writestring("  help     - Show this help message\n");
    terminal_writestring("  clear    - Clear the screen\n");
    terminal_writestring("  about    - Show system information\n");
    terminal_writestring("  memory   - Show memory usage\n");
    terminal_writestring("  memtest  - Test memory allocation\n");
    terminal_writestring("  vmem     - Show virtual memory status\n");
    terminal_writestring("  paging   - Enable/disable paging\n");
    terminal_writestring("  reboot   - Restart the system\n");
    terminal_writestring("  poweroff - Shutdown the system\n");
    terminal_writestring("\n");
}

void command_about(void) {
    terminal_writestring("MyOS - Simple Operating System\n");
    terminal_writestring("Version: 0.4 (with Virtual Memory)\n");
    terminal_writestring("Features: Keyboard, Shell, Memory & Virtual Memory\n");
    terminal_writestring("Written in: C and Assembly\n");
    terminal_writestring("Written by: Kavoshnik\n");
    terminal_writestring("\n");
}

void command_memory(void) {
    uint32_t total, free, used;
    get_memory_info(&total, &free, &used);
    
    terminal_writestring("Memory Usage:\n");
    terminal_writestring("  Total: ");
    print_number(total);
    terminal_writestring(" bytes\n");
    terminal_writestring("  Used:  ");
    print_number(used);
    terminal_writestring(" bytes\n");
    terminal_writestring("  Free:  ");
    print_number(free);
    terminal_writestring(" bytes\n\n");
}

void command_memtest(void) {
    terminal_writestring("Memory allocation test:\n");
    
    // Тестируем выделение и освобождение памяти
    void* ptr1 = kmalloc(100);
    void* ptr2 = kmalloc(200);
    void* ptr3 = kmalloc(50);
    
    terminal_writestring("Allocated 3 blocks (100, 200, 50 bytes)\n");
    
    if (ptr1) terminal_writestring("Block 1: OK\n");
    else terminal_writestring("Block 1: FAILED\n");
    
    if (ptr2) terminal_writestring("Block 2: OK\n");
    else terminal_writestring("Block 2: FAILED\n");
    
    if (ptr3) terminal_writestring("Block 3: OK\n");
    else terminal_writestring("Block 3: FAILED\n");
    
    // Записываем данные в блоки
    if (ptr1) {
        char* data1 = (char*)ptr1;
        for (int i = 0; i < 10; i++) {
            data1[i] = 'A' + i;
        }
        terminal_writestring("Data written to block 1\n");
    }
    
    // Освобождаем память
    kfree(ptr2);
    terminal_writestring("Block 2 freed\n");
    
    kfree(ptr1);
    kfree(ptr3);
    terminal_writestring("All blocks freed\n");
    
    terminal_writestring("Memory test completed!\n\n");
}

void command_reboot(void) {
    terminal_writestring("Rebooting system...\n");
    // Простая перезагрузка через клавиатурный контроллер
    outb(0x64, 0xFE);
    while(1) asm volatile("hlt");
}

void command_poweroff(void) {
    terminal_writestring("Shutting down system...\n");
    
    // Попытка ACPI shutdown через порт 0x604 (QEMU)
    outb(0x604, 0x20);
    outb(0x605, 0x00);
    
    // Альтернативный метод для QEMU - порт 0xB004
    outb(0xB004, 0x20);
    outb(0xB005, 0x00);
    
    // Если ничего не помогло, просто останавливаем систему
    terminal_writestring("System halted. You can close the emulator.\n");
    asm volatile("cli");
    while(1) asm volatile("hlt");
}

// Вспомогательная функция для вывода чисел в hex формате
void print_number_hex(uint32_t num) {
    for (int i = 28; i >= 0; i -= 4) {
        uint32_t nibble = (num >> i) & 0xF;
        char hex_char = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        terminal_putchar(hex_char);
    }
}

void command_vmem(void) {
    terminal_writestring("Virtual Memory Status:\n");
    terminal_writestring("  Paging: ");
    if (paging_enabled) {
        terminal_writestring("ENABLED\n");
    } else {
        terminal_writestring("DISABLED\n");
    }
    
    terminal_writestring("  Page Directory: 0x");
    print_number_hex((uint32_t)page_directory);
    terminal_writestring("\n");
    
    terminal_writestring("  Page Tables: 0x");
    print_number_hex((uint32_t)page_tables);
    terminal_writestring("\n");
    
    terminal_writestring("  Page Size: ");
    print_number(PAGE_SIZE);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("  Entries per table: ");
    print_number(PAGE_ENTRIES);
    terminal_writestring("\n\n");
}

void command_paging(void) {
    if (paging_enabled) {
        terminal_writestring("Paging is currently ENABLED\n");
        terminal_writestring("Virtual memory is active and working!\n");
    } else {
        terminal_writestring("Paging is currently DISABLED\n");
        terminal_writestring("Initializing and enabling virtual memory...\n");
        
        init_virtual_memory();
        enable_virtual_memory();
        
        terminal_writestring("Virtual memory is now active!\n");
    }
    terminal_writestring("\n");
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
    } else if (strcmp(command, "memory") == 0) {
        command_memory();
    } else if (strcmp(command, "memtest") == 0) {
        command_memtest();
    } else if (strcmp(command, "vmem") == 0) {
        command_vmem();
    } else if (strcmp(command, "paging") == 0) {
        command_paging();
    } else if (strcmp(command, "reboot") == 0) {
        command_reboot();
    } else if (strcmp(command, "poweroff") == 0) {
        command_poweroff();
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
    terminal_writestring("MyOS v0.3 with Memory Management!\n");
    terminal_writestring("==================================\n\n");

    // Инициализация управления памятью
    init_memory_management();
    terminal_writestring("Memory management initialized\n");
    
    // Инициализация виртуальной памяти
    terminal_writestring("Initializing virtual memory system...\n");
    init_virtual_memory();
    enable_virtual_memory();
    terminal_writestring("Virtual memory system ready\n");

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
    
    // Устанавливаем специальный обработчик Page Fault (исключение 14)
    idt_set_gate(14, (uint32_t)page_fault_handler, 0x08, 0x8E);

    // Устанавливаем обработчик клавиатуры (IRQ1 = прерывание 33)
    idt_set_gate(33, (uint32_t)irq1_handler, 0x08, 0x8E);
    idt_flush();

    // ПОЛНАЯ инициализация системы для работы с GRUB
    initialize_system();

    // Включаем прерывания
    asm volatile("sti");

    // Инициализация шелла
    command_clear();
    terminal_writestring("Welcome to MyOS with Virtual Memory!\n");
    terminal_writestring("Type 'help' for available commands.\n");
    
    shell_ready = 1;
    shell_prompt();

    while (1) asm volatile("hlt");
}