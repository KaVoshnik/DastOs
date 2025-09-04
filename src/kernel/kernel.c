typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned int   size_t;

#define NULL ((void*)0)

// Include модулей
#include "../include/types.h"
#include "../include/elf.h"
#include "../include/keyboard.h"

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

// Файловая система
#define FS_MAX_FILES        64          // Максимум файлов
#define FS_MAX_FILENAME     32          // Максимум символов в имени файла
#define FS_MAX_FILESIZE     1024        // Максимум байт в файле
#define FS_BLOCK_SIZE       64          // Размер блока данных
#define FS_MAX_BLOCKS       256         // Максимум блоков данных

#define FS_INODE_FREE       0           // Свободный inode
#define FS_INODE_FILE       1           // Обычный файл
#define FS_INODE_DIR        2           // Директория (пока не используется)

// Планировщик задач
#define MAX_TASKS           8           // Максимум задач
#define TASK_STACK_SIZE     4096        // Размер стека для каждой задачи

#define TASK_STATE_RUNNING  0           // Выполняется
#define TASK_STATE_READY    1           // Готова к выполнению
#define TASK_STATE_BLOCKED  2           // Заблокирована
#define TASK_STATE_DEAD     3           // Завершена

// VGA курсор
#define VGA_CURSOR_COMMAND_PORT 0x3D4
#define VGA_CURSOR_DATA_PORT 0x3D5

// Клавиатура
#define KEY_LSHIFT_PRESSED   0x2A
#define KEY_LSHIFT_RELEASED  0xAA
#define KEY_RSHIFT_PRESSED   0x36
#define KEY_RSHIFT_RELEASED  0xB6
#define KEY_CTRL_PRESSED     0x1D
#define KEY_CTRL_RELEASED    0x9D
#define KEY_ALT_PRESSED      0x38
#define KEY_ALT_RELEASED     0xB8
#define KEY_CAPS_LOCK        0x3A

// Шелл
#define COMMAND_BUFFER_SIZE 256

// Системные вызовы
#define SYS_EXIT    0
#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_OPEN    3
#define SYS_CLOSE   4
#define SYS_FORK    5
#define SYS_EXEC    6
#define SYS_WAIT    7
#define SYS_GETPID  8
#define SYS_YIELD   9
#define SYS_SLEEP   10

// Таймер (PIT - Programmable Interval Timer)
#define PIT_FREQUENCY   1193182
#define TIMER_FREQUENCY 100          // 100 Hz = 10ms тики
#define PIT_COMMAND     0x43
#define PIT_DATA0       0x40

// GDT (Global Descriptor Table)
#define GDT_ENTRIES     8
#define GDT_NULL        0x00         // Нулевой дескриптор
#define GDT_KERNEL_CODE 0x08         // Дескриптор кода ядра (Ring 0)
#define GDT_KERNEL_DATA 0x10         // Дескриптор данных ядра (Ring 0)
#define GDT_USER_CODE   0x1B         // Дескриптор кода пользователя (Ring 3)
#define GDT_USER_DATA   0x23         // Дескриптор данных пользователя (Ring 3)

// ELF загрузчик
#define ELF_LOAD_BASE   0x8000000    // Базовый адрес загрузки ELF программ (128MB)

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

// GDT структуры
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
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

// === СТРУКТУРЫ ФАЙЛОВОЙ СИСТЕМЫ ===

// Суперблок файловой системы
typedef struct {
    uint32_t magic;              // Магическое число для проверки
    uint32_t total_inodes;       // Общее количество inodes
    uint32_t free_inodes;        // Свободные inodes
    uint32_t total_blocks;       // Общее количество блоков данных
    uint32_t free_blocks;        // Свободные блоки
    uint32_t block_size;         // Размер блока данных
} fs_superblock_t;

// Индексный узел (inode) файла
typedef struct {
    uint8_t type;                        // Тип: свободный, файл, директория
    char filename[FS_MAX_FILENAME];      // Имя файла
    uint32_t size;                       // Размер файла в байтах
    uint32_t blocks[16];                 // Прямые указатели на блоки данных
    uint32_t created_time;               // Время создания (упрощенно)
    uint32_t modified_time;              // Время модификации
} fs_inode_t;

// Запись директории (пока не используется, но структура готова)
typedef struct {
    uint32_t inode_number;               // Номер inode
    char name[FS_MAX_FILENAME];          // Имя файла
} fs_dir_entry_t;

// Глобальное состояние файловой системы
typedef struct {
    fs_superblock_t superblock;          // Суперблок
    fs_inode_t inodes[FS_MAX_FILES];     // Таблица inodes
    uint8_t* data_blocks;                // Указатель на область данных
    uint8_t inode_bitmap[FS_MAX_FILES];  // Битовая карта занятых inodes
    uint8_t block_bitmap[FS_MAX_BLOCKS]; // Битовая карта занятых блоков
    int initialized;                     // Флаг инициализации
} fs_state_t;

// === СТРУКТУРЫ ПЛАНИРОВЩИКА ЗАДАЧ ===

// Состояние регистров для переключения контекста
typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, esp, ebp;
    uint32_t eip, eflags;
    uint16_t cs, ds, es, fs, gs, ss;
} registers_t;

// ELF структуры теперь определены в elf.h

// Структура задачи
typedef struct task {
    uint32_t id;                         // ID задачи
    char name[32];                       // Имя задачи
    uint32_t state;                      // Состояние задачи
    uint32_t priority;                   // Приоритет (0-высший, 255-низший)
    registers_t regs;                    // Сохраненные регистры
    uint32_t* stack;                     // Стек задачи
    uint32_t stack_size;                 // Размер стека
    uint32_t time_slice;                 // Оставшееся время выполнения
    elf_loader_t* elf_loader;            // ELF-загрузчик для этой задачи
    struct task* next;                   // Следующая задача в списке
} task_t;

// Глобальные переменные
struct idt_entry idt[256];
struct idt_ptr idtp;
struct gdt_entry gdt[GDT_ENTRIES];
struct gdt_ptr gdtp;
int terminal_row = 0, terminal_column = 0;

// Переменные таймера и планировщика
uint32_t timer_ticks = 0;
uint32_t timer_frequency = TIMER_FREQUENCY;

// Переменные управления памятью
memory_block_t* heap_start = NULL;
uint32_t total_memory = 0;
uint32_t free_memory = 0;
uint32_t used_memory = 0;

// Переменные виртуальной памяти
page_directory_t* page_directory = (page_directory_t*)PAGE_DIRECTORY_ADDR;
page_table_t* page_tables = (page_table_t*)PAGE_TABLES_ADDR;
int paging_enabled = 0;

// Переменные файловой системы
fs_state_t filesystem;
uint32_t fs_time_counter = 0; // Простой счетчик времени

// Переменные планировщика
task_t* current_task = NULL;
task_t* task_list = NULL;
uint32_t next_task_id = 1;
uint32_t scheduler_ticks = 0;

// Переменные шелла
char command_buffer[COMMAND_BUFFER_SIZE];
int command_length = 0;
int shell_ready = 0;

// Переменные шелла для обработки ввода
void shell_keyboard_callback(keyboard_event_t* event);

// Предварительные объявления всех функций
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void print_number(uint32_t num);
void terminal_clear(void);
void shell_prompt(void);
void execute_command(const char* command);

// Объявления функций файловой системы
void init_filesystem(void);
int fs_create_file(const char* filename);
int fs_delete_file(const char* filename);
int fs_write_file(const char* filename, const char* data, uint32_t size);
int fs_read_file(const char* filename, char* buffer, uint32_t max_size);
void fs_list_files(void);
int fs_file_exists(const char* filename);
fs_inode_t* fs_find_inode(const char* filename);

// Объявления функций планировщика
void init_scheduler(void);
task_t* create_task(const char* name, void (*entry_point)(void), uint32_t priority);
void schedule(void);
void task_yield(void);
void switch_to_task(task_t* task);

// Объявления функций ELF-загрузчика теперь в elf.h
// Локальные функции
task_t* create_elf_task(const char* name, uint8_t* elf_data, uint32_t elf_size, uint32_t priority);
int load_elf_from_file(const char* filename, uint8_t** elf_data, uint32_t* elf_size);
void cleanup_elf_task(task_t* task);

// Функции для работы с виртуальной памятью (из interrupts.asm)
extern void enable_paging(uint32_t page_directory_addr);
extern void disable_paging(void);
extern uint32_t get_page_fault_address(void);
extern void flush_tlb(void);
extern void page_fault_handler(void);
extern void irq1_handler(void);
extern void timer_handler(void);
extern void syscall_handler(void);
extern void exception_handler(void);
extern void idt_flush(void);

// Функции переключения контекста (из context.asm)
extern void save_context(task_t* task);
extern void restore_context(task_t* task);
extern void switch_context(task_t* current, task_t* next);
extern void init_task_context(task_t* task, void* entry_point, void* stack_top);
extern void task_switch(task_t* next_task);

// Функции пользовательского режима (из usermode.asm)
extern void switch_to_user_mode(uint32_t user_esp, uint32_t user_eip);
extern void switch_to_kernel_mode(void);
extern void create_user_task(uint32_t entry_point, uint32_t stack_top, task_t* task);
extern int get_current_privilege_level(void);
extern int is_user_mode(void);
extern void setup_user_mode_gdt(void);

// Функции системных вызовов (из syscalls.asm)
extern int syscall0(int syscall_num);
extern int syscall1(int syscall_num, int arg0);
extern int syscall2(int syscall_num, int arg0, int arg1);
extern int syscall3(int syscall_num, int arg0, int arg1, int arg2);
extern int syscall4(int syscall_num, int arg0, int arg1, int arg2, int arg3);
extern int syscall5(int syscall_num, int arg0, int arg1, int arg2, int arg3, int arg4);

// Callback для обработки клавиатуры в шелле
void shell_keyboard_callback(keyboard_event_t* event) {
    if (!event->pressed || !event->character) {
        return; // Обрабатываем только нажатия с символами
    }
    
    char c = event->character;
    
    if (c == '\n') {
        // Enter - выполняем команду
        command_buffer[command_length] = '\0';
        terminal_putchar('\n');
        execute_command(command_buffer);
        command_length = 0;
        shell_prompt();
    } else if (c == '\b') {
        // Backspace
        if (command_length > 0) {
            command_length--;
            terminal_putchar('\b');
        }
    } else if (command_length < COMMAND_BUFFER_SIZE - 1) {
        // Обычный символ
        command_buffer[command_length++] = c;
        terminal_putchar(c);
    }
}

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

// Функции для работы со строками
int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

void strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

int strncmp(const char* str1, const char* str2, size_t n) {
    while (n-- && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return (n == 0) ? 0 : (*str1 - *str2);
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}

// === ФУНКЦИИ ТЕРМИНАЛА ===

// Функции для управления VGA курсором
void update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    
    outb(VGA_CURSOR_COMMAND_PORT, 0x0F);
    outb(VGA_CURSOR_DATA_PORT, (uint8_t)(pos & 0xFF));
    outb(VGA_CURSOR_COMMAND_PORT, 0x0E);
    outb(VGA_CURSOR_DATA_PORT, (uint8_t)((pos >> 8) & 0xFF));
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(VGA_CURSOR_COMMAND_PORT, 0x0A);
    outb(VGA_CURSOR_DATA_PORT, (inb(VGA_CURSOR_DATA_PORT) & 0xC0) | cursor_start);
    
    outb(VGA_CURSOR_COMMAND_PORT, 0x0B);
    outb(VGA_CURSOR_DATA_PORT, (inb(VGA_CURSOR_DATA_PORT) & 0xE0) | cursor_end);
}

void disable_cursor() {
    outb(VGA_CURSOR_COMMAND_PORT, 0x0A);
    outb(VGA_CURSOR_DATA_PORT, 0x20);
}

void terminal_clear(void) {
    uint16_t* video_memory = (uint16_t*)VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_memory[i] = 0x0720; // Пробел с белым текстом на черном фоне
    }
    terminal_row = 0;
    terminal_column = 0;
    update_cursor(terminal_column, terminal_row);
}

void terminal_putchar(char c) {
    uint16_t* video_memory = (uint16_t*)VGA_MEMORY;
    
    if (c == '\n') {
        terminal_row++;
        terminal_column = 0;
    } else if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            video_memory[terminal_row * VGA_WIDTH + terminal_column] = 0x0720;
        }
        update_cursor(terminal_column, terminal_row);
        return;
    } else {
        video_memory[terminal_row * VGA_WIDTH + terminal_column] = (0x07 << 8) | c;
        terminal_column++;
    }
    
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }
    
    if (terminal_row >= VGA_HEIGHT) {
        // Прокрутка экрана вверх
        for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            video_memory[i] = video_memory[i + VGA_WIDTH];
        }
        for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
            video_memory[i] = 0x0720;
        }
        terminal_row = VGA_HEIGHT - 1;
    }
    
    update_cursor(terminal_column, terminal_row);
}

void terminal_writestring(const char* data) {
    while (*data) {
        terminal_putchar(*data++);
    }
}

void print_number(uint32_t num) {
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    
    char buffer[12];
    int i = 0;
    
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}

void print_hex(uint32_t num) {
    terminal_writestring("0x");
    
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    
    char buffer[9];
    int i = 0;
    
    while (num > 0) {
        int digit = num % 16;
        buffer[i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        num /= 16;
    }
    
    while (--i >= 0) {
        terminal_putchar(buffer[i]);
    }
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

// === ELF ЗАГРУЗЧИК ===

// ELF магические числа и константы
#define ELFMAG0         0x7f
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'
#define ELFCLASS32      1
#define ELFDATA2LSB     1
#define EV_CURRENT      1
#define ET_EXEC         2
#define EM_386          3
#define PT_LOAD         1

// Validate ELF file format
int elf_validate(uint8_t* data, uint32_t size) {
    if (!data || size < sizeof(elf_header_t)) {
        return 0; // Invalid: NULL data or too small
    }
    
    elf_header_t* header = (elf_header_t*)data;
    
    // Check ELF magic number
    if (header->e_ident[0] != ELFMAG0 ||
        header->e_ident[1] != ELFMAG1 ||
        header->e_ident[2] != ELFMAG2 ||
        header->e_ident[3] != ELFMAG3) {
        return 0; // Invalid magic number
    }
    
    // Check class (32-bit)
    if (header->e_ident[4] != ELFCLASS32) {
        return 0; // Only support 32-bit ELF
    }
    
    // Check data encoding (little endian)
    if (header->e_ident[5] != ELFDATA2LSB) {
        return 0; // Only support little endian
    }
    
    // Check ELF version
    if (header->e_ident[6] != EV_CURRENT) {
        return 0; // Invalid version
    }
    
    // Check machine type (i386)
    if (header->e_machine != EM_386) {
        return 0; // Only support i386
    }
    
    // Check file type (executable)
    if (header->e_type != ET_EXEC) {
        return 0; // Only support executable files
    }
    
    // Check program header table
    if (header->e_phoff == 0 || header->e_phnum == 0) {
        return 0; // No program headers
    }
    
    // Validate program header bounds
    uint32_t ph_end = header->e_phoff + (header->e_phnum * header->e_phentsize);
    if (ph_end > size) {
        return 0; // Program headers exceed file size
    }
    
    return 1; // Valid ELF file
}

// Parse ELF file and initialize loader context
int elf_parse(elf_loader_t* loader, uint8_t* data, uint32_t size) {
    if (!loader || !data || size == 0) {
        return 0;
    }
    
    // Clear loader structure
    memset(loader, 0, sizeof(elf_loader_t));
    
    // Validate ELF file
    if (!elf_validate(data, size)) {
        return 0;
    }
    
    // Initialize loader context
    loader->data = data;
    loader->size = size;
    loader->header = (elf_header_t*)data;
    loader->valid = 1;
    
    // Set up program headers
    if (loader->header->e_phoff > 0 && loader->header->e_phnum > 0) {
        loader->pheaders = (elf_program_header_t*)(data + loader->header->e_phoff);
    }
    
    // Set entry point
    loader->entry_point = loader->header->e_entry;
    
    return 1;
}

// Load ELF program into memory
uint32_t elf_load_program(elf_loader_t* loader) {
    if (!loader || !loader->valid) {
        return 0;
    }
    
    uint32_t min_addr = 0xFFFFFFFF;
    uint32_t max_addr = 0;
    
    // First pass: find memory range needed
    for (int i = 0; i < loader->header->e_phnum; i++) {
        elf_program_header_t* ph = &loader->pheaders[i];
        
        if (ph->p_type == PT_LOAD) {
            if (ph->p_vaddr < min_addr) {
                min_addr = ph->p_vaddr;
            }
            if (ph->p_vaddr + ph->p_memsz > max_addr) {
                max_addr = ph->p_vaddr + ph->p_memsz;
            }
        }
    }
    
    if (min_addr == 0xFFFFFFFF) {
        return 0; // No loadable segments
    }
    
    // Use fixed load address
    uint32_t load_base = ELF_LOAD_BASE;
    loader->load_base = load_base;
    
    // Second pass: actually load segments
    for (int i = 0; i < loader->header->e_phnum; i++) {
        elf_program_header_t* ph = &loader->pheaders[i];
        
        if (ph->p_type == PT_LOAD) {
            // Calculate load address
            uint32_t load_addr = load_base + (ph->p_vaddr - min_addr);
            
            // Copy file data to memory
            if (ph->p_filesz > 0) {
                memcpy((void*)load_addr, 
                      loader->data + ph->p_offset, 
                      ph->p_filesz);
            }
            
            // Zero out any remaining memory (BSS section)
            if (ph->p_memsz > ph->p_filesz) {
                memset((void*)(load_addr + ph->p_filesz), 0, 
                       ph->p_memsz - ph->p_filesz);
            }
        }
    }
    
    // Adjust entry point to loaded address
    loader->entry_point = load_base + (loader->header->e_entry - min_addr);
    
    return loader->entry_point;
}

// Get entry point address
int elf_get_entry_point(elf_loader_t* loader, uint32_t* entry) {
    if (!loader || !loader->valid || !entry) {
        return 0;
    }
    
    *entry = loader->entry_point;
    return 1;
}

// Clean up loader resources
void elf_cleanup(elf_loader_t* loader) {
    if (!loader) {
        return;
    }
    
    // Clear the structure
    memset(loader, 0, sizeof(elf_loader_t));
}

// Создание задачи из ELF файла
task_t* create_elf_task(const char* name, uint8_t* elf_data, uint32_t elf_size, uint32_t priority) {
    if (!name || !elf_data || elf_size == 0) {
        return NULL;
    }
    
    // Создаем новую задачу
    task_t* task = (task_t*)kmalloc(sizeof(task_t));
    if (!task) {
        terminal_writestring("Error: Failed to allocate memory for task\n");
        return NULL;
    }
    
    // Инициализируем задачу
    memset(task, 0, sizeof(task_t));
    task->id = next_task_id++;
    strncpy(task->name, name, 31);
    task->name[31] = '\0';
    task->state = TASK_STATE_READY;
    task->priority = priority;
    task->time_slice = 10; // 10 тиков таймера
    
    // Создаем стек для задачи
    task->stack = (uint32_t*)kmalloc(TASK_STACK_SIZE);
    if (!task->stack) {
        terminal_writestring("Error: Failed to allocate stack for task\n");
        kfree(task);
        return NULL;
    }
    task->stack_size = TASK_STACK_SIZE;
    
    // Создаем ELF загрузчик
    task->elf_loader = (elf_loader_t*)kmalloc(sizeof(elf_loader_t));
    if (!task->elf_loader) {
        terminal_writestring("Error: Failed to allocate ELF loader\n");
        kfree(task->stack);
        kfree(task);
        return NULL;
    }
    
    // Парсим ELF файл
    if (!elf_parse(task->elf_loader, elf_data, elf_size)) {
        terminal_writestring("Error: Failed to parse ELF file\n");
        kfree(task->elf_loader);
        kfree(task->stack);
        kfree(task);
        return NULL;
    }
    
    // Загружаем программу в память
    uint32_t entry_point = elf_load_program(task->elf_loader);
    if (entry_point == 0) {
        terminal_writestring("Error: Failed to load ELF program\n");
        elf_cleanup(task->elf_loader);
        kfree(task->elf_loader);
        kfree(task->stack);
        kfree(task);
        return NULL;
    }
    
    // Настраиваем контекст для пользовательского режима
    uint32_t stack_top = (uint32_t)task->stack + task->stack_size - 4;
    create_user_task(entry_point, stack_top, task);
    
    // Добавляем в список задач
    task->next = task_list;
    task_list = task;
    
    terminal_writestring("ELF task created: ");
    terminal_writestring(name);
    terminal_writestring(" (Entry: ");
    print_hex(entry_point);
    terminal_writestring(")\n");
    
    return task;
}

// Загрузка ELF файла из файловой системы
int load_elf_from_file(const char* filename, uint8_t** elf_data, uint32_t* elf_size) {
    if (!filename || !elf_data || !elf_size) {
        return 0;
    }
    
    // Проверяем существование файла
    if (!fs_file_exists(filename)) {
        terminal_writestring("Error: ELF file not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return 0;
    }
    
    fs_inode_t* inode = fs_find_inode(filename);
    if (!inode) {
        return 0;
    }
    
    *elf_size = inode->size;
    *elf_data = (uint8_t*)kmalloc(*elf_size);
    if (!*elf_data) {
        terminal_writestring("Error: Failed to allocate memory for ELF file\n");
        return 0;
    }
    
    // Читаем данные файла
    if (fs_read_file(filename, (char*)*elf_data, *elf_size) <= 0) {
        terminal_writestring("Error: Failed to read ELF file\n");
        kfree(*elf_data);
        *elf_data = NULL;
        *elf_size = 0;
        return 0;
    }
    
    return 1;
}

// Очистка ресурсов ELF задачи
void cleanup_elf_task(task_t* task) {
    if (!task) {
        return;
    }
    
    if (task->elf_loader) {
        elf_cleanup(task->elf_loader);
        kfree(task->elf_loader);
    }
    
    if (task->stack) {
        kfree(task->stack);
    }
    
    kfree(task);
}

// === ТЕСТОВАЯ ELF ПРОГРАММА ===

// Простая ELF программа "Hello, World!" для тестирования
uint8_t test_elf_program[] = {
    // ELF header
    0x7f, 0x45, 0x4c, 0x46,  // Magic: 0x7f, 'E', 'L', 'F'
    0x01,                     // Class: ELFCLASS32
    0x01,                     // Data: ELFDATA2LSB
    0x01,                     // Version: EV_CURRENT
    0x00,                     // ABI: SYSV
    0x00, 0x00, 0x00, 0x00,   // ABI version and padding
    0x00, 0x00, 0x00, 0x00,   // More padding
    
    0x02, 0x00,               // Type: ET_EXEC
    0x03, 0x00,               // Machine: EM_386
    0x01, 0x00, 0x00, 0x00,   // Version: 1
    0x54, 0x80, 0x04, 0x08,   // Entry point: 0x08048054
    0x34, 0x00, 0x00, 0x00,   // Program header offset: 52
    0x00, 0x00, 0x00, 0x00,   // Section header offset: 0
    0x00, 0x00, 0x00, 0x00,   // Flags: 0
    0x34, 0x00,               // ELF header size: 52
    0x20, 0x00,               // Program header size: 32
    0x01, 0x00,               // Program header count: 1
    0x00, 0x00,               // Section header size: 0
    0x00, 0x00,               // Section header count: 0
    0x00, 0x00,               // String table index: 0
    
    // Program header
    0x01, 0x00, 0x00, 0x00,   // Type: PT_LOAD
    0x00, 0x00, 0x00, 0x00,   // Offset: 0
    0x00, 0x80, 0x04, 0x08,   // Virtual address: 0x08048000
    0x00, 0x80, 0x04, 0x08,   // Physical address: 0x08048000
    0x74, 0x00, 0x00, 0x00,   // File size: 116
    0x74, 0x00, 0x00, 0x00,   // Memory size: 116
    0x05, 0x00, 0x00, 0x00,   // Flags: PF_R | PF_X
    0x00, 0x10, 0x00, 0x00,   // Alignment: 4096
    
    // Code section (simplified)
    // mov eax, 1 (sys_exit)
    0xb8, 0x01, 0x00, 0x00, 0x00,
    // mov ebx, 0 (exit code)
    0xbb, 0x00, 0x00, 0x00, 0x00,
    // int 0x80 (syscall)
    0xcd, 0x80,
    // Padding to align
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90
};

uint32_t test_elf_size = sizeof(test_elf_program);

// === ФАЙЛОВАЯ СИСТЕМА ===

void init_filesystem(void) {
    memset(&filesystem, 0, sizeof(fs_state_t));
    
    // Инициализируем суперблок
    filesystem.superblock.magic = 0x12345678;
    filesystem.superblock.total_inodes = FS_MAX_FILES;
    filesystem.superblock.free_inodes = FS_MAX_FILES;
    filesystem.superblock.total_blocks = FS_MAX_BLOCKS;
    filesystem.superblock.free_blocks = FS_MAX_BLOCKS;
    filesystem.superblock.block_size = FS_BLOCK_SIZE;
    
    // Выделяем память для блоков данных
    filesystem.data_blocks = (uint8_t*)kmalloc(FS_MAX_BLOCKS * FS_BLOCK_SIZE);
    if (!filesystem.data_blocks) {
        terminal_writestring("Error: Failed to allocate filesystem data blocks\n");
        return;
    }
    
    memset(filesystem.data_blocks, 0, FS_MAX_BLOCKS * FS_BLOCK_SIZE);
    memset(filesystem.inode_bitmap, 0, FS_MAX_FILES);
    memset(filesystem.block_bitmap, 0, FS_MAX_BLOCKS);
    
    filesystem.initialized = 1;
    
    terminal_writestring("Filesystem initialized\n");
}

int fs_create_file(const char* filename) {
    if (!filesystem.initialized || !filename) return -1;
    
    // Проверяем, не существует ли файл уже
    if (fs_file_exists(filename)) {
        terminal_writestring("File already exists: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return -1;
    }
    
    // Ищем свободный inode
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (filesystem.inode_bitmap[i] == 0) {
            filesystem.inode_bitmap[i] = 1;
            filesystem.superblock.free_inodes--;
            
            fs_inode_t* inode = &filesystem.inodes[i];
            memset(inode, 0, sizeof(fs_inode_t));
            inode->type = FS_INODE_FILE;
            strncpy(inode->filename, filename, FS_MAX_FILENAME - 1);
            inode->created_time = fs_time_counter++;
            inode->modified_time = inode->created_time;
            
            return i;
        }
    }
    
    terminal_writestring("No free inodes available\n");
    return -1; // Нет свободных inodes
}

int fs_file_exists(const char* filename) {
    if (!filesystem.initialized || !filename) return 0;
    
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (filesystem.inode_bitmap[i] && 
            filesystem.inodes[i].type == FS_INODE_FILE &&
            strcmp(filesystem.inodes[i].filename, filename) == 0) {
            return 1;
        }
    }
    
    return 0;
}

fs_inode_t* fs_find_inode(const char* filename) {
    if (!filesystem.initialized || !filename) return NULL;
    
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (filesystem.inode_bitmap[i] && 
            filesystem.inodes[i].type == FS_INODE_FILE &&
            strcmp(filesystem.inodes[i].filename, filename) == 0) {
            return &filesystem.inodes[i];
        }
    }
    
    return NULL;
}

int fs_write_file(const char* filename, const char* data, uint32_t size) {
    if (!filesystem.initialized || !filename || !data || size == 0) return -1;
    
    fs_inode_t* inode = fs_find_inode(filename);
    if (!inode) {
        terminal_writestring("File not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return -1;
    }
    
    if (size > FS_MAX_FILESIZE) {
        size = FS_MAX_FILESIZE;
    }
    
    // Простая реализация: записываем в первый блок
    if (inode->blocks[0] == 0) {
        // Ищем свободный блок
        for (uint32_t i = 0; i < FS_MAX_BLOCKS; i++) {
            if (filesystem.block_bitmap[i] == 0) {
                filesystem.block_bitmap[i] = 1;
                filesystem.superblock.free_blocks--;
                inode->blocks[0] = i;
                break;
            }
        }
    }
    
    if (inode->blocks[0] > 0) {
        uint8_t* block_data = filesystem.data_blocks + (inode->blocks[0] * FS_BLOCK_SIZE);
        memcpy(block_data, data, size);
        inode->size = size;
        inode->modified_time = fs_time_counter++;
        return 0;
    }
    
    terminal_writestring("No free blocks available\n");
    return -1;
}

int fs_read_file(const char* filename, char* buffer, uint32_t max_size) {
    if (!filesystem.initialized || !filename || !buffer) return -1;
    
    fs_inode_t* inode = fs_find_inode(filename);
    if (!inode || inode->size == 0) return -1;
    
    uint32_t read_size = (inode->size < max_size) ? inode->size : max_size;
    
    if (inode->blocks[0] > 0) {
        uint8_t* block_data = filesystem.data_blocks + (inode->blocks[0] * FS_BLOCK_SIZE);
        memcpy(buffer, block_data, read_size);
        return read_size;
    }
    
    return -1;
}

int fs_delete_file(const char* filename) {
    if (!filesystem.initialized || !filename) return -1;
    
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (filesystem.inode_bitmap[i] && 
            filesystem.inodes[i].type == FS_INODE_FILE &&
            strcmp(filesystem.inodes[i].filename, filename) == 0) {
            
            fs_inode_t* inode = &filesystem.inodes[i];
            
            // Освобождаем блоки данных
            for (int j = 0; j < 16 && inode->blocks[j] > 0; j++) {
                filesystem.block_bitmap[inode->blocks[j]] = 0;
                filesystem.superblock.free_blocks++;
            }
            
            // Освобождаем inode
            filesystem.inode_bitmap[i] = 0;
            filesystem.superblock.free_inodes++;
            memset(inode, 0, sizeof(fs_inode_t));
            
            return 0;
        }
    }
    
    terminal_writestring("File not found: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    return -1;
}

void fs_list_files(void) {
    if (!filesystem.initialized) {
        terminal_writestring("ERROR: File system not initialized!\n");
        return;
    }
    
    terminal_writestring("Files in filesystem:\n");
    terminal_writestring("Name               Size      Time\n");
    terminal_writestring("----------------------------------\n");
    
    int file_count = 0;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (filesystem.inodes[i].type == FS_INODE_FILE) {
            fs_inode_t* inode = &filesystem.inodes[i];
            
            // Имя файла
            terminal_writestring(inode->filename);
            
            // Выравнивание
            int name_len = strlen(inode->filename);
            for (int j = name_len; j < 18; j++) {
                terminal_putchar(' ');
            }
            
            // Размер
            print_number(inode->size);
            terminal_writestring(" bytes   ");
            
            // Время
            print_number(inode->modified_time);
            terminal_putchar('\n');
            
            file_count++;
        }
    }
    
    if (file_count == 0) {
        terminal_writestring("No files found.\n");
    } else {
        terminal_writestring("\nTotal files: ");
        print_number(file_count);
        terminal_putchar('\n');
    }
    
    terminal_writestring("Free inodes: ");
    print_number(filesystem.superblock.free_inodes);
    terminal_writestring(", Free blocks: ");
    print_number(filesystem.superblock.free_blocks);
    terminal_writestring("\n\n");
}

// === ПЛАНИРОВЩИК ЗАДАЧ ===

void init_scheduler(void) {
    current_task = NULL;
    task_list = NULL;
    next_task_id = 1;
    scheduler_ticks = 0;
    terminal_writestring("Task scheduler initialized\n");
}

task_t* create_task(const char* name, void (*entry_point)(void), uint32_t priority) {
    task_t* task = (task_t*)kmalloc(sizeof(task_t));
    if (!task) {
        terminal_writestring("ERROR: Failed to allocate memory for task!\n");
        return NULL;
    }
    
    // Выделяем стек для задачи
    task->stack = (uint32_t*)kmalloc(TASK_STACK_SIZE);
    if (!task->stack) {
        kfree(task);
        terminal_writestring("ERROR: Failed to allocate stack for task!\n");
        return NULL;
    }
    
    // Инициализируем структуру задачи
    task->id = next_task_id++;
    strcpy(task->name, name);
    task->state = TASK_STATE_READY;
    task->priority = priority;
    task->stack_size = TASK_STACK_SIZE;
    task->time_slice = 10; // 10 тиков
    task->elf_loader = NULL;
    task->next = NULL;
    
    // Инициализируем регистры
    memset(&task->regs, 0, sizeof(registers_t));
    task->regs.eip = (uint32_t)entry_point;
    task->regs.esp = (uint32_t)task->stack + TASK_STACK_SIZE - 4; // Вершина стека
    task->regs.cs = 0x08;  // Селектор кода ядра
    task->regs.ds = 0x10;  // Селектор данных ядра
    task->regs.es = 0x10;
    task->regs.fs = 0x10;
    task->regs.gs = 0x10;
    task->regs.ss = 0x10;
    task->regs.eflags = 0x202; // Прерывания включены
    
    // Добавляем задачу в список
    if (task_list == NULL) {
        task_list = task;
        current_task = task;
    } else {
        task_t* last = task_list;
        while (last->next) {
            last = last->next;
        }
        last->next = task;
    }
    
    return task;
}

void schedule(void) {
    if (!current_task || !current_task->next) {
        return;
    }
    
    // Находим следующую готовую задачу
    task_t* next_task = current_task->next;
    while (next_task && next_task->state != TASK_STATE_READY) {
        next_task = next_task->next;
        if (!next_task) {
            next_task = task_list; // Возвращаемся к началу списка
            break;
        }
    }
    
    if (next_task && next_task != current_task) {
        current_task = next_task;
        switch_to_task(current_task);
    }
}

void switch_to_task(task_t* task) {
    if (!task) return;
    
    // В реальной ОС здесь было бы переключение контекста
    // Переключение происходит без вывода сообщений
}

void task_yield(void) {
    if (current_task) {
        current_task->time_slice = 0; // Принудительно вызываем переключение
        schedule();
    }
}

// === ДЕМОНСТРАЦИОННЫЕ ЗАДАЧИ ===

void idle_task(void) {
    while (1) {
        asm volatile("hlt"); // Ждем прерывания
    }
}

void demo_task1(void) {
    for (int i = 0; i < 5; i++) {
        terminal_writestring("Task 1 running... ");
        print_number(i);
        terminal_putchar('\n');
        
        // Имитация работы
        for (volatile int j = 0; j < 1000000; j++);
    }
}

void demo_task2(void) {
    for (int i = 0; i < 3; i++) {
        terminal_writestring("Task 2 executing... ");
        print_number(i);
        terminal_putchar('\n');
        
        // Имитация работы
        for (volatile int j = 0; j < 500000; j++);
    }
}

// === ОБРАБОТЧИКИ ПРЕРЫВАНИЙ ===

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void handle_exception(void) {
    // Минимальная обработка исключений - просто продолжаем
}

void handle_page_fault(void) {
    uint32_t fault_addr = get_page_fault_address();
    terminal_writestring("Page fault at address: ");
    print_hex(fault_addr);
    terminal_writestring("\n");
}

// Старая функция keyboard_handler заменена на новый модуль клавиатуры
// Теперь используется keyboard_handler из keyboard.c

void timer_interrupt_handler(void) {
    timer_ticks++;
    
    // Планировщик задач каждые 10 тиков (100ms при 100Hz)
    if (timer_ticks % 10 == 0) {
        scheduler_ticks++;
        if (current_task && current_task->time_slice > 0) {
            current_task->time_slice--;
            if (current_task->time_slice == 0) {
                current_task->time_slice = 10; // Сбрасываем time slice
                schedule(); // Переключаем задачу
            }
        }
    }
    
    // Отправляем EOI
    outb(PIC1_COMMAND, PIC_EOI);
}

// === СИСТЕМНЫЕ ВЫЗОВЫ ===

int handle_syscall(int syscall_num, int arg0, int arg1, int arg2, int arg3 __attribute__((unused)), int arg4 __attribute__((unused))) {
    switch (syscall_num) {
        case SYS_EXIT:
            // Завершение текущей задачи
            if (current_task) {
                current_task->state = TASK_STATE_DEAD;
                schedule(); // Переключаемся на другую задачу
            }
            return 0;
            
        case SYS_WRITE:
            // Простой вывод (arg0 = fd, arg1 = buffer, arg2 = count)
            if (arg0 == 1) { // stdout
                char* buffer = (char*)arg1;
                for (int i = 0; i < arg2; i++) {
                    terminal_putchar(buffer[i]);
                }
                return arg2;
            }
            return -1;
            
        case SYS_GETPID:
            return current_task ? current_task->id : 0;
            
        case SYS_YIELD:
            schedule();
            return 0;
            
        default:
            return -1; // Неизвестный системный вызов
    }
}

// === КОМАНДЫ ШЕЛЛА ===

void shell_prompt(void) {
    terminal_writestring("myos> ");
}

void command_help(void) {
    terminal_writestring("Available commands:\n");
    terminal_writestring("  help       - Show this help\n");
    terminal_writestring("  clear      - Clear screen\n");
    terminal_writestring("  about      - System information\n");
    terminal_writestring("  memory     - Memory usage\n");
    terminal_writestring("  memtest    - Test memory allocator\n");
    terminal_writestring("  keyboard   - Keyboard status\n");
    terminal_writestring("  tasks      - List tasks\n");
    terminal_writestring("  schedule   - Trigger scheduler\n");
    terminal_writestring("  ls         - List files\n");
    terminal_writestring("  touch <f>  - Create file\n");
    terminal_writestring("  cat <f>    - Show file content\n");
    terminal_writestring("  rm <f>     - Delete file\n");
    terminal_writestring("  echo <t> > <f> - Write text to file\n");
    terminal_writestring("  testelf    - Test ELF loader\n");
    terminal_writestring("  run <f>    - Run ELF program from file\n");
    terminal_writestring("  reboot     - Restart system\n");
    terminal_writestring("  poweroff   - Shutdown system\n");
    terminal_writestring("\nELF Loader Commands:\n");
    terminal_writestring("  testelf    - Test built-in ELF program\n");
    terminal_writestring("  run <file> - Load and execute ELF file\n");
    terminal_writestring("\n");
}

void command_clear(void) {
    terminal_clear();
}

void command_about(void) {
    terminal_writestring("MyOS v1.1 - Operating System with ELF Loader\n");
    terminal_writestring("==============================================\n");
    terminal_writestring("Features:\n");
    terminal_writestring("  - 32-bit protected mode\n");
    terminal_writestring("  - Dynamic memory management\n");
    terminal_writestring("  - Virtual memory with paging\n");
    terminal_writestring("  - Simple file system\n");
    terminal_writestring("  - Basic task scheduler\n");
    terminal_writestring("  - Enhanced keyboard support\n");
    terminal_writestring("  - VGA cursor support\n");
    terminal_writestring("  - Interrupt handling\n");
    terminal_writestring("  - Interactive command shell\n");
    terminal_writestring("  - ELF executable loader\n");
    terminal_writestring("  - User mode execution\n");
    terminal_writestring("  - System calls (INT 0x80)\n");
    terminal_writestring("\nNew in v1.1:\n");
    terminal_writestring("  - Full ELF32 loader implementation\n");
    terminal_writestring("  - User mode task execution\n");
    terminal_writestring("  - ELF program validation and loading\n");
    terminal_writestring("  - File-based ELF execution\n");
    terminal_writestring("\nBuilt with Assembly and C\n");
    terminal_writestring("For x86 architecture\n\n");
}

void command_memory(void) {
    uint32_t total, free, used;
    get_memory_info(&total, &free, &used);
    
    terminal_writestring("Memory Usage:\n");
    terminal_writestring("  Total: ");
    print_number(total);
    terminal_writestring(" bytes\n");
    terminal_writestring("  Free:  ");
    print_number(free);
    terminal_writestring(" bytes\n");
    terminal_writestring("  Used:  ");
    print_number(used);
    terminal_writestring(" bytes\n");
    
    // Процент использования
    uint32_t percent = (used * 100) / total;
    terminal_writestring("  Usage: ");
    print_number(percent);
    terminal_writestring("%\n\n");
}

void command_memtest(void) {
    terminal_writestring("Testing memory allocator...\n");
    
    // Тест 1: выделение и освобождение
    void* ptr1 = kmalloc(128);
    terminal_writestring("Allocated 128 bytes\n");
    
    void* ptr2 = kmalloc(256);
    terminal_writestring("Allocated 256 bytes\n");
    
    void* ptr3 = kmalloc(64);
    terminal_writestring("Allocated 64 bytes\n");
    
    kfree(ptr2);
    terminal_writestring("Block 2 freed\n");
    
    kfree(ptr1);
    kfree(ptr3);
    terminal_writestring("All blocks freed\n");
    
    terminal_writestring("Memory test completed!\n\n");
}

void command_keyboard(void) {
    terminal_writestring("Keyboard Status (v2.0):\n");
    
    uint8_t modifiers = keyboard_get_modifiers();
    keyboard_state_t* state = keyboard_get_state();
    
    terminal_writestring("  Modifier Keys:\n");
    terminal_writestring("    Shift: ");
    if (IS_SHIFT_PRESSED(modifiers)) {
        terminal_writestring("PRESSED\n");
    } else {
        terminal_writestring("Released\n");
    }
    
    terminal_writestring("    Ctrl:  ");
    if (IS_CTRL_PRESSED(modifiers)) {
        terminal_writestring("PRESSED\n");
    } else {
        terminal_writestring("Released\n");
    }
    
    terminal_writestring("    Alt:   ");
    if (IS_ALT_PRESSED(modifiers)) {
        terminal_writestring("PRESSED\n");
    } else {
        terminal_writestring("Released\n");
    }
    
    terminal_writestring("  Caps Lock: ");
    if (IS_CAPS_ACTIVE(modifiers)) {
        terminal_writestring("ON\n");
    } else {
        terminal_writestring("OFF\n");
    }
    
    terminal_writestring("  Last Key: ");
    if (state->last_char) {
        terminal_putchar(state->last_char);
        terminal_writestring(" (scancode: ");
        print_number(state->last_scancode);
        terminal_writestring(")\n");
    } else {
        terminal_writestring("None\n");
    }
    
    terminal_writestring("\n  New Features v2.0:\n");
    terminal_writestring("    - Extended key support (F1-F12, arrows, etc.)\n");
    terminal_writestring("    - E0-prefixed scancodes\n");
    terminal_writestring("    - Event-driven architecture\n");
    terminal_writestring("    - Modular keyboard system\n");
    terminal_writestring("    - Full 256-key scancode table\n");
    terminal_writestring("\n  Try all keys including F1-F12!\n\n");
}

void command_tasks(void) {
    terminal_writestring("Task List:\n");
    terminal_writestring("ID   Name             State    Priority\n");
    terminal_writestring("------------------------------------\n");
    
    if (!task_list) {
        terminal_writestring("No tasks created yet.\n\n");
        return;
    }
    
    task_t* task = task_list;
    while (task) {
        print_number(task->id);
        terminal_writestring("    ");
        terminal_writestring(task->name);
        
        // Выравнивание
        int name_len = strlen(task->name);
        for (int i = name_len; i < 16; i++) {
            terminal_putchar(' ');
        }
        
        switch (task->state) {
            case TASK_STATE_RUNNING:
                terminal_writestring("RUNNING  ");
                break;
            case TASK_STATE_READY:
                terminal_writestring("READY    ");
                break;
            case TASK_STATE_BLOCKED:
                terminal_writestring("BLOCKED  ");
                break;
            case TASK_STATE_DEAD:
                terminal_writestring("DEAD     ");
                break;
            default:
                terminal_writestring("UNKNOWN  ");
        }
        
        print_number(task->priority);
        terminal_putchar('\n');
        
        task = task->next;
    }
    
    terminal_writestring("\nCurrent task: ");
    if (current_task) {
        terminal_writestring(current_task->name);
    } else {
        terminal_writestring("None");
    }
    terminal_writestring("\n\n");
}

void command_schedule(void) {
    terminal_writestring("Triggering scheduler...\n");
    schedule();
    terminal_writestring("Scheduler executed\n\n");
}

void command_ls(void) {
    fs_list_files();
}

void command_touch(const char* filename) {
    if (filename == NULL || filename[0] == '\0') {
        terminal_writestring("Usage: touch <filename>\n");
        return;
    }
    
    if (fs_create_file(filename) >= 0) {
        terminal_writestring("File created: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
    }
}

void command_cat(const char* filename) {
    if (filename == NULL || filename[0] == '\0') {
        terminal_writestring("Usage: cat <filename>\n");
        return;
    }
    
    char buffer[FS_MAX_FILESIZE + 1];
    int bytes_read = fs_read_file(filename, buffer, FS_MAX_FILESIZE);
    
    if (bytes_read >= 0) {
        buffer[bytes_read] = '\0'; // Null-terminate
        terminal_writestring("Content of ");
        terminal_writestring(filename);
        terminal_writestring(":\n");
        terminal_writestring(buffer);
        if (bytes_read > 0 && buffer[bytes_read-1] != '\n') {
            terminal_writestring("\n");
        }
    }
}

void command_rm(const char* filename) {
    if (filename == NULL || filename[0] == '\0') {
        terminal_writestring("Usage: rm <filename>\n");
        return;
    }
    
    if (fs_delete_file(filename) == 0) {
        terminal_writestring("File deleted: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
    }
}

void command_echo(const char* args) {
    if (args == NULL || args[0] == '\0') {
        terminal_writestring("Usage: echo <text> > <filename>\n");
        terminal_writestring("Example: echo Hello World > myfile.txt\n");
        return;
    }
    
    // Простой парсинг команды echo "text" > filename
    const char* text_start = args;
    const char* redirect_pos = NULL;
    
    // Ищем символ '>'
    for (const char* p = args; *p; p++) {
        if (*p == '>') {
            redirect_pos = p;
            break;
        }
    }
    
    if (!redirect_pos) {
        terminal_writestring("Error: Missing '>' redirection\n");
        terminal_writestring("Usage: echo <text> > <filename>\n");
        return;
    }
    
    // Извлекаем текст (до '>')
    char text[256];
    int text_len = redirect_pos - text_start;
    if (text_len >= 256) text_len = 255;
    
    // Копируем текст, убираем пробелы в конце
    int i;
    for (i = 0; i < text_len; i++) {
        text[i] = text_start[i];
    }
    while (i > 0 && text[i-1] == ' ') i--; // Убираем пробелы в конце
    text[i] = '\0';
    
    // Извлекаем имя файла (после '>')
    const char* filename_start = redirect_pos + 1;
    while (*filename_start == ' ') filename_start++; // Пропускаем пробелы
    
    char filename[FS_MAX_FILENAME];
    for (i = 0; i < FS_MAX_FILENAME - 1 && filename_start[i] && filename_start[i] != ' '; i++) {
        filename[i] = filename_start[i];
    }
    filename[i] = '\0';
    
    if (filename[0] == '\0') {
        terminal_writestring("Error: Missing filename\n");
        return;
    }
    
    // Создаем файл если не существует
    if (!fs_file_exists(filename)) {
        if (fs_create_file(filename) < 0) {
            return; // Ошибка уже выведена
        }
    }
    
    // Записываем данные
    if (fs_write_file(filename, text, strlen(text)) == 0) {
        terminal_writestring("Text written to ");
        terminal_writestring(filename);
        terminal_writestring("\n");
    }
}

void command_testelf(void) {
    terminal_writestring("Testing ELF loader...\n");
    
    // Создаем тестовую ELF задачу
    task_t* elf_task = create_elf_task("test_program", test_elf_program, test_elf_size, 10);
    
    if (elf_task) {
        terminal_writestring("ELF test program loaded successfully!\n");
        terminal_writestring("Task ID: ");
        print_number(elf_task->id);
        terminal_writestring("\n");
        terminal_writestring("This program will exit immediately via sys_exit\n");
    } else {
        terminal_writestring("Failed to load ELF test program\n");
    }
}

void command_run(const char* filename) {
    if (!filename || strlen(filename) == 0) {
        terminal_writestring("Usage: run <filename>\n");
        return;
    }
    
    terminal_writestring("Loading ELF program: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    
    uint8_t* elf_data;
    uint32_t elf_size;
    
    if (load_elf_from_file(filename, &elf_data, &elf_size)) {
        task_t* elf_task = create_elf_task(filename, elf_data, elf_size, 10);
        
        if (elf_task) {
            terminal_writestring("Program loaded successfully!\n");
            terminal_writestring("Task ID: ");
            print_number(elf_task->id);
            terminal_writestring("\n");
        } else {
            terminal_writestring("Failed to create task from ELF file\n");
        }
        
        kfree(elf_data);
    } else {
        terminal_writestring("Failed to load ELF file\n");
    }
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

void execute_command(const char* command) {
    if (strcmp(command, "") == 0) {
        // Пустая команда
        return;
    } 
    
    // Парсим команду и аргументы
    char cmd[64];
    char args[256];
    int i = 0, j = 0;
    
    // Извлекаем команду (до первого пробела)
    while (command[i] && command[i] != ' ' && i < 63) {
        cmd[i] = command[i];
        i++;
    }
    cmd[i] = '\0';
    
    // Пропускаем пробелы
    while (command[i] == ' ') i++;
    
    // Извлекаем аргументы
    while (command[i] && j < 255) {
        args[j] = command[i];
        i++;
        j++;
    }
    args[j] = '\0';
    
    // Выполняем команды
    if (strcmp(cmd, "help") == 0) {
        command_help();
    } else if (strcmp(cmd, "clear") == 0) {
        command_clear();
    } else if (strcmp(cmd, "about") == 0) {
        command_about();
    } else if (strcmp(cmd, "memory") == 0) {
        command_memory();
    } else if (strcmp(cmd, "memtest") == 0) {
        command_memtest();
    } else if (strcmp(cmd, "keyboard") == 0) {
        command_keyboard();
    } else if (strcmp(cmd, "tasks") == 0) {
        command_tasks();
    } else if (strcmp(cmd, "schedule") == 0) {
        command_schedule();
    } else if (strcmp(cmd, "ls") == 0) {
        command_ls();
    } else if (strcmp(cmd, "touch") == 0) {
        command_touch(args);
    } else if (strcmp(cmd, "cat") == 0) {
        command_cat(args);
    } else if (strcmp(cmd, "rm") == 0) {
        command_rm(args);
    } else if (strcmp(cmd, "echo") == 0) {
        command_echo(args);
    } else if (strcmp(cmd, "testelf") == 0) {
        command_testelf();
    } else if (strcmp(cmd, "run") == 0) {
        command_run(args);
    } else if (strcmp(cmd, "reboot") == 0) {
        command_reboot();
    } else if (strcmp(cmd, "poweroff") == 0) {
        command_poweroff();
    } else {
        terminal_writestring("Unknown command: ");
        terminal_writestring(command);
        terminal_writestring("\n");
        terminal_writestring("Type 'help' for available commands\n");
    }
}

void setup_gdt_user_segments(void) {
    // Простая функция-заглушка для GDT
    // В реальной реализации здесь была бы настройка GDT
}

void init_timer(int frequency) {
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    outb(PIT_COMMAND, 0x36);         // Канал 0, режим 3, бинарный
    outb(PIT_DATA0, divisor & 0xFF);
    outb(PIT_DATA0, (divisor >> 8) & 0xFF);
    
    terminal_writestring("Timer initialized at ");
    print_number(frequency);
    terminal_writestring(" Hz\n");
}

// Главная функция
void kernel_main(void) {
    terminal_clear();
    
    // Включаем VGA курсор
    enable_cursor(14, 15); // Обычный курсор
    
    terminal_writestring("MyOS v1.1 - Operating System with ELF Loader\n");
    terminal_writestring("==============================================\n\n");

    // Настройка GDT для поддержки пользовательского режима
    terminal_writestring("Setting up GDT for user mode...\n");
    setup_gdt_user_segments();
    terminal_writestring("GDT configured\n");

    // Сначала настраиваем IDT ДО включения прерываний
    terminal_writestring("Setting up interrupt handlers...\n");
    
    // Настройка IDT
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)&idt;

    // Инициализируем все записи IDT как пустые
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Устанавливаем обработчики исключений (0-31)
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)exception_handler, 0x08, 0x8E);
    }
    
    // Устанавливаем специальный обработчик Page Fault (исключение 14)
    idt_set_gate(14, (uint32_t)page_fault_handler, 0x08, 0x8E);

    // Устанавливаем обработчик системных вызовов (INT 0x80 = прерывание 128)
    idt_set_gate(128, (uint32_t)syscall_handler, 0x08, 0x8E);
    
    // Устанавливаем обработчик таймера (IRQ0 = прерывание 32)
    idt_set_gate(32, (uint32_t)timer_handler, 0x08, 0x8E);
    
    // Устанавливаем обработчик клавиатуры (IRQ1 = прерывание 33)
    idt_set_gate(33, (uint32_t)irq1_handler, 0x08, 0x8E);
    idt_flush();
    
    terminal_writestring("IDT configured with system calls and timer\n");

    // Инициализация управления памятью
    init_memory_management();
    terminal_writestring("Memory management initialized\n");
    
    // Инициализация файловой системы
    init_filesystem();
    terminal_writestring("File system ready\n");
    
    // Инициализация планировщика задач
    init_scheduler();
    terminal_writestring("Task scheduler ready\n");
    
    // Инициализация PIC (Programmable Interrupt Controller)
    terminal_writestring("Initializing PIC...\n");
    outb(PIC1_COMMAND, 0x11); // ICW1: Инициализация
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20);    // ICW2: Смещение векторов прерываний (IRQ0-7 -> 32-39)
    outb(PIC2_DATA, 0x28);    // ICW2: Смещение векторов прерываний (IRQ8-15 -> 40-47)
    outb(PIC1_DATA, 0x04);    // ICW3: Подключение PIC2 к IRQ2
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);    // ICW4: Режим 8086
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0xFC);    // Маска: разрешаем IRQ0 (таймер) и IRQ1 (клавиатура)
    outb(PIC2_DATA, 0xFF);    // Маска: отключаем все прерывания PIC2
    terminal_writestring("PIC configured for timer and keyboard\n");
    
    // Инициализация таймера (100 Hz)
    init_timer(TIMER_FREQUENCY);
    
    // Инициализация модуля клавиатуры
    terminal_writestring("Initializing keyboard module...\n");
    keyboard_init();
    keyboard_set_callback(shell_keyboard_callback);
    terminal_writestring("Keyboard module ready\n");
    
    // Включаем прерывания
    terminal_writestring("Enabling interrupts...\n");
    asm volatile("sti");
    terminal_writestring("Interrupts enabled.\n");

    // Создаем демонстрационные задачи
    terminal_writestring("Creating demo tasks...\n");
    create_task("idle", idle_task, 255);
    create_task("demo1", demo_task1, 10);
    create_task("demo2", demo_task2, 20);
    terminal_writestring("Demo tasks created\n");

    // Инициализация шелла
    terminal_writestring("Starting ELF-enabled shell...\n");
    enable_cursor(14, 15);
    terminal_writestring("\n=== MyOS v2.0 ===\n");
    terminal_writestring("Type 'help' for available commands.\n\n");
    
    shell_ready = 1;
    shell_prompt();

    while (1) {
        asm volatile("hlt");
    }
}