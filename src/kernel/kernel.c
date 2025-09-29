typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned int size_t;

#define NULL ((void *)0)

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
#define HEAP_START 0x400000 // Начало кучи (4 MB)
#define HEAP_SIZE 0x100000  // Размер кучи (1 MB)
#define BLOCK_SIZE sizeof(memory_block_t)

// Виртуальная память и пейджинг
#define PAGE_SIZE 4096    // Размер страницы 4KB
#define PAGE_ENTRIES 1024 // Количество записей в таблице страниц
#define PAGE_DIRECTORY_SIZE PAGE_ENTRIES * sizeof(uint32_t)
#define PAGE_TABLE_SIZE PAGE_ENTRIES * sizeof(uint32_t)

// Флаги для Page Directory Entry и Page Table Entry
#define PAGE_PRESENT 0x001  // Страница присутствует в памяти
#define PAGE_WRITABLE 0x002 // Страница доступна для записи
#define PAGE_USER 0x004     // Страница доступна пользователю
#define PAGE_PWT 0x008      // Page Write Through
#define PAGE_PCD 0x010      // Page Cache Disable
#define PAGE_ACCESSED 0x020 // Страница была accessed
#define PAGE_DIRTY 0x040    // Страница была изменена
#define PAGE_PS 0x080       // Page Size (только для PDE)
#define PAGE_GLOBAL 0x100   // Глобальная страница

// Раздел адресного пространства: 0..0xBFFFFFFF — ядро, 0xC0000000.. — пользователь
#define USER_SPACE_BASE 0xC0000000

// Адреса для размещения структур пейджинга
#define PAGE_DIRECTORY_ADDR 0x300000 // 3MB - Page Directory
#define PAGE_TABLES_ADDR 0x301000    // 3MB+4KB - Page Tables

// Файловая система
#define FS_MAX_FILES 64      // Максимум файлов
#define FS_MAX_FILENAME 32   // Максимум символов в имени файла
#define FS_MAX_FILESIZE 1024 // Максимум байт в файле
#define FS_BLOCK_SIZE 64     // Размер блока данных
#define FS_MAX_BLOCKS 256    // Максимум блоков данных
#define FS_MAX_DIR_ENTRIES 16 // Максимум записей в директории

#define FS_INODE_FREE 0 // Свободный inode
#define FS_INODE_FILE 1 // Обычный файл
#define FS_INODE_DIR 2  // Директория

// Планировщик задач
#define MAX_TASKS 8          // Максимум задач
#define TASK_STACK_SIZE 4096 // Размер стека для каждой задачи

#define TASK_STATE_RUNNING 0 // Выполняется
#define TASK_STATE_READY 1   // Готова к выполнению
#define TASK_STATE_BLOCKED 2 // Заблокирована
#define TASK_STATE_DEAD 3    // Завершена

// VGA курсор
#define VGA_CURSOR_COMMAND_PORT 0x3D4
#define VGA_CURSOR_DATA_PORT 0x3D5

// Клавиатура
#define KEY_LSHIFT_PRESSED 0x2A
#define KEY_LSHIFT_RELEASED 0xAA
#define KEY_RSHIFT_PRESSED 0x36
#define KEY_RSHIFT_RELEASED 0xB6
#define KEY_CTRL_PRESSED 0x1D
#define KEY_CTRL_RELEASED 0x9D
#define KEY_ALT_PRESSED 0x38
#define KEY_ALT_RELEASED 0xB8
#define KEY_CAPS_LOCK 0x3A

// Шелл
#define COMMAND_BUFFER_SIZE 256

// Системные вызовы
#define SYS_EXIT 0
#define SYS_WRITE 1
#define SYS_READ 2
#define SYS_OPEN 3
#define SYS_CLOSE 4
#define SYS_FORK 5
#define SYS_EXEC 6
#define SYS_WAIT 7
#define SYS_GETPID 8
#define SYS_YIELD 9
#define SYS_SLEEP 10
#define SYS_GETPPID 11
#define SYS_GETUID 12
#define SYS_GETGID 13
#define SYS_CHDIR 14
#define SYS_GETCWD 15
#define SYS_STAT 16
#define SYS_IOCTL 17
#define SYS_FCNTL 18
#define SYS_MMAP 19
#define SYS_MUNMAP 20

// Таймер (PIT - Programmable Interval Timer)
#define PIT_FREQUENCY 1193182
#define TIMER_FREQUENCY 100 // 100 Hz = 10ms тики
#define PIT_COMMAND 0x43
#define PIT_DATA0 0x40

// GDT (Global Descriptor Table)
#define GDT_ENTRIES 8
#define GDT_NULL 0x00        // Нулевой дескриптор
#define GDT_KERNEL_CODE 0x08 // Дескриптор кода ядра (Ring 0)
#define GDT_KERNEL_DATA 0x10 // Дескриптор данных ядра (Ring 0)
#define GDT_USER_CODE 0x1B   // Дескриптор кода пользователя (Ring 3)
#define GDT_USER_DATA 0x23   // Дескриптор данных пользователя (Ring 3)

// ELF загрузчик
#define ELF_LOAD_BASE 0x8000000 // Базовый адрес загрузки ELF программ (128MB)

// ===== Простые макросы проверки адресов пользователя =====
static inline int is_user_address(const void *ptr, uint32_t size)
{
    uint32_t start = (uint32_t)ptr;
    uint32_t end = start + (size ? size - 1 : 0);
    return (start >= USER_SPACE_BASE) && (end >= USER_SPACE_BASE);
}

static inline int is_cpl3(void)
{
    int cpl;
    asm volatile("mov %%cs, %0" : "=r"(cpl));
    return (cpl & 3) == 3;
}

// ===== БЕЗОПАСНОЕ КОПИРОВАНИЕ USER/KERNEL =====
static int copy_from_user(void *kernel_dst, const void *user_src, uint32_t size)
{
    if (!is_user_address(user_src, size)) return -1;
    if (size == 0) return 0;
    
    // Простая проверка: читаем по байту с обработкой page fault
    uint8_t *dst = (uint8_t *)kernel_dst;
    const uint8_t *src = (const uint8_t *)user_src;
    
    for (uint32_t i = 0; i < size; i++) {
        // В реальной системе здесь был бы try/catch для page fault
        // Пока просто копируем напрямую
        dst[i] = src[i];
    }
    return 0;
}

static int copy_to_user(void *user_dst, const void *kernel_src, uint32_t size)
{
    if (!is_user_address(user_dst, size)) return -1;
    if (size == 0) return 0;
    
    const uint8_t *src = (const uint8_t *)kernel_src;
    uint8_t *dst = (uint8_t *)user_dst;
    
    for (uint32_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }
    return 0;
}

// Флаги открытия файлов
#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR   0x0002
#define O_CREAT  0x0040
#define O_TRUNC  0x0200
#define O_APPEND 0x0400

// Стандартные файловые дескрипторы
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// IDT структуры
struct idt_entry
{
    uint16_t base_lo, sel;
    uint8_t always0, flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// GDT структуры
struct gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Структура блока памяти
typedef struct memory_block
{
    size_t size;               // Размер блока
    int is_free;               // Флаг: свободен ли блок
    struct memory_block *next; // Указатель на следующий блок
    struct memory_block *prev; // Указатель на предыдущий блок
} memory_block_t;

// Структуры для виртуальной памяти
typedef uint32_t page_directory_entry_t; // PDE - 32-битная запись
typedef uint32_t page_table_entry_t;     // PTE - 32-битная запись

// Структура Page Directory
typedef struct
{
    page_directory_entry_t entries[PAGE_ENTRIES];
} page_directory_t;

// Структура Page Table
typedef struct
{
    page_table_entry_t entries[PAGE_ENTRIES];
} page_table_t;

// === СТРУКТУРЫ ФАЙЛОВОЙ СИСТЕМЫ ===

// Суперблок файловой системы
typedef struct
{
    uint32_t magic;        // Магическое число для проверки
    uint32_t total_inodes; // Общее количество inodes
    uint32_t free_inodes;  // Свободные inodes
    uint32_t total_blocks; // Общее количество блоков данных
    uint32_t free_blocks;  // Свободные блоки
    uint32_t block_size;   // Размер блока данных
} fs_superblock_t;

// Индексный узел (inode) файла
typedef struct
{
    uint8_t type;                   // Тип: свободный, файл, директория
    char filename[FS_MAX_FILENAME]; // Имя файла
    uint32_t size;                  // Размер файла в байтах
    uint32_t blocks[16];            // Прямые указатели на блоки данных
    uint32_t created_time;          // Время создания (упрощенно)
    uint32_t modified_time;         // Время модификации
    uint32_t parent_inode;          // Родительская директория (для файлов)
} fs_inode_t;

// Запись директории
typedef struct
{
    uint32_t inode_number;      // Номер inode
    char name[FS_MAX_FILENAME]; // Имя файла
    uint8_t type;               // Тип записи (файл/директория)
} fs_dir_entry_t;

// Глобальное состояние файловой системы
typedef struct
{
    fs_superblock_t superblock;          // Суперблок
    fs_inode_t inodes[FS_MAX_FILES];     // Таблица inodes
    uint8_t *data_blocks;                // Указатель на область данных
    uint8_t inode_bitmap[FS_MAX_FILES];  // Битовая карта занятых inodes
    uint8_t block_bitmap[FS_MAX_BLOCKS]; // Битовая карта занятых блоков
    int initialized;                     // Флаг инициализации
} fs_state_t;

// === СТРУКТУРЫ ПЛАНИРОВЩИКА ЗАДАЧ ===

// Состояние регистров для переключения контекста
typedef struct
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, esp, ebp;
    uint32_t eip, eflags;
    uint16_t cs, ds, es, fs, gs, ss;
} registers_t;

// ELF структуры теперь определены в elf.h

// Структура файлового дескриптора
typedef struct file_descriptor
{
    int fd;                   // Номер файлового дескриптора
    int flags;                // Флаги открытия (O_RDONLY, O_WRONLY, O_RDWR)
    uint32_t position;        // Текущая позиция в файле
    char filename[FS_MAX_FILENAME]; // Имя файла
    int valid;                // Валидность дескриптора
} file_descriptor_t;

// Структура процесса
typedef struct process
{
    uint32_t pid;             // Process ID
    uint32_t ppid;            // Parent Process ID
    uint32_t uid;             // User ID
    uint32_t gid;             // Group ID
    char working_dir[256];    // Текущая рабочая директория
    file_descriptor_t fds[32]; // Файловые дескрипторы (0-31)
    int next_fd;              // Следующий свободный FD
    uint32_t page_directory;  // Адрес Page Directory процесса
    uint32_t memory_limit;    // Лимит памяти процесса
    uint32_t memory_used;     // Используемая память
} process_t;

// Структура задачи
typedef struct task
{
    uint32_t id;              // ID задачи
    char name[32];            // Имя задачи
    uint32_t state;           // Состояние задачи
    uint32_t priority;        // Приоритет (0-высший, 255-низший)
    registers_t regs;         // Сохраненные регистры
    uint32_t *stack;          // Стек задачи
    uint32_t stack_size;      // Размер стека
    uint32_t time_slice;      // Оставшееся время выполнения
    elf_loader_t *elf_loader; // ELF-загрузчик для этой задачи
    process_t process;        // Информация о процессе
    struct task *next;        // Следующая задача в списке
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
memory_block_t *heap_start = NULL;
uint32_t total_memory = 0;
uint32_t free_memory = 0;
uint32_t used_memory = 0;

// Переменные виртуальной памяти
page_directory_t *page_directory = (page_directory_t *)PAGE_DIRECTORY_ADDR;
page_table_t *page_tables = (page_table_t *)PAGE_TABLES_ADDR;
int paging_enabled = 0;

// Переменные файловой системы
fs_state_t filesystem;
uint32_t fs_time_counter = 0; // Простой счетчик времени

// Переменные планировщика
task_t *current_task = NULL;
task_t *task_list = NULL;
uint32_t next_task_id = 1;
uint32_t scheduler_ticks = 0;

// Переменные шелла
char command_buffer[COMMAND_BUFFER_SIZE];
int command_length = 0;
int shell_ready = 0;

// Переменные шелла для обработки ввода
void shell_keyboard_callback(keyboard_event_t *event);

// Предварительные объявления всех функций
void terminal_writestring(const char *data);
void terminal_putchar(char c);
void print_number(uint32_t num);
void terminal_clear(void);
void shell_prompt(void);
void execute_command(const char *command);
// Графический режим (объявления перед использованием)
static void enter_graphics(void);
static void exit_graphics(void);
static void fb_present(void);

// ===== VERY EARLY SERIAL DEBUG (COM1) =====
#define COM1_PORT 0x3F8
static inline void serial_out(uint16_t port, uint8_t val){ asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port)); }
static inline uint8_t serial_in(uint16_t port){ uint8_t v; asm volatile ("inb %1, %0" : "=a"(v) : "Nd"(port)); return v; }
static void serial_init(void)
{
    serial_out(COM1_PORT + 1, 0x00); // Disable all interrupts
    serial_out(COM1_PORT + 3, 0x80); // Enable DLAB
    serial_out(COM1_PORT + 0, 0x03); // Set divisor to 3 (38400 baud)
    serial_out(COM1_PORT + 1, 0x00);
    serial_out(COM1_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    serial_out(COM1_PORT + 2, 0xC7); // Enable FIFO, clear, 14-byte threshold
    serial_out(COM1_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
}
static int serial_can_tx(void) { return serial_in(COM1_PORT + 5) & 0x20; }
static void serial_write_char(char c){ while (!serial_can_tx()); serial_out(COM1_PORT, (uint8_t)c); }
static void serial_write_string(const char *s){ while (*s) { if (*s=='\n') serial_write_char('\r'); serial_write_char(*s++); } }

// Объявления функций файловой системы
void init_filesystem(void);
int fs_create_file(const char *filename);
int fs_delete_file(const char *filename);
int fs_write_file(const char *filename, const char *data, uint32_t size);
int fs_read_file(const char *filename, char *buffer, uint32_t max_size);
void fs_list_files(void);
int fs_file_exists(const char *filename);
fs_inode_t *fs_find_inode(const char *filename);

// Функции для работы с директориями
int fs_create_directory(const char *dirname);
int fs_delete_directory(const char *dirname);
int fs_list_directory(const char *dirname);
int fs_change_directory(const char *dirname);
int fs_get_current_directory(char *buffer, uint32_t size);
fs_inode_t *fs_find_inode_in_dir(uint32_t parent_inode, const char *name);
int fs_add_entry_to_dir(uint32_t parent_inode, uint32_t child_inode, const char *name, uint8_t type);
int fs_remove_entry_from_dir(uint32_t parent_inode, const char *name);

// Объявления функций планировщика
void init_scheduler(void);
task_t *create_task(const char *name, void (*entry_point)(void), uint32_t priority);
void schedule(void);
void task_yield(void);
void switch_to_task(task_t *task);

// Объявления функций управления процессами
void init_process_management(void);
task_t *fork_process(task_t *parent);
int exec_process(const char *filename, char **argv);
int wait_process(uint32_t pid);
void cleanup_process(task_t *task);
int allocate_fd(task_t *task, const char *filename, int flags);
void free_fd(task_t *task, int fd);
file_descriptor_t *get_fd(task_t *task, int fd);

// Функции для управления памятью процессов
uint32_t create_process_page_directory(void);
void destroy_process_page_directory(uint32_t page_dir);
void switch_to_process_page_directory(uint32_t page_dir);
int map_memory_for_process(task_t *task, uint32_t virtual_addr, uint32_t physical_addr, uint32_t size, int flags);
int unmap_memory_for_process(task_t *task, uint32_t virtual_addr, uint32_t size);
void *allocate_memory_for_process(task_t *task, uint32_t size);
void free_memory_for_process(task_t *task, void *ptr, uint32_t size);

// Объявления функций ELF-загрузчика теперь в elf.h
// Локальные функции
task_t *create_elf_task(const char *name, uint8_t *elf_data, uint32_t elf_size, uint32_t priority);
int load_elf_from_file(const char *filename, uint8_t **elf_data, uint32_t *elf_size);
void cleanup_elf_task(task_t *task);

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
extern void save_context(task_t *task);
extern void restore_context(task_t *task);
extern void switch_context(task_t *current, task_t *next);
extern void init_task_context(task_t *task, void *entry_point, void *stack_top);
extern void task_switch(task_t *next_task);

// Функции пользовательского режима (из usermode.asm)
extern void switch_to_user_mode(uint32_t user_esp, uint32_t user_eip);
extern void switch_to_kernel_mode(void);
extern void create_user_task(uint32_t entry_point, uint32_t stack_top, task_t *task);
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
void shell_keyboard_callback(keyboard_event_t *event)
{
    if (!event->pressed || !event->character)
    {
        return; // Обрабатываем только нажатия с символами
    }

    char c = event->character;

    if (c == '\n')
    {
        // Enter - выполняем команду
        command_buffer[command_length] = '\0';
        terminal_putchar('\n');
        execute_command(command_buffer);
        command_length = 0;
        shell_prompt();
    }
    else if (c == '\b')
    {
        // Backspace
        if (command_length > 0)
        {
            command_length--;
            terminal_putchar('\b');
        }
    }
    else if (command_length < COMMAND_BUFFER_SIZE - 1)
    {
        // Обычный символ
        command_buffer[command_length++] = c;
        terminal_putchar(c);
    }
}

// Порты
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Функции для работы с памятью
void *memset(void *dest, int val, size_t len)
{
    uint8_t *ptr = (uint8_t *)dest;
    while (len-- > 0)
    {
        *ptr++ = val;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t len)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (len-- > 0)
    {
        *d++ = *s++;
    }
    return dest;
}

// Функции для работы со строками
int strlen(const char *str)
{
    int len = 0;
    while (str[len])
        len++;
    return len;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

void strcpy(char *dest, const char *src)
{
    while ((*dest++ = *src++))
        ;
}

int strncmp(const char *str1, const char *str2, size_t n)
{
    while (n-- && *str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return (n == 0) ? 0 : (*str1 - *str2);
}

char *strncpy(char *dest, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}

// === ФУНКЦИИ ТЕРМИНАЛА ===

// Функции для управления VGA курсором
void update_cursor(int x, int y)
{
    uint16_t pos = y * VGA_WIDTH + x;

    outb(VGA_CURSOR_COMMAND_PORT, 0x0F);
    outb(VGA_CURSOR_DATA_PORT, (uint8_t)(pos & 0xFF));
    outb(VGA_CURSOR_COMMAND_PORT, 0x0E);
    outb(VGA_CURSOR_DATA_PORT, (uint8_t)((pos >> 8) & 0xFF));
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    outb(VGA_CURSOR_COMMAND_PORT, 0x0A);
    outb(VGA_CURSOR_DATA_PORT, (inb(VGA_CURSOR_DATA_PORT) & 0xC0) | cursor_start);

    outb(VGA_CURSOR_COMMAND_PORT, 0x0B);
    outb(VGA_CURSOR_DATA_PORT, (inb(VGA_CURSOR_DATA_PORT) & 0xE0) | cursor_end);
}

void disable_cursor()
{
    outb(VGA_CURSOR_COMMAND_PORT, 0x0A);
    outb(VGA_CURSOR_DATA_PORT, 0x20);
}

void terminal_clear(void)
{
    uint16_t *video_memory = (uint16_t *)VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        video_memory[i] = 0x0720; // Пробел с белым текстом на черном фоне
    }
    terminal_row = 0;
    terminal_column = 0;
    update_cursor(terminal_column, terminal_row);
}

void terminal_putchar(char c)
{
    uint16_t *video_memory = (uint16_t *)VGA_MEMORY;

    if (c == '\n')
    {
        terminal_row++;
        terminal_column = 0;
    }
    else if (c == '\b')
    {
        if (terminal_column > 0)
        {
            terminal_column--;
            video_memory[terminal_row * VGA_WIDTH + terminal_column] = 0x0720;
        }
        update_cursor(terminal_column, terminal_row);
        return;
    }
    else
    {
        video_memory[terminal_row * VGA_WIDTH + terminal_column] = (0x07 << 8) | c;
        terminal_column++;
    }

    if (terminal_column >= VGA_WIDTH)
    {
        terminal_column = 0;
        terminal_row++;
    }

    if (terminal_row >= VGA_HEIGHT)
    {
        // Прокрутка экрана вверх
        for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
        {
            video_memory[i] = video_memory[i + VGA_WIDTH];
        }
        for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
        {
            video_memory[i] = 0x0720;
        }
        terminal_row = VGA_HEIGHT - 1;
    }

    update_cursor(terminal_column, terminal_row);
}

void terminal_writestring(const char *data)
{
    while (*data)
    {
        terminal_putchar(*data++);
    }
}

void print_number(uint32_t num)
{
    if (num == 0)
    {
        terminal_putchar('0');
        return;
    }

    char buffer[12];
    int i = 0;

    while (num > 0)
    {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i > 0)
    {
        terminal_putchar(buffer[--i]);
    }
}

void print_hex(uint32_t num)
{
    terminal_writestring("0x");

    if (num == 0)
    {
        terminal_putchar('0');
        return;
    }

    char buffer[9];
    int i = 0;

    while (num > 0)
    {
        int digit = num % 16;
        buffer[i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        num /= 16;
    }

    while (--i >= 0)
    {
        terminal_putchar(buffer[i]);
    }
}

// Инициализация системы управления памятью
void init_memory_management()
{
    heap_start = (memory_block_t *)HEAP_START;
    heap_start->size = HEAP_SIZE - BLOCK_SIZE;
    heap_start->is_free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;

    total_memory = HEAP_SIZE;
    free_memory = HEAP_SIZE - BLOCK_SIZE;
    used_memory = BLOCK_SIZE;
}

// Выделение памяти (простой аллокатор)
void *kmalloc(size_t size)
{
    if (size == 0)
        return NULL;

    // Выравниваем размер по границе 4 байта
    size = (size + 3) & ~3;

    memory_block_t *current = heap_start;

    // Ищем подходящий свободный блок
    while (current != NULL)
    {
        if (current->is_free && current->size >= size)
        {
            // Если блок намного больше нужного, разделяем его
            if (current->size > size + BLOCK_SIZE + 16)
            {
                memory_block_t *new_block = (memory_block_t *)((uint32_t)current + BLOCK_SIZE + size);
                new_block->size = current->size - size - BLOCK_SIZE;
                new_block->is_free = 1;
                new_block->next = current->next;
                new_block->prev = current;

                if (current->next)
                {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = size;
            }

            current->is_free = 0;
            free_memory -= current->size;
            used_memory += current->size;

            return (void *)((uint32_t)current + BLOCK_SIZE);
        }
        current = current->next;
    }

    return NULL; // Память не найдена
}

// Освобождение памяти
void kfree(void *ptr)
{
    if (ptr == NULL)
        return;

    memory_block_t *block = (memory_block_t *)((uint32_t)ptr - BLOCK_SIZE);

    if (block->is_free)
        return; // Уже освобожден

    block->is_free = 1;
    free_memory += block->size;
    used_memory -= block->size;

    // Объединяем смежные свободные блоки
    if (block->next && block->next->is_free)
    {
        block->size += block->next->size + BLOCK_SIZE;
        if (block->next->next)
        {
            block->next->next->prev = block;
        }
        block->next = block->next->next;
    }

    if (block->prev && block->prev->is_free)
    {
        block->prev->size += block->size + BLOCK_SIZE;
        if (block->next)
        {
            block->next->prev = block->prev;
        }
        block->prev->next = block->next;
    }
}

// Получение информации о памяти
void get_memory_info(uint32_t *total, uint32_t *free, uint32_t *used)
{
    *total = total_memory;
    *free = free_memory;
    *used = used_memory;
}

// === ELF ЗАГРУЗЧИК ===

// ELF магические числа и константы
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define ELFCLASS32 1
#define ELFDATA2LSB 1
#define EV_CURRENT 1
#define ET_EXEC 2
#define EM_386 3
#define PT_LOAD 1

// Validate ELF file format
int elf_validate(uint8_t *data, uint32_t size)
{
    if (!data || size < sizeof(elf_header_t))
    {
        return 0; // Invalid: NULL data or too small
    }

    elf_header_t *header = (elf_header_t *)data;

    // Check ELF magic number
    if (header->e_ident[0] != ELFMAG0 ||
        header->e_ident[1] != ELFMAG1 ||
        header->e_ident[2] != ELFMAG2 ||
        header->e_ident[3] != ELFMAG3)
    {
        return 0; // Invalid magic number
    }

    // Check class (32-bit)
    if (header->e_ident[4] != ELFCLASS32)
    {
        return 0; // Only support 32-bit ELF
    }

    // Check data encoding (little endian)
    if (header->e_ident[5] != ELFDATA2LSB)
    {
        return 0; // Only support little endian
    }

    // Check ELF version
    if (header->e_ident[6] != EV_CURRENT)
    {
        return 0; // Invalid version
    }

    // Check machine type (i386)
    if (header->e_machine != EM_386)
    {
        return 0; // Only support i386
    }

    // Check file type (executable)
    if (header->e_type != ET_EXEC)
    {
        return 0; // Only support executable files
    }

    // Check program header table
    if (header->e_phoff == 0 || header->e_phnum == 0)
    {
        return 0; // No program headers
    }

    // Validate program header bounds
    uint32_t ph_end = header->e_phoff + (header->e_phnum * header->e_phentsize);
    if (ph_end > size)
    {
        return 0; // Program headers exceed file size
    }

    return 1; // Valid ELF file
}

// Parse ELF file and initialize loader context
int elf_parse(elf_loader_t *loader, uint8_t *data, uint32_t size)
{
    if (!loader || !data || size == 0)
    {
        return 0;
    }

    // Clear loader structure
    memset(loader, 0, sizeof(elf_loader_t));

    // Validate ELF file
    if (!elf_validate(data, size))
    {
        return 0;
    }

    // Initialize loader context
    loader->data = data;
    loader->size = size;
    loader->header = (elf_header_t *)data;
    loader->valid = 1;

    // Set up program headers
    if (loader->header->e_phoff > 0 && loader->header->e_phnum > 0)
    {
        loader->pheaders = (elf_program_header_t *)(data + loader->header->e_phoff);
    }

    // Capture PT_LOAD segments for demand paging
    loader->num_segments = 0;
    loader->min_vaddr = 0xFFFFFFFF;
    if (loader->pheaders) {
        for (uint32_t i = 0; i < loader->header->e_phnum && loader->num_segments < 16; i++) {
            elf_program_header_t *ph = &loader->pheaders[i];
            if (ph->p_type == PT_LOAD) {
                loader->segments[loader->num_segments].vaddr = ph->p_vaddr;
                loader->segments[loader->num_segments].offset = ph->p_offset;
                loader->segments[loader->num_segments].filesz = ph->p_filesz;
                loader->segments[loader->num_segments].memsz = ph->p_memsz;
                loader->segments[loader->num_segments].flags = ph->p_flags;
                if (ph->p_vaddr < loader->min_vaddr) loader->min_vaddr = ph->p_vaddr;
                loader->num_segments++;
            }
        }
        if (loader->min_vaddr == 0xFFFFFFFF) loader->min_vaddr = 0;
    }

    // Set entry point
    loader->entry_point = loader->header->e_entry;

    return 1;
}

// Load ELF program into memory
uint32_t elf_load_program(elf_loader_t *loader)
{
    if (!loader || !loader->valid)
    {
        return 0;
    }

    uint32_t min_addr = 0xFFFFFFFF;
    uint32_t max_addr = 0;

    // First pass: find memory range needed
    for (int i = 0; i < loader->header->e_phnum; i++)
    {
        elf_program_header_t *ph = &loader->pheaders[i];

        if (ph->p_type == PT_LOAD)
        {
            if (ph->p_vaddr < min_addr)
            {
                min_addr = ph->p_vaddr;
            }
            if (ph->p_vaddr + ph->p_memsz > max_addr)
            {
                max_addr = ph->p_vaddr + ph->p_memsz;
            }
        }
    }

    if (min_addr == 0xFFFFFFFF)
    {
        return 0; // No loadable segments
    }

    // Use fixed load address
    uint32_t load_base = ELF_LOAD_BASE;
    loader->load_base = load_base;

    // Second pass: actually load segments (eager load for now; DP also supported)
    for (int i = 0; i < loader->header->e_phnum; i++)
    {
        elf_program_header_t *ph = &loader->pheaders[i];

        if (ph->p_type == PT_LOAD)
        {
            // Calculate load address
            uint32_t load_addr = load_base + (ph->p_vaddr - min_addr);

            // Copy file data to memory
            if (ph->p_filesz > 0)
            {
                memcpy((void *)load_addr,
                       loader->data + ph->p_offset,
                       ph->p_filesz);
            }

            // Zero out any remaining memory (BSS section)
            if (ph->p_memsz > ph->p_filesz)
            {
                memset((void *)(load_addr + ph->p_filesz), 0,
                       ph->p_memsz - ph->p_filesz);
            }
        }
    }

    // Adjust entry point to loaded address
    loader->entry_point = load_base + (loader->header->e_entry - min_addr);

    return loader->entry_point;
}

// Get entry point address
int elf_get_entry_point(elf_loader_t *loader, uint32_t *entry)
{
    if (!loader || !loader->valid || !entry)
    {
        return 0;
    }

    *entry = loader->entry_point;
    return 1;
}

// Clean up loader resources
void elf_cleanup(elf_loader_t *loader)
{
    if (!loader)
    {
        return;
    }

    // Clear the structure
    memset(loader, 0, sizeof(elf_loader_t));
}

// Создание задачи из ELF файла
task_t *create_elf_task(const char *name, uint8_t *elf_data, uint32_t elf_size, uint32_t priority)
{
    if (!name || !elf_data || elf_size == 0)
    {
        return NULL;
    }

    // Создаем новую задачу
    task_t *task = (task_t *)kmalloc(sizeof(task_t));
    if (!task)
    {
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
    task->stack = (uint32_t *)kmalloc(TASK_STACK_SIZE);
    if (!task->stack)
    {
        terminal_writestring("Error: Failed to allocate stack for task\n");
        kfree(task);
        return NULL;
    }
    task->stack_size = TASK_STACK_SIZE;

    // Создаем ELF загрузчик
    task->elf_loader = (elf_loader_t *)kmalloc(sizeof(elf_loader_t));
    if (!task->elf_loader)
    {
        terminal_writestring("Error: Failed to allocate ELF loader\n");
        kfree(task->stack);
        kfree(task);
        return NULL;
    }

    // Парсим ELF файл
    if (!elf_parse(task->elf_loader, elf_data, elf_size))
    {
        terminal_writestring("Error: Failed to parse ELF file\n");
        kfree(task->elf_loader);
        kfree(task->stack);
        kfree(task);
        return NULL;
    }

    // Загружаем программу в память
    uint32_t entry_point = elf_load_program(task->elf_loader);
    if (entry_point == 0)
    {
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
int load_elf_from_file(const char *filename, uint8_t **elf_data, uint32_t *elf_size)
{
    if (!filename || !elf_data || !elf_size)
    {
        return 0;
    }

    // Проверяем существование файла
    if (!fs_file_exists(filename))
    {
        terminal_writestring("Error: ELF file not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return 0;
    }

    fs_inode_t *inode = fs_find_inode(filename);
    if (!inode)
    {
        return 0;
    }

    *elf_size = inode->size;
    *elf_data = (uint8_t *)kmalloc(*elf_size);
    if (!*elf_data)
    {
        terminal_writestring("Error: Failed to allocate memory for ELF file\n");
        return 0;
    }

    // Читаем данные файла
    if (fs_read_file(filename, (char *)*elf_data, *elf_size) <= 0)
    {
        terminal_writestring("Error: Failed to read ELF file\n");
        kfree(*elf_data);
        *elf_data = NULL;
        *elf_size = 0;
        return 0;
    }

    return 1;
}

// Очистка ресурсов ELF задачи
void cleanup_elf_task(task_t *task)
{
    if (!task)
    {
        return;
    }

    if (task->elf_loader)
    {
        elf_cleanup(task->elf_loader);
        kfree(task->elf_loader);
    }

    if (task->stack)
    {
        kfree(task->stack);
    }

    kfree(task);
}

// === ТЕСТОВАЯ ELF ПРОГРАММА ===

// Простая ELF программа "Hello, World!" для тестирования
uint8_t test_elf_program[] = {
    // ELF header
    0x7f, 0x45, 0x4c, 0x46, // Magic: 0x7f, 'E', 'L', 'F'
    0x01,                   // Class: ELFCLASS32
    0x01,                   // Data: ELFDATA2LSB
    0x01,                   // Version: EV_CURRENT
    0x00,                   // ABI: SYSV
    0x00, 0x00, 0x00, 0x00, // ABI version and padding
    0x00, 0x00, 0x00, 0x00, // More padding

    0x02, 0x00,             // Type: ET_EXEC
    0x03, 0x00,             // Machine: EM_386
    0x01, 0x00, 0x00, 0x00, // Version: 1
    0x54, 0x80, 0x04, 0x08, // Entry point: 0x08048054
    0x34, 0x00, 0x00, 0x00, // Program header offset: 52
    0x00, 0x00, 0x00, 0x00, // Section header offset: 0
    0x00, 0x00, 0x00, 0x00, // Flags: 0
    0x34, 0x00,             // ELF header size: 52
    0x20, 0x00,             // Program header size: 32
    0x01, 0x00,             // Program header count: 1
    0x00, 0x00,             // Section header size: 0
    0x00, 0x00,             // Section header count: 0
    0x00, 0x00,             // String table index: 0

    // Program header
    0x01, 0x00, 0x00, 0x00, // Type: PT_LOAD
    0x00, 0x00, 0x00, 0x00, // Offset: 0
    0x00, 0x80, 0x04, 0x08, // Virtual address: 0x08048000
    0x00, 0x80, 0x04, 0x08, // Physical address: 0x08048000
    0x74, 0x00, 0x00, 0x00, // File size: 116
    0x74, 0x00, 0x00, 0x00, // Memory size: 116
    0x05, 0x00, 0x00, 0x00, // Flags: PF_R | PF_X
    0x00, 0x10, 0x00, 0x00, // Alignment: 4096

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
    0x90, 0x90, 0x90};

uint32_t test_elf_size = sizeof(test_elf_program);

// === ФАЙЛОВАЯ СИСТЕМА ===

void init_filesystem(void)
{
    memset(&filesystem, 0, sizeof(fs_state_t));

    // Инициализируем суперблок
    filesystem.superblock.magic = 0x12345678;
    filesystem.superblock.total_inodes = FS_MAX_FILES;
    filesystem.superblock.free_inodes = FS_MAX_FILES;
    filesystem.superblock.total_blocks = FS_MAX_BLOCKS;
    filesystem.superblock.free_blocks = FS_MAX_BLOCKS;
    filesystem.superblock.block_size = FS_BLOCK_SIZE;

    // Выделяем память для блоков данных
    filesystem.data_blocks = (uint8_t *)kmalloc(FS_MAX_BLOCKS * FS_BLOCK_SIZE);
    if (!filesystem.data_blocks)
    {
        terminal_writestring("Error: Failed to allocate filesystem data blocks\n");
        return;
    }

    memset(filesystem.data_blocks, 0, FS_MAX_BLOCKS * FS_BLOCK_SIZE);
    memset(filesystem.inode_bitmap, 0, FS_MAX_FILES);
    memset(filesystem.block_bitmap, 0, FS_MAX_BLOCKS);

    filesystem.initialized = 1;

    terminal_writestring("Filesystem initialized\n");
}

int fs_create_file(const char *filename)
{
    if (!filesystem.initialized || !filename)
        return -1;

    // Проверяем, не существует ли файл уже
    if (fs_file_exists(filename))
    {
        terminal_writestring("File already exists: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return -1;
    }

    // Ищем свободный inode
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (filesystem.inode_bitmap[i] == 0)
        {
            filesystem.inode_bitmap[i] = 1;
            filesystem.superblock.free_inodes--;

            fs_inode_t *inode = &filesystem.inodes[i];
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

int fs_file_exists(const char *filename)
{
    if (!filesystem.initialized || !filename)
        return 0;

    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (filesystem.inode_bitmap[i] &&
            filesystem.inodes[i].type == FS_INODE_FILE &&
            strcmp(filesystem.inodes[i].filename, filename) == 0)
        {
            return 1;
        }
    }

    return 0;
}

fs_inode_t *fs_find_inode(const char *filename)
{
    if (!filesystem.initialized || !filename)
        return NULL;

    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (filesystem.inode_bitmap[i] &&
            filesystem.inodes[i].type == FS_INODE_FILE &&
            strcmp(filesystem.inodes[i].filename, filename) == 0)
        {
            return &filesystem.inodes[i];
        }
    }

    return NULL;
}

int fs_write_file(const char *filename, const char *data, uint32_t size)
{
    if (!filesystem.initialized || !filename || !data || size == 0)
        return -1;

    fs_inode_t *inode = fs_find_inode(filename);
    if (!inode)
    {
        terminal_writestring("File not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return -1;
    }

    if (size > FS_MAX_FILESIZE)
    {
        size = FS_MAX_FILESIZE;
    }

    // Простая реализация: записываем в первый блок
    if (inode->blocks[0] == 0)
    {
        // Ищем свободный блок
        for (uint32_t i = 0; i < FS_MAX_BLOCKS; i++)
        {
            if (filesystem.block_bitmap[i] == 0)
            {
                filesystem.block_bitmap[i] = 1;
                filesystem.superblock.free_blocks--;
                inode->blocks[0] = i;
                break;
            }
        }
    }

    if (inode->blocks[0] > 0)
    {
        uint8_t *block_data = filesystem.data_blocks + (inode->blocks[0] * FS_BLOCK_SIZE);
        memcpy(block_data, data, size);
        inode->size = size;
        inode->modified_time = fs_time_counter++;
        return 0;
    }

    terminal_writestring("No free blocks available\n");
    return -1;
}

int fs_read_file(const char *filename, char *buffer, uint32_t max_size)
{
    if (!filesystem.initialized || !filename || !buffer)
        return -1;

    fs_inode_t *inode = fs_find_inode(filename);
    if (!inode || inode->size == 0)
        return -1;

    uint32_t read_size = (inode->size < max_size) ? inode->size : max_size;

    if (inode->blocks[0] > 0)
    {
        uint8_t *block_data = filesystem.data_blocks + (inode->blocks[0] * FS_BLOCK_SIZE);
        memcpy(buffer, block_data, read_size);
        return read_size;
    }

    return -1;
}

int fs_delete_file(const char *filename)
{
    if (!filesystem.initialized || !filename)
        return -1;

    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (filesystem.inode_bitmap[i] &&
            filesystem.inodes[i].type == FS_INODE_FILE &&
            strcmp(filesystem.inodes[i].filename, filename) == 0)
        {

            fs_inode_t *inode = &filesystem.inodes[i];

            // Освобождаем блоки данных
            for (int j = 0; j < 16 && inode->blocks[j] > 0; j++)
            {
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

void fs_list_files(void)
{
    if (!filesystem.initialized)
    {
        terminal_writestring("ERROR: File system not initialized!\n");
        return;
    }

    terminal_writestring("Files in filesystem:\n");
    terminal_writestring("Name               Size      Time\n");
    terminal_writestring("----------------------------------\n");

    int file_count = 0;
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (filesystem.inodes[i].type == FS_INODE_FILE)
        {
            fs_inode_t *inode = &filesystem.inodes[i];

            // Имя файла
            terminal_writestring(inode->filename);

            // Выравнивание
            int name_len = strlen(inode->filename);
            for (int j = name_len; j < 18; j++)
            {
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

    if (file_count == 0)
    {
        terminal_writestring("No files found.\n");
    }
    else
    {
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

// === ФУНКЦИИ ДЛЯ РАБОТЫ С ДИРЕКТОРИЯМИ ===

// Создание директории
int fs_create_directory(const char *dirname)
{
    if (!filesystem.initialized || !dirname)
        return -1;

    // Проверяем, не существует ли директория уже
    if (fs_file_exists(dirname))
    {
        terminal_writestring("Directory already exists: ");
        terminal_writestring(dirname);
        terminal_writestring("\n");
        return -1;
    }

    // Ищем свободный inode
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (filesystem.inode_bitmap[i] == 0)
        {
            filesystem.inode_bitmap[i] = 1;
            filesystem.superblock.free_inodes--;

            fs_inode_t *inode = &filesystem.inodes[i];
            memset(inode, 0, sizeof(fs_inode_t));
            inode->type = FS_INODE_DIR;
            strncpy(inode->filename, dirname, FS_MAX_FILENAME - 1);
            inode->created_time = fs_time_counter++;
            inode->modified_time = inode->created_time;
            inode->parent_inode = 0; // Корневая директория

            // Выделяем блок для записей директории
            for (uint32_t j = 0; j < FS_MAX_BLOCKS; j++)
            {
                if (filesystem.block_bitmap[j] == 0)
                {
                    filesystem.block_bitmap[j] = 1;
                    filesystem.superblock.free_blocks--;
                    inode->blocks[0] = j;
                    break;
                }
            }

            // Инициализируем блок директории
            if (inode->blocks[0] > 0)
            {
                uint8_t *block_data = filesystem.data_blocks + (inode->blocks[0] * FS_BLOCK_SIZE);
                memset(block_data, 0, FS_BLOCK_SIZE);
                inode->size = 0; // Пустая директория
            }

            return i;
        }
    }

    terminal_writestring("No free inodes available\n");
    return -1;
}

// Удаление директории
int fs_delete_directory(const char *dirname)
{
    if (!filesystem.initialized || !dirname)
        return -1;

    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (filesystem.inode_bitmap[i] &&
            filesystem.inodes[i].type == FS_INODE_DIR &&
            strcmp(filesystem.inodes[i].filename, dirname) == 0)
        {
            fs_inode_t *inode = &filesystem.inodes[i];

            // Проверяем, что директория пуста
            if (inode->size > 0)
            {
                terminal_writestring("Directory not empty: ");
                terminal_writestring(dirname);
                terminal_writestring("\n");
                return -1;
            }

            // Освобождаем блоки данных
            for (int j = 0; j < 16 && inode->blocks[j] > 0; j++)
            {
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

    terminal_writestring("Directory not found: ");
    terminal_writestring(dirname);
    terminal_writestring("\n");
    return -1;
}

// Список содержимого директории
int fs_list_directory(const char *dirname)
{
    if (!filesystem.initialized || !dirname)
        return -1;

    fs_inode_t *dir_inode = fs_find_inode(dirname);
    if (!dir_inode || dir_inode->type != FS_INODE_DIR)
    {
        terminal_writestring("Directory not found: ");
        terminal_writestring(dirname);
        terminal_writestring("\n");
        return -1;
    }

    terminal_writestring("Contents of ");
    terminal_writestring(dirname);
    terminal_writestring(":\n");

    if (dir_inode->blocks[0] > 0)
    {
        uint8_t *block_data = filesystem.data_blocks + (dir_inode->blocks[0] * FS_BLOCK_SIZE);
        fs_dir_entry_t *entries = (fs_dir_entry_t *)block_data;
        
        int entry_count = dir_inode->size / sizeof(fs_dir_entry_t);
        for (int i = 0; i < entry_count; i++)
        {
            if (entries[i].inode_number > 0)
            {
                fs_inode_t *entry_inode = &filesystem.inodes[entries[i].inode_number];
                if (entry_inode->type == FS_INODE_DIR)
                {
                    terminal_writestring("[DIR]  ");
                }
                else
                {
                    terminal_writestring("[FILE] ");
                }
                terminal_writestring(entries[i].name);
                terminal_putchar('\n');
            }
        }
    }

    if (dir_inode->size == 0)
    {
        terminal_writestring("(empty)\n");
    }

    return 0;
}

// Поиск inode в директории
fs_inode_t *fs_find_inode_in_dir(uint32_t parent_inode, const char *name)
{
    if (!filesystem.initialized || parent_inode >= FS_MAX_FILES || !name)
        return NULL;

    fs_inode_t *parent = &filesystem.inodes[parent_inode];
    if (parent->type != FS_INODE_DIR || !parent->blocks[0])
        return NULL;

    uint8_t *block_data = filesystem.data_blocks + (parent->blocks[0] * FS_BLOCK_SIZE);
    fs_dir_entry_t *entries = (fs_dir_entry_t *)block_data;
    
    int entry_count = parent->size / sizeof(fs_dir_entry_t);
    for (int i = 0; i < entry_count; i++)
    {
        if (entries[i].inode_number > 0 && strcmp(entries[i].name, name) == 0)
        {
            return &filesystem.inodes[entries[i].inode_number];
        }
    }

    return NULL;
}

// Добавление записи в директорию
int fs_add_entry_to_dir(uint32_t parent_inode, uint32_t child_inode, const char *name, uint8_t type)
{
    if (!filesystem.initialized || parent_inode >= FS_MAX_FILES || child_inode >= FS_MAX_FILES || !name)
        return -1;

    fs_inode_t *parent = &filesystem.inodes[parent_inode];
    if (parent->type != FS_INODE_DIR || !parent->blocks[0])
        return -1;

    uint8_t *block_data = filesystem.data_blocks + (parent->blocks[0] * FS_BLOCK_SIZE);
    fs_dir_entry_t *entries = (fs_dir_entry_t *)block_data;
    
    // Ищем свободную запись
    int entry_count = parent->size / sizeof(fs_dir_entry_t);
    for (int i = 0; i < FS_MAX_DIR_ENTRIES; i++)
    {
        if (i >= entry_count || entries[i].inode_number == 0)
        {
            entries[i].inode_number = child_inode;
            strncpy(entries[i].name, name, FS_MAX_FILENAME - 1);
            entries[i].name[FS_MAX_FILENAME - 1] = '\0';
            entries[i].type = type;
            
            if (i >= entry_count)
            {
                parent->size += sizeof(fs_dir_entry_t);
            }
            
            parent->modified_time = fs_time_counter++;
            return 0;
        }
    }

    return -1; // Директория полна
}

// Удаление записи из директории
int fs_remove_entry_from_dir(uint32_t parent_inode, const char *name)
{
    if (!filesystem.initialized || parent_inode >= FS_MAX_FILES || !name)
        return -1;

    fs_inode_t *parent = &filesystem.inodes[parent_inode];
    if (parent->type != FS_INODE_DIR || !parent->blocks[0])
        return -1;

    uint8_t *block_data = filesystem.data_blocks + (parent->blocks[0] * FS_BLOCK_SIZE);
    fs_dir_entry_t *entries = (fs_dir_entry_t *)block_data;
    
    int entry_count = parent->size / sizeof(fs_dir_entry_t);
    for (int i = 0; i < entry_count; i++)
    {
        if (entries[i].inode_number > 0 && strcmp(entries[i].name, name) == 0)
        {
            entries[i].inode_number = 0;
            memset(entries[i].name, 0, FS_MAX_FILENAME);
            entries[i].type = 0;
            parent->modified_time = fs_time_counter++;
            return 0;
        }
    }

    return -1; // Запись не найдена
}

// === ФУНКЦИИ ДЛЯ УПРАВЛЕНИЯ ПАМЯТЬЮ ПРОЦЕССОВ ===

// Создание Page Directory для процесса
uint32_t create_process_page_directory(void)
{
    // Выделяем память для Page Directory
    uint32_t page_dir_addr = (uint32_t)kmalloc(PAGE_DIRECTORY_SIZE);
    if (!page_dir_addr) return 0;

    page_directory_t *new_page_dir = (page_directory_t *)page_dir_addr;
    
    // Инициализируем все записи как несуществующие
    for (int i = 0; i < PAGE_ENTRIES; i++)
    {
        new_page_dir->entries[i] = 0;
    }

    // Копируем первые 768 записей из ядра (первые 3GB - пространство ядра)
    for (int i = 0; i < 768; i++)
    {
        new_page_dir->entries[i] = page_directory->entries[i];
    }

    // Последние 256 записей (1GB пользовательского пространства) оставляем пустыми
    // Они будут заполняться по мере необходимости

    return page_dir_addr;
}

// Уничтожение Page Directory процесса
void destroy_process_page_directory(uint32_t page_dir)
{
    if (!page_dir) return;

    page_directory_t *dir = (page_directory_t *)page_dir;
    
    // Освобождаем Page Tables для пользовательского пространства (последние 256 записей)
    for (int i = 768; i < PAGE_ENTRIES; i++)
    {
        if (dir->entries[i] & PAGE_PRESENT)
        {
            uint32_t page_table_addr = dir->entries[i] & 0xFFFFF000;
            kfree((void *)page_table_addr);
        }
    }

    // Освобождаем сам Page Directory
    kfree((void *)page_dir);
}

// Переключение на Page Directory процесса
void switch_to_process_page_directory(uint32_t page_dir)
{
    if (!page_dir) return;
    
    // Загружаем новый Page Directory в CR3
    asm volatile("mov %0, %%cr3" : : "r"(page_dir));
}

// Отображение памяти для процесса
int map_memory_for_process(task_t *task, uint32_t virtual_addr, uint32_t physical_addr, uint32_t size, int flags)
{
    if (!task || !task->process.page_directory) return -1;

    page_directory_t *page_dir = (page_directory_t *)task->process.page_directory;
    
    // Выравниваем адреса по границе страницы
    virtual_addr = virtual_addr & 0xFFFFF000;
    physical_addr = physical_addr & 0xFFFFF000;
    size = (size + PAGE_SIZE - 1) & 0xFFFFF000;

    for (uint32_t addr = virtual_addr; addr < virtual_addr + size; addr += PAGE_SIZE, physical_addr += PAGE_SIZE)
    {
        uint32_t page_dir_index = addr >> 22;
        uint32_t page_table_index = (addr >> 12) & 0x3FF;

        // Проверяем, что адрес в пользовательском пространстве
        if (page_dir_index < 768) return -1;

        // Создаем Page Table если нужно
        if (!(page_dir->entries[page_dir_index] & PAGE_PRESENT))
        {
            uint32_t page_table_addr = (uint32_t)kmalloc(PAGE_TABLE_SIZE);
            if (!page_table_addr) return -1;

            page_table_t *page_table = (page_table_t *)page_table_addr;
            memset(page_table, 0, PAGE_TABLE_SIZE);

        // Устанавливаем запись в Page Directory (учитываем флаги WRITABLE/USER)
        uint32_t pde_flags = PAGE_PRESENT;
        if (flags & PAGE_WRITABLE) pde_flags |= PAGE_WRITABLE;
        if (flags & PAGE_USER)     pde_flags |= PAGE_USER;
        page_dir->entries[page_dir_index] = page_table_addr | pde_flags;
        }

        // Получаем адрес Page Table
        uint32_t page_table_addr = page_dir->entries[page_dir_index] & 0xFFFFF000;
        page_table_t *page_table = (page_table_t *)page_table_addr;

        // Устанавливаем запись в Page Table
        uint32_t pte_flags = PAGE_PRESENT;
        if (flags & PAGE_WRITABLE) pte_flags |= PAGE_WRITABLE;
        if (flags & PAGE_USER)     pte_flags |= PAGE_USER;
        page_table->entries[page_table_index] = physical_addr | pte_flags;
    }

    return 0;
}

// Отмена отображения памяти для процесса
int unmap_memory_for_process(task_t *task, uint32_t virtual_addr, uint32_t size)
{
    if (!task || !task->process.page_directory) return -1;

    page_directory_t *page_dir = (page_directory_t *)task->process.page_directory;
    
    // Выравниваем адреса по границе страницы
    virtual_addr = virtual_addr & 0xFFFFF000;
    size = (size + PAGE_SIZE - 1) & 0xFFFFF000;

    for (uint32_t addr = virtual_addr; addr < virtual_addr + size; addr += PAGE_SIZE)
    {
        uint32_t page_dir_index = addr >> 22;
        uint32_t page_table_index = (addr >> 12) & 0x3FF;

        // Проверяем, что адрес в пользовательском пространстве
        if (page_dir_index < 768) return -1;

        if (page_dir->entries[page_dir_index] & PAGE_PRESENT)
        {
            uint32_t page_table_addr = page_dir->entries[page_dir_index] & 0xFFFFF000;
            page_table_t *page_table = (page_table_t *)page_table_addr;

            // Очищаем запись в Page Table
            page_table->entries[page_table_index] = 0;
        }
    }

    return 0;
}

// ===== GUARD PAGES ДЛЯ СТЕКА =====
#define GUARD_PAGE_SIZE PAGE_SIZE

// Выделение памяти для процесса с guard pages
void *allocate_memory_for_process(task_t *task, uint32_t size)
{
    if (!task || size == 0) return NULL;

    // Выравниваем размер по границе страницы
    size = (size + PAGE_SIZE - 1) & 0xFFFFF000;

    // Проверяем лимит памяти (учитываем guard pages)
    if (task->process.memory_used + size + GUARD_PAGE_SIZE > task->process.memory_limit)
    {
        return NULL;
    }

    // Выделяем физическую память
    void *physical_mem = kmalloc(size);
    if (!physical_mem) return NULL;

    // Находим свободный виртуальный адрес в пользовательском пространстве
    uint32_t virtual_addr = USER_SPACE_BASE + 0x1000; // Начинаем с 0xC0001000
    // В реальной системе здесь был бы более сложный алгоритм поиска свободного адреса

    // Отображаем основную память
    if (map_memory_for_process(task, virtual_addr, (uint32_t)physical_mem, size, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0)
    {
        kfree(physical_mem);
        return NULL;
    }

    // Добавляем guard page после выделенной области (без маппинга)
    // Следующая страница после virtual_addr+size остается неотображенной
    // Это защищает от переполнения буфера

    task->process.memory_used += size + GUARD_PAGE_SIZE;
    return (void *)virtual_addr;
}

// Освобождение памяти процесса
void free_memory_for_process(task_t *task, void *ptr, uint32_t size)
{
    if (!task || !ptr) return;

    // Выравниваем размер по границе страницы
    size = (size + PAGE_SIZE - 1) & 0xFFFFF000;

    // Отменяем отображение
    unmap_memory_for_process(task, (uint32_t)ptr, size);

    // Освобождаем физическую память
    // В реальной системе нужно найти физический адрес по виртуальному
    // Здесь упрощенная реализация
    kfree(ptr);

    if (task->process.memory_used >= size)
    {
        task->process.memory_used -= size;
    }
}

// === УПРАВЛЕНИЕ ПРОЦЕССАМИ ===

void init_process_management(void)
{
    terminal_writestring("Process management initialized\n");
}

// Создание нового процесса (fork)
task_t *fork_process(task_t *parent)
{
    if (!parent) return NULL;

    // Создаем новую задачу
    task_t *child = (task_t *)kmalloc(sizeof(task_t));
    if (!child) return NULL;

    // Копируем структуру задачи
    memcpy(child, parent, sizeof(task_t));

    // Устанавливаем новые ID
    child->id = next_task_id++;
    child->process.pid = child->id;
    child->process.ppid = parent->process.pid;

    // Создаем новый стек
    child->stack = (uint32_t *)kmalloc(TASK_STACK_SIZE);
    if (!child->stack) {
        kfree(child);
        return NULL;
    }

    // Копируем содержимое стека
    memcpy(child->stack, parent->stack, TASK_STACK_SIZE);

    // Обновляем указатель на стек в регистрах
    child->regs.esp = (uint32_t)child->stack + TASK_STACK_SIZE - 4;

    // Сбрасываем состояние
    child->state = TASK_STATE_READY;
    child->time_slice = 10;

    // Добавляем в список задач
    child->next = task_list;
    task_list = child;

    terminal_writestring("Process forked: PID ");
    print_number(child->process.pid);
    terminal_writestring("\n");

    return child;
}

// Выполнение программы (exec)
int exec_process(const char *filename, char **argv)
{
    (void)argv;
    if (!current_task || !filename) return -1;

    // Загружаем ELF файл
    uint8_t *elf_data;
    uint32_t elf_size;
    
    if (!load_elf_from_file(filename, &elf_data, &elf_size)) {
        return -1;
    }

    // Очищаем старый ELF загрузчик
    if (current_task->elf_loader) {
        elf_cleanup(current_task->elf_loader);
        kfree(current_task->elf_loader);
    }

    // Создаем новый ELF загрузчик
    current_task->elf_loader = (elf_loader_t *)kmalloc(sizeof(elf_loader_t));
    if (!current_task->elf_loader) {
        kfree(elf_data);
        return -1;
    }

    // Парсим и загружаем новую программу
    if (!elf_parse(current_task->elf_loader, elf_data, elf_size)) {
        kfree(current_task->elf_loader);
        kfree(elf_data);
        return -1;
    }

    uint32_t entry_point = elf_load_program(current_task->elf_loader);
    if (entry_point == 0) {
        elf_cleanup(current_task->elf_loader);
        kfree(current_task->elf_loader);
        kfree(elf_data);
        return -1;
    }

    // Обновляем имя задачи
    strcpy(current_task->name, filename);

    // Настраиваем контекст для новой программы
    uint32_t stack_top = (uint32_t)current_task->stack + current_task->stack_size - 4;
    create_user_task(entry_point, stack_top, current_task);

    kfree(elf_data);
    return 0;
}

// Ожидание завершения процесса
int wait_process(uint32_t pid)
{
    if (!current_task) return -1;

    // Ищем процесс с указанным PID
    task_t *target = task_list;
    while (target) {
        if (target->process.pid == pid) {
            if (target->state == TASK_STATE_DEAD) {
                // Процесс уже завершен
                cleanup_process(target);
                return pid;
            } else {
                // Блокируем текущий процесс до завершения целевого
                current_task->state = TASK_STATE_BLOCKED;
                schedule();
                return pid;
            }
        }
        target = target->next;
    }

    return -1; // Процесс не найден
}

// Очистка ресурсов процесса
void cleanup_process(task_t *task)
{
    if (!task) return;

    // Очищаем ELF загрузчик
    if (task->elf_loader) {
        elf_cleanup(task->elf_loader);
        kfree(task->elf_loader);
        task->elf_loader = NULL;
    }

    // Очищаем стек
    if (task->stack) {
        kfree(task->stack);
        task->stack = NULL;
    }

    // Закрываем все файловые дескрипторы
    for (int i = 0; i < 32; i++) {
        if (task->process.fds[i].valid) {
            free_fd(task, i);
        }
    }

    // Очищаем Page Directory процесса
    if (task->process.page_directory) {
        destroy_process_page_directory(task->process.page_directory);
        task->process.page_directory = 0;
    }

    terminal_writestring("Process cleaned up: PID ");
    print_number(task->process.pid);
    terminal_writestring("\n");
}

// Выделение файлового дескриптора
int allocate_fd(task_t *task, const char *filename, int flags)
{
    if (!task || !filename) return -1;

    // Ищем свободный FD
    for (int i = 3; i < 32; i++) { // Пропускаем stdin, stdout, stderr
        if (!task->process.fds[i].valid) {
            task->process.fds[i].fd = i;
            task->process.fds[i].flags = flags;
            task->process.fds[i].position = 0;
            strcpy(task->process.fds[i].filename, filename);
            task->process.fds[i].valid = 1;
            return i;
        }
    }

    return -1; // Нет свободных FD
}

// Освобождение файлового дескриптора
void free_fd(task_t *task, int fd)
{
    if (!task || fd < 0 || fd >= 32) return;

    if (task->process.fds[fd].valid) {
        memset(&task->process.fds[fd], 0, sizeof(file_descriptor_t));
    }
}

// Получение файлового дескриптора
file_descriptor_t *get_fd(task_t *task, int fd)
{
    if (!task || fd < 0 || fd >= 32) return NULL;
    if (!task->process.fds[fd].valid) return NULL;
    return &task->process.fds[fd];
}

// === ПЛАНИРОВЩИК ЗАДАЧ ===

void init_scheduler(void)
{
    current_task = NULL;
    task_list = NULL;
    next_task_id = 1;
    scheduler_ticks = 0;
    terminal_writestring("Task scheduler initialized\n");
}

task_t *create_task(const char *name, void (*entry_point)(void), uint32_t priority)
{
    task_t *task = (task_t *)kmalloc(sizeof(task_t));
    if (!task)
    {
        terminal_writestring("ERROR: Failed to allocate memory for task!\n");
        return NULL;
    }

    // Выделяем стек для задачи
    task->stack = (uint32_t *)kmalloc(TASK_STACK_SIZE);
    if (!task->stack)
    {
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

    // Инициализируем процесс
    task->process.pid = task->id;
    task->process.ppid = 0; // Корневой процесс
    task->process.uid = 0;  // root
    task->process.gid = 0;  // root
    strcpy(task->process.working_dir, "/");
    task->process.next_fd = 3; // Начинаем с FD 3
    task->process.page_directory = create_process_page_directory();
    task->process.memory_limit = 0x100000; // 1MB лимит
    task->process.memory_used = 0;

    // Инициализируем файловые дескрипторы
    memset(task->process.fds, 0, sizeof(task->process.fds));
    
    // Стандартные FD
    task->process.fds[STDIN_FILENO].fd = STDIN_FILENO;
    task->process.fds[STDIN_FILENO].flags = O_RDONLY;
    task->process.fds[STDIN_FILENO].valid = 1;
    strcpy(task->process.fds[STDIN_FILENO].filename, "/dev/stdin");

    task->process.fds[STDOUT_FILENO].fd = STDOUT_FILENO;
    task->process.fds[STDOUT_FILENO].flags = O_WRONLY;
    task->process.fds[STDOUT_FILENO].valid = 1;
    strcpy(task->process.fds[STDOUT_FILENO].filename, "/dev/stdout");

    task->process.fds[STDERR_FILENO].fd = STDERR_FILENO;
    task->process.fds[STDERR_FILENO].flags = O_WRONLY;
    task->process.fds[STDERR_FILENO].valid = 1;
    strcpy(task->process.fds[STDERR_FILENO].filename, "/dev/stderr");

    // Инициализируем регистры
    memset(&task->regs, 0, sizeof(registers_t));
    task->regs.eip = (uint32_t)entry_point;
    task->regs.esp = (uint32_t)task->stack + TASK_STACK_SIZE - 4; // Вершина стека
    task->regs.cs = 0x08;                                         // Селектор кода ядра
    task->regs.ds = 0x10;                                         // Селектор данных ядра
    task->regs.es = 0x10;
    task->regs.fs = 0x10;
    task->regs.gs = 0x10;
    task->regs.ss = 0x10;
    task->regs.eflags = 0x202; // Прерывания включены

    // Добавляем задачу в список
    if (task_list == NULL)
    {
        task_list = task;
        current_task = task;
    }
    else
    {
        task_t *last = task_list;
        while (last->next)
        {
            last = last->next;
        }
        last->next = task;
    }

    return task;
}

void schedule(void)
{
    if (!current_task || !current_task->next)
    {
        return;
    }

    // Находим следующую готовую задачу
    task_t *next_task = current_task->next;
    while (next_task && next_task->state != TASK_STATE_READY)
    {
        next_task = next_task->next;
        if (!next_task)
        {
            next_task = task_list; // Возвращаемся к началу списка
            break;
        }
    }

    if (next_task && next_task != current_task)
    {
        current_task = next_task;
        switch_to_task(current_task);
    }
}

void switch_to_task(task_t *task)
{
    if (!task)
        return;

    // Переключаем адресное пространство процесса (CR3)
    if (task->process.page_directory)
    {
        switch_to_process_page_directory(task->process.page_directory);
        flush_tlb();
    }

    // Здесь может быть реальное переключение контекста (регистров/стека)
    // через вызов низкоуровневой функции, когда будет готово
    // task_switch(task);
}

void task_yield(void)
{
    if (current_task)
    {
        current_task->time_slice = 0; // Принудительно вызываем переключение
        schedule();
    }
}

// === ДЕМОНСТРАЦИОННЫЕ ЗАДАЧИ ===

void idle_task(void)
{
    while (1)
    {
        asm volatile("hlt"); // Ждем прерывания
    }
}

void demo_task1(void)
{
    for (int i = 0; i < 5; i++)
    {
        terminal_writestring("Task 1 running... ");
        print_number(i);
        terminal_putchar('\n');

        // Имитация работы
        for (volatile int j = 0; j < 1000000; j++)
            ;
    }
}

void demo_task2(void)
{
    for (int i = 0; i < 3; i++)
    {
        terminal_writestring("Task 2 executing... ");
        print_number(i);
        terminal_putchar('\n');

        // Имитация работы
        for (volatile int j = 0; j < 500000; j++)
            ;
    }
}

// === ОБРАБОТЧИКИ ПРЕРЫВАНИЙ ===

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// ===== ЛОГГЕР И ПАНИКА =====
static void log_reg_dump(const char *msg)
{
    terminal_writestring("\n[EXC] ");
    terminal_writestring(msg);
    terminal_writestring("\n");
}

static void dump_registers(void)
{
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp;
    uint16_t cs, ds, es, fs, gs, ss;
    
    asm volatile("mov %%eax, %0" : "=m"(eax));
    asm volatile("mov %%ebx, %0" : "=m"(ebx));
    asm volatile("mov %%ecx, %0" : "=m"(ecx));
    asm volatile("mov %%edx, %0" : "=m"(edx));
    asm volatile("mov %%esi, %0" : "=m"(esi));
    asm volatile("mov %%edi, %0" : "=m"(edi));
    asm volatile("mov %%esp, %0" : "=m"(esp));
    asm volatile("mov %%ebp, %0" : "=m"(ebp));
    asm volatile("mov %%cs, %0" : "=m"(cs));
    asm volatile("mov %%ds, %0" : "=m"(ds));
    asm volatile("mov %%es, %0" : "=m"(es));
    asm volatile("mov %%fs, %0" : "=m"(fs));
    asm volatile("mov %%gs, %0" : "=m"(gs));
    asm volatile("mov %%ss, %0" : "=m"(ss));
    
    terminal_writestring("Registers:\n");
    terminal_writestring("EAX="); print_hex(eax); terminal_writestring(" EBX="); print_hex(ebx);
    terminal_writestring(" ECX="); print_hex(ecx); terminal_writestring(" EDX="); print_hex(edx);
    terminal_writestring("\nESI="); print_hex(esi); terminal_writestring(" EDI="); print_hex(edi);
    terminal_writestring(" ESP="); print_hex(esp); terminal_writestring(" EBP="); print_hex(ebp);
    terminal_writestring("\nCS="); print_hex(cs); terminal_writestring(" DS="); print_hex(ds);
    terminal_writestring(" ES="); print_hex(es); terminal_writestring(" FS="); print_hex(fs);
    terminal_writestring(" GS="); print_hex(gs); terminal_writestring(" SS="); print_hex(ss);
    terminal_writestring("\n");
}

static void kernel_panic(const char *msg)
{
    terminal_writestring("\nKERNEL PANIC: ");
    terminal_writestring(msg);
    terminal_writestring("\nSystem halted.\n");
    dump_registers();
    asm volatile("cli");
    while (1) asm volatile("hlt");
}

void handle_exception(void)
{
    log_reg_dump("CPU exception occurred");
    kernel_panic("Unhandled CPU exception");
}

// ===== ПРОСТОЙ ФИЗИЧЕСКИЙ АЛЛОКАТОР (BITMAP) =====
#define MAX_PHYS_PAGES (HEAP_SIZE / PAGE_SIZE)
static uint8_t phys_bitmap[MAX_PHYS_PAGES];
static uint32_t phys_alloc_count = 0;
static uint32_t phys_free_count = 0;

static int phys_alloc_page(uint32_t *out_phys)
{
    for (uint32_t i = 0; i < MAX_PHYS_PAGES; i++) {
        if (!phys_bitmap[i]) {
            phys_bitmap[i] = 1;
            phys_alloc_count++;
            *out_phys = HEAP_START + i * PAGE_SIZE;
            return 0;
        }
    }
    return -1;
}

static void phys_free_page(uint32_t phys)
{
    if (phys < HEAP_START) return;
    uint32_t idx = (phys - HEAP_START) / PAGE_SIZE;
    if (idx < MAX_PHYS_PAGES) {
        phys_bitmap[idx] = 0;
        phys_free_count++;
    }
}

// ===== DEMAND-PAGING: подкачка страниц ELF при fault =====
static uint32_t demand_page_count = 0;

static int demand_page_load(task_t *task, uint32_t fault_addr)
{
    if (!task || !task->elf_loader) return -1;
    elf_loader_t *ldr = task->elf_loader;
    for (uint32_t i = 0; i < ldr->num_segments && i < 16; i++) {
        // Translate original vaddr to loaded vaddr using min_vaddr baseline
        uint32_t seg_start = ldr->load_base + (ldr->segments[i].vaddr - ldr->min_vaddr);
        uint32_t seg_end = seg_start + ldr->segments[i].memsz;
        if (fault_addr >= seg_start && fault_addr < seg_end) {
            uint32_t page_base = fault_addr & 0xFFFFF000;
            uint32_t phys;
            if (phys_alloc_page(&phys) != 0) return -1;
            // Инициализируем страницу из файла если попадает в filesz
            uint32_t within = page_base - seg_start;
            uint32_t file_off = ldr->segments[i].offset + within;
            memset((void *)phys, 0, PAGE_SIZE);
            if (within < ldr->segments[i].filesz) {
                uint32_t to_copy = ldr->segments[i].filesz - within;
                if (to_copy > PAGE_SIZE) to_copy = PAGE_SIZE;
                memcpy((void *)phys, ldr->data + file_off, to_copy);
            }
            // Отобразим страницу в адресное пространство процесса
            if (map_memory_for_process(task, page_base, phys, PAGE_SIZE, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) < 0) {
                phys_free_page(phys);
                return -1;
            }
            flush_tlb();
            demand_page_count++;
            return 0;
        }
    }
    return -1;
}

static void decode_pf_error(uint32_t err)
{
    terminal_writestring("PF error: ");
    if (err & 1) terminal_writestring("PRESENT "); else terminal_writestring("NOTPRESENT ");
    if (err & 2) terminal_writestring("WRITE "); else terminal_writestring("READ ");
    if (err & 4) terminal_writestring("USER "); else terminal_writestring("KERNEL ");
    if (err & 8) terminal_writestring("RESV ");
    if (err & 16) terminal_writestring("IFETCH ");
    terminal_writestring("\n");
}

void handle_page_fault(uint32_t error_code)
{
    uint32_t fault_addr = get_page_fault_address();
    if (current_task && is_user_address((void *)fault_addr, 1)) {
        if (demand_page_load(current_task, fault_addr) == 0) {
            return; // успешно подкачали
        }
    }
    terminal_writestring("Page fault at address: ");
    print_hex(fault_addr);
    terminal_writestring("\n");
    decode_pf_error(error_code);
    kernel_panic("Unhandled page fault");
}

// Старая функция keyboard_handler заменена на новый модуль клавиатуры
// Теперь используется keyboard_handler из keyboard.c

void timer_interrupt_handler(void)
{
    timer_ticks++;

    // Планировщик задач каждые 10 тиков (100ms при 100Hz)
    if (timer_ticks % 10 == 0)
    {
        scheduler_ticks++;
        if (current_task && current_task->time_slice > 0)
        {
            current_task->time_slice--;
            if (current_task->time_slice == 0)
            {
                current_task->time_slice = 10; // Сбрасываем time slice
                schedule();                    // Переключаем задачу
            }
        }
    }

    // Отправляем EOI
    outb(PIC1_COMMAND, PIC_EOI);
}

// === СИСТЕМНЫЕ ВЫЗОВЫ ===

// ===== ВАЛИДАЦИЯ СИСТЕМНЫХ ВЫЗОВОВ =====
typedef int (*syscall_fn_t)(int,int,int,int,int);

static int sys_exit_impl(int code, int _1, int _2, int _3, int _4)
{
    (void)_1; (void)_2; (void)_3; (void)_4;
    if (!current_task) return -1;
    current_task->state = TASK_STATE_DEAD;
    cleanup_process(current_task);
    schedule();
    return code;
}

static int sys_write_impl(int fdnum, int buf, int count, int _3, int _4)
{
    (void)_3; (void)_4;
    if (count < 0) return -1;
    if (is_cpl3()) {
        if (!is_user_address((void *)buf, (uint32_t)count)) return -1;
    }
    if (fdnum == STDOUT_FILENO || fdnum == STDERR_FILENO) {
        char buffer[256];
        uint32_t to_copy = (count > 255) ? 255 : count;
        if (is_cpl3()) {
            if (copy_from_user(buffer, (void *)buf, to_copy) != 0) return -1;
        } else {
            memcpy(buffer, (void *)buf, to_copy);
        }
        for (uint32_t i = 0; i < to_copy; i++) terminal_putchar(buffer[i]);
        return to_copy;
    }
    file_descriptor_t *fd = get_fd(current_task, fdnum);
    if (!fd || !fd->valid) return -1;
    
    char *kernel_buf = (char *)kmalloc(count + 1);
    if (!kernel_buf) return -1;
    
    int result = -1;
    if (is_cpl3()) {
        if (copy_from_user(kernel_buf, (void *)buf, count) == 0) {
            result = fs_write_file(fd->filename, kernel_buf, count) >= 0 ? count : -1;
        }
    } else {
        result = fs_write_file(fd->filename, (char *)buf, count) >= 0 ? count : -1;
    }
    
    kfree(kernel_buf);
    return result;
}

static int sys_getpid_impl(int _0, int _1, int _2, int _3, int _4)
{
    (void)_0; (void)_1; (void)_2; (void)_3; (void)_4;
    return current_task ? (int)current_task->process.pid : -1;
}

static int sys_read_impl(int fdnum, int buf, int count, int _3, int _4)
{
    (void)_3; (void)_4;
    if (count < 0) return -1;
    if (is_cpl3() && !is_user_address((void *)buf, (uint32_t)count)) return -1;
    
    if (fdnum == STDIN_FILENO) {
        // Пока не реализовано чтение с клавиатуры
        return 0;
    }
    
    file_descriptor_t *fd = get_fd(current_task, fdnum);
    if (!fd || !fd->valid) return -1;
    
    char *kernel_buf = (char *)kmalloc(count + 1);
    if (!kernel_buf) return -1;
    
    int result = fs_read_file(fd->filename, kernel_buf, count);
    if (result > 0) {
        if (is_cpl3()) {
            if (copy_to_user((void *)buf, kernel_buf, result) != 0) result = -1;
        } else {
            memcpy((void *)buf, kernel_buf, result);
        }
    }
    
    kfree(kernel_buf);
    return result;
}

static int sys_open_impl(int path, int flags, int _2, int _3, int _4)
{
    (void)_2; (void)_3; (void)_4;
    if (is_cpl3() && !is_user_address((void *)path, 256)) return -1;
    
    char kernel_path[256];
    if (is_cpl3()) {
        if (copy_from_user(kernel_path, (void *)path, 255) != 0) return -1;
    } else {
        strncpy(kernel_path, (char *)path, 255);
    }
    kernel_path[255] = '\0';
    
    return allocate_fd(current_task, kernel_path, flags);
}

static int sys_close_impl(int fdnum, int _1, int _2, int _3, int _4)
{
    (void)_1; (void)_2; (void)_3; (void)_4;
    free_fd(current_task, fdnum);
    return 0;
}

static int sys_fork_impl(int _0, int _1, int _2, int _3, int _4)
{
    (void)_0; (void)_1; (void)_2; (void)_3; (void)_4;
    task_t *child = fork_process(current_task);
    return child ? (int)child->process.pid : -1;
}

static int sys_exec_impl(int path, int argv, int _2, int _3, int _4)
{
    (void)_2; (void)_3; (void)_4;
    if (is_cpl3() && !is_user_address((void *)path, 256)) return -1;
    
    char kernel_path[256];
    if (is_cpl3()) {
        if (copy_from_user(kernel_path, (void *)path, 255) != 0) return -1;
    } else {
        strncpy(kernel_path, (char *)path, 255);
    }
    kernel_path[255] = '\0';
    
    return exec_process(kernel_path, (char **)argv);
}

static int sys_wait_impl(int pid, int _1, int _2, int _3, int _4)
{
    (void)_1; (void)_2; (void)_3; (void)_4;
    return wait_process(pid);
}

static int sys_yield_impl(int _0, int _1, int _2, int _3, int _4)
{
    (void)_0; (void)_1; (void)_2; (void)_3; (void)_4;
    schedule();
    return 0;
}

static int sys_getppid_impl(int _0, int _1, int _2, int _3, int _4)
{
    (void)_0; (void)_1; (void)_2; (void)_3; (void)_4;
    return current_task ? (int)current_task->process.ppid : -1;
}

static int sys_getuid_impl(int _0, int _1, int _2, int _3, int _4)
{
    (void)_0; (void)_1; (void)_2; (void)_3; (void)_4;
    return current_task ? (int)current_task->process.uid : -1;
}

static int sys_getgid_impl(int _0, int _1, int _2, int _3, int _4)
{
    (void)_0; (void)_1; (void)_2; (void)_3; (void)_4;
    return current_task ? (int)current_task->process.gid : -1;
}

static const struct { int num; syscall_fn_t fn; } syscall_table[] = {
    { SYS_EXIT,    sys_exit_impl },
    { SYS_WRITE,   sys_write_impl },
    { SYS_READ,    sys_read_impl },
    { SYS_OPEN,    sys_open_impl },
    { SYS_CLOSE,   sys_close_impl },
    { SYS_FORK,    sys_fork_impl },
    { SYS_EXEC,    sys_exec_impl },
    { SYS_WAIT,    sys_wait_impl },
    { SYS_GETPID,  sys_getpid_impl },
    { SYS_GETPPID, sys_getppid_impl },
    { SYS_GETUID,  sys_getuid_impl },
    { SYS_GETGID,  sys_getgid_impl },
    { SYS_YIELD,   sys_yield_impl },
};

static syscall_fn_t find_syscall(int num)
{
    for (unsigned i = 0; i < (sizeof(syscall_table)/sizeof(syscall_table[0])); i++)
        if (syscall_table[i].num == num) return syscall_table[i].fn;
    return NULL;
}

int handle_syscall(int syscall_num, int arg0, int arg1, int arg2, int arg3, int arg4)
{
    if (!current_task) return -1;
    syscall_fn_t fn = find_syscall(syscall_num);
    if (!fn) return -1;
    return fn(arg0, arg1, arg2, arg3, arg4);
}

// === КОМАНДЫ ШЕЛЛА ===

void shell_prompt(void)
{
    terminal_writestring("myos> ");
}

void command_help(void)
{
    terminal_writestring("Available commands:\n");
    terminal_writestring("  help       - Show this help\n");
    terminal_writestring("  clear      - Clear screen\n");
    terminal_writestring("  about      - System information\n");
    terminal_writestring("  memory     - Memory usage + stats\n");
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
    terminal_writestring("  ps         - List processes\n");
    terminal_writestring("  kill <pid> - Kill process\n");
    terminal_writestring("  fork       - Test fork system call\n");
    terminal_writestring("  pwd        - Print working directory\n");
    terminal_writestring("  cd <dir>   - Change directory\n");
    terminal_writestring("  mkdir <dir> - Create directory\n");
    terminal_writestring("  rmdir <dir> - Remove directory\n");
    terminal_writestring("  syscalls   - Test system calls\n");
    terminal_writestring("  reboot     - Restart system\n");
    terminal_writestring("  poweroff   - Shutdown system\n");
    terminal_writestring("\nELF Loader Commands:\n");
    terminal_writestring("  testelf    - Test built-in ELF program\n");
    terminal_writestring("  run <file> - Load and execute ELF file\n");
    terminal_writestring("\nProcess Management:\n");
    terminal_writestring("  ps         - List all processes\n");
    terminal_writestring("  kill <pid> - Terminate process\n");
    terminal_writestring("  fork       - Test process forking\n");
    terminal_writestring("\n");
}

void command_clear(void)
{
    terminal_clear();
}

void command_about(void)
{
    terminal_writestring("MyOS v0.7 - Operating System with ELF Loader\n");
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
    terminal_writestring("\nNew in v0.7:\n");
    terminal_writestring("  - Full ELF32 loader implementation\n");
    terminal_writestring("  - User mode task execution\n");
    terminal_writestring("  - ELF program validation and loading\n");
    terminal_writestring("  - File-based ELF execution\n");
    terminal_writestring("\nBuilt with Assembly and C\n");
    terminal_writestring("For x86 architecture\n\n");
}

void command_memory(void)
{
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
    terminal_writestring("%\n");
    
    // Статистика физических страниц
    terminal_writestring("\nPhysical Pages:\n");
    terminal_writestring("  Allocated: ");
    print_number(phys_alloc_count);
    terminal_writestring("\n");
    terminal_writestring("  Freed: ");
    print_number(phys_free_count);
    terminal_writestring("\n");
    terminal_writestring("  Demand pages loaded: ");
    print_number(demand_page_count);
    terminal_writestring("\n\n");
}

void command_memtest(void)
{
    terminal_writestring("Testing memory allocator...\n");

    // Тест 1: выделение и освобождение
    void *ptr1 = kmalloc(128);
    terminal_writestring("Allocated 128 bytes\n");

    void *ptr2 = kmalloc(256);
    terminal_writestring("Allocated 256 bytes\n");

    void *ptr3 = kmalloc(64);
    terminal_writestring("Allocated 64 bytes\n");

    kfree(ptr2);
    terminal_writestring("Block 2 freed\n");

    kfree(ptr1);
    kfree(ptr3);
    terminal_writestring("All blocks freed\n");

    terminal_writestring("Memory test completed!\n\n");
}

void command_keyboard(void)
{
    terminal_writestring("Keyboard Status (v0.7):\n");

    uint8_t modifiers = keyboard_get_modifiers();
    keyboard_state_t *state = keyboard_get_state();

    terminal_writestring("  Modifier Keys:\n");
    terminal_writestring("    Shift: ");
    if (IS_SHIFT_PRESSED(modifiers))
    {
        terminal_writestring("PRESSED\n");
    }
    else
    {
        terminal_writestring("Released\n");
    }

    terminal_writestring("    Ctrl:  ");
    if (IS_CTRL_PRESSED(modifiers))
    {
        terminal_writestring("PRESSED\n");
    }
    else
    {
        terminal_writestring("Released\n");
    }

    terminal_writestring("    Alt:   ");
    if (IS_ALT_PRESSED(modifiers))
    {
        terminal_writestring("PRESSED\n");
    }
    else
    {
        terminal_writestring("Released\n");
    }

    terminal_writestring("  Caps Lock: ");
    if (IS_CAPS_ACTIVE(modifiers))
    {
        terminal_writestring("ON\n");
    }
    else
    {
        terminal_writestring("OFF\n");
    }

    terminal_writestring("  Last Key: ");
    if (state->last_char)
    {
        terminal_putchar(state->last_char);
        terminal_writestring(" (scancode: ");
        print_number(state->last_scancode);
        terminal_writestring(")\n");
    }
    else
    {
        terminal_writestring("None\n");
    }

    terminal_writestring("\n  New Features v0.7:\n");
    terminal_writestring("    - Extended key support (F1-F12, arrows, etc.)\n");
    terminal_writestring("    - E0-prefixed scancodes\n");
    terminal_writestring("    - Event-driven architecture\n");
    terminal_writestring("    - Modular keyboard system\n");
    terminal_writestring("    - Full 256-key scancode table\n");
    terminal_writestring("\n  Try all keys including F1-F12!\n\n");
}

void command_tasks(void)
{
    terminal_writestring("Task List:\n");
    terminal_writestring("ID   Name             State    Priority\n");
    terminal_writestring("------------------------------------\n");

    if (!task_list)
    {
        terminal_writestring("No tasks created yet.\n\n");
        return;
    }

    task_t *task = task_list;
    while (task)
    {
        print_number(task->id);
        terminal_writestring("    ");
        terminal_writestring(task->name);

        // Выравнивание
        int name_len = strlen(task->name);
        for (int i = name_len; i < 16; i++)
        {
            terminal_putchar(' ');
        }

        switch (task->state)
        {
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
    if (current_task)
    {
        terminal_writestring(current_task->name);
    }
    else
    {
        terminal_writestring("None");
    }
    terminal_writestring("\n\n");
}

void command_schedule(void)
{
    terminal_writestring("Triggering scheduler...\n");
    schedule();
    terminal_writestring("Scheduler executed\n\n");
}

void command_ls(void)
{
    fs_list_files();
}

void command_touch(const char *filename)
{
    if (filename == NULL || filename[0] == '\0')
    {
        terminal_writestring("Usage: touch <filename>\n");
        return;
    }

    if (fs_create_file(filename) >= 0)
    {
        terminal_writestring("File created: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
    }
}

void command_cat(const char *filename)
{
    if (filename == NULL || filename[0] == '\0')
    {
        terminal_writestring("Usage: cat <filename>\n");
        return;
    }

    char buffer[FS_MAX_FILESIZE + 1];
    int bytes_read = fs_read_file(filename, buffer, FS_MAX_FILESIZE);

    if (bytes_read >= 0)
    {
        buffer[bytes_read] = '\0'; // Null-terminate
        terminal_writestring("Content of ");
        terminal_writestring(filename);
        terminal_writestring(":\n");
        terminal_writestring(buffer);
        if (bytes_read > 0 && buffer[bytes_read - 1] != '\n')
        {
            terminal_writestring("\n");
        }
    }
}

void command_rm(const char *filename)
{
    if (filename == NULL || filename[0] == '\0')
    {
        terminal_writestring("Usage: rm <filename>\n");
        return;
    }

    if (fs_delete_file(filename) == 0)
    {
        terminal_writestring("File deleted: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
    }
}

void command_echo(const char *args)
{
    if (args == NULL || args[0] == '\0')
    {
        terminal_writestring("Usage: echo <text> > <filename>\n");
        terminal_writestring("Example: echo Hello World > myfile.txt\n");
        return;
    }

    // Простой парсинг команды echo "text" > filename
    const char *text_start = args;
    const char *redirect_pos = NULL;

    // Ищем символ '>'
    for (const char *p = args; *p; p++)
    {
        if (*p == '>')
        {
            redirect_pos = p;
            break;
        }
    }

    if (!redirect_pos)
    {
        terminal_writestring("Error: Missing '>' redirection\n");
        terminal_writestring("Usage: echo <text> > <filename>\n");
        return;
    }

    // Извлекаем текст (до '>')
    char text[256];
    int text_len = redirect_pos - text_start;
    if (text_len >= 256)
        text_len = 255;

    // Копируем текст, убираем пробелы в конце
    int i;
    for (i = 0; i < text_len; i++)
    {
        text[i] = text_start[i];
    }
    while (i > 0 && text[i - 1] == ' ')
        i--; // Убираем пробелы в конце
    text[i] = '\0';

    // Извлекаем имя файла (после '>')
    const char *filename_start = redirect_pos + 1;
    while (*filename_start == ' ')
        filename_start++; // Пропускаем пробелы

    char filename[FS_MAX_FILENAME];
    for (i = 0; i < FS_MAX_FILENAME - 1 && filename_start[i] && filename_start[i] != ' '; i++)
    {
        filename[i] = filename_start[i];
    }
    filename[i] = '\0';

    if (filename[0] == '\0')
    {
        terminal_writestring("Error: Missing filename\n");
        return;
    }

    // Создаем файл если не существует
    if (!fs_file_exists(filename))
    {
        if (fs_create_file(filename) < 0)
        {
            return; // Ошибка уже выведена
        }
    }

    // Записываем данные
    if (fs_write_file(filename, text, strlen(text)) == 0)
    {
        terminal_writestring("Text written to ");
        terminal_writestring(filename);
        terminal_writestring("\n");
    }
}

void command_testelf(void)
{
    terminal_writestring("Testing ELF loader...\n");

    // Создаем тестовую ELF задачу
    task_t *elf_task = create_elf_task("test_program", test_elf_program, test_elf_size, 10);

    if (elf_task)
    {
        terminal_writestring("ELF test program loaded successfully!\n");
        terminal_writestring("Task ID: ");
        print_number(elf_task->id);
        terminal_writestring("\n");
        terminal_writestring("This program will exit immediately via sys_exit\n");
    }
    else
    {
        terminal_writestring("Failed to load ELF test program\n");
    }
}

void command_run(const char *filename)
{
    if (!filename || strlen(filename) == 0)
    {
        terminal_writestring("Usage: run <filename>\n");
        return;
    }

    terminal_writestring("Loading ELF program: ");
    terminal_writestring(filename);
    terminal_writestring("\n");

    uint8_t *elf_data;
    uint32_t elf_size;

    if (load_elf_from_file(filename, &elf_data, &elf_size))
    {
        task_t *elf_task = create_elf_task(filename, elf_data, elf_size, 10);

        if (elf_task)
        {
            terminal_writestring("Program loaded successfully!\n");
            terminal_writestring("Task ID: ");
            print_number(elf_task->id);
            terminal_writestring("\n");
        }
        else
        {
            terminal_writestring("Failed to create task from ELF file\n");
        }

        kfree(elf_data);
    }
    else
    {
        terminal_writestring("Failed to load ELF file\n");
    }
}

void command_reboot(void)
{
    terminal_writestring("Rebooting system...\n");
    // Простая перезагрузка через клавиатурный контроллер
    outb(0x64, 0xFE);
    while (1)
        asm volatile("hlt");
}

void command_poweroff(void)
{
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
    while (1)
        asm volatile("hlt");
}

void command_ps(void)
{
    terminal_writestring("Process List:\n");
    terminal_writestring("PID   PPID  UID   GID   State     Name\n");
    terminal_writestring("----------------------------------------\n");

    if (!task_list)
    {
        terminal_writestring("No processes found.\n\n");
        return;
    }

    task_t *task = task_list;
    while (task)
    {
        // PID
        print_number(task->process.pid);
        terminal_writestring("    ");

        // PPID
        print_number(task->process.ppid);
        terminal_writestring("    ");

        // UID
        print_number(task->process.uid);
        terminal_writestring("    ");

        // GID
        print_number(task->process.gid);
        terminal_writestring("    ");

        // State
        switch (task->state)
        {
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

        // Name
        terminal_writestring(task->name);
        terminal_putchar('\n');

        task = task->next;
    }
    terminal_putchar('\n');
}

void command_kill(const char *pid_str)
{
    if (!pid_str || strlen(pid_str) == 0)
    {
        terminal_writestring("Usage: kill <pid>\n");
        return;
    }

    // Простой парсинг PID
    uint32_t pid = 0;
    for (int i = 0; pid_str[i] && pid_str[i] >= '0' && pid_str[i] <= '9'; i++)
    {
        pid = pid * 10 + (pid_str[i] - '0');
    }

    // Ищем процесс
    task_t *task = task_list;
    while (task)
    {
        if (task->process.pid == pid)
        {
            if (task->state == TASK_STATE_DEAD)
            {
                terminal_writestring("Process already dead\n");
                return;
            }

            // Завершаем процесс
            task->state = TASK_STATE_DEAD;
            cleanup_process(task);
            terminal_writestring("Process killed: PID ");
            print_number(pid);
            terminal_writestring("\n");
            return;
        }
        task = task->next;
    }

    terminal_writestring("Process not found: PID ");
    print_number(pid);
    terminal_writestring("\n");
}

void command_fork(void)
{
    if (!current_task)
    {
        terminal_writestring("No current task to fork\n");
        return;
    }

    terminal_writestring("Testing fork system call...\n");
    task_t *child = fork_process(current_task);
    
    if (child)
    {
        terminal_writestring("Fork successful! Child PID: ");
        print_number(child->process.pid);
        terminal_writestring("\n");
    }
    else
    {
        terminal_writestring("Fork failed!\n");
    }
}

void command_pwd(void)
{
    if (!current_task)
    {
        terminal_writestring("No current task\n");
        return;
    }

    terminal_writestring("Current directory: ");
    terminal_writestring(current_task->process.working_dir);
    terminal_putchar('\n');
}

void command_cd(const char *path)
{
    if (!current_task)
    {
        terminal_writestring("No current task\n");
        return;
    }

    if (!path || strlen(path) == 0)
    {
        terminal_writestring("Usage: cd <directory>\n");
        return;
    }

    // Простая реализация - просто меняем рабочую директорию
    strcpy(current_task->process.working_dir, path);
    terminal_writestring("Changed directory to: ");
    terminal_writestring(path);
    terminal_putchar('\n');
}

void command_mkdir(const char *dirname)
{
    if (!dirname || strlen(dirname) == 0)
    {
        terminal_writestring("Usage: mkdir <directory>\n");
        return;
    }

    if (fs_create_directory(dirname) >= 0)
    {
        terminal_writestring("Directory created: ");
        terminal_writestring(dirname);
        terminal_writestring("\n");
    }
}

void command_rmdir(const char *dirname)
{
    if (!dirname || strlen(dirname) == 0)
    {
        terminal_writestring("Usage: rmdir <directory>\n");
        return;
    }

    if (fs_delete_directory(dirname) == 0)
    {
        terminal_writestring("Directory removed: ");
        terminal_writestring(dirname);
        terminal_writestring("\n");
    }
}

void command_syscalls(void)
{
    terminal_writestring("Testing system calls...\n");
    
    // Тест getpid
    terminal_writestring("getpid() = ");
    print_number(syscall1(SYS_GETPID, 0));
    terminal_writestring("\n");
    
    // Тест write
    terminal_writestring("write(1, \"Hello from syscall!\", 20) = ");
    print_number(syscall3(SYS_WRITE, 1, (int)"Hello from syscall!", 20));
    terminal_writestring("\n");
    
    // Тест getppid
    terminal_writestring("getppid() = ");
    print_number(syscall1(SYS_GETPPID, 0));
    terminal_writestring("\n");
    
    // Тест getuid/getgid
    terminal_writestring("getuid() = ");
    print_number(syscall1(SYS_GETUID, 0));
    terminal_writestring(", getgid() = ");
    print_number(syscall1(SYS_GETGID, 0));
    terminal_writestring("\n");
    
    terminal_writestring("System call tests completed!\n\n");
}

void execute_command(const char *command)
{
    if (strcmp(command, "") == 0)
    {
        // Пустая команда
        return;
    }

    // Парсим команду и аргументы
    char cmd[64];
    char args[256];
    int i = 0, j = 0;

    // Извлекаем команду (до первого пробела)
    while (command[i] && command[i] != ' ' && i < 63)
    {
        cmd[i] = command[i];
        i++;
    }
    cmd[i] = '\0';

    // Пропускаем пробелы
    while (command[i] == ' ')
        i++;

    // Извлекаем аргументы
    while (command[i] && j < 255)
    {
        args[j] = command[i];
        i++;
        j++;
    }
    args[j] = '\0';

    // Выполняем команды
    if (strcmp(cmd, "help") == 0)
    {
        command_help();
    }
    else if (strcmp(cmd, "clear") == 0)
    {
        command_clear();
    }
    else if (strcmp(cmd, "about") == 0)
    {
        command_about();
    }
    else if (strcmp(cmd, "memory") == 0)
    {
        command_memory();
    }
    else if (strcmp(cmd, "memtest") == 0)
    {
        command_memtest();
    }
    else if (strcmp(cmd, "keyboard") == 0)
    {
        command_keyboard();
    }
    else if (strcmp(cmd, "tasks") == 0)
    {
        command_tasks();
    }
    else if (strcmp(cmd, "schedule") == 0)
    {
        command_schedule();
    }
    else if (strcmp(cmd, "ls") == 0)
    {
        command_ls();
    }
    else if (strcmp(cmd, "touch") == 0)
    {
        command_touch(args);
    }
    else if (strcmp(cmd, "cat") == 0)
    {
        command_cat(args);
    }
    else if (strcmp(cmd, "rm") == 0)
    {
        command_rm(args);
    }
    else if (strcmp(cmd, "echo") == 0)
    {
        command_echo(args);
    }
    else if (strcmp(cmd, "testelf") == 0)
    {
        command_testelf();
    }
    else if (strcmp(cmd, "run") == 0)
    {
        command_run(args);
    }
    else if (strcmp(cmd, "reboot") == 0)
    {
        command_reboot();
    }
    else if (strcmp(cmd, "poweroff") == 0)
    {
        command_poweroff();
    }
    else if (strcmp(cmd, "ps") == 0)
    {
        command_ps();
    }
    else if (strcmp(cmd, "kill") == 0)
    {
        command_kill(args);
    }
    else if (strcmp(cmd, "fork") == 0)
    {
        command_fork();
    }
    else if (strcmp(cmd, "pwd") == 0)
    {
        command_pwd();
    }
    else if (strcmp(cmd, "cd") == 0)
    {
        command_cd(args);
    }
    else if (strcmp(cmd, "mkdir") == 0)
    {
        command_mkdir(args);
    }
    else if (strcmp(cmd, "rmdir") == 0)
    {
        command_rmdir(args);
    }
    else if (strcmp(cmd, "syscalls") == 0)
    {
        command_syscalls();
    }
    else if (strcmp(cmd, "gfxon") == 0)
    {
        enter_graphics();
        fb_present();
    }
    else if (strcmp(cmd, "gfxoff") == 0)
    {
        exit_graphics();
    }
    else
    {
        terminal_writestring("Unknown command: ");
        terminal_writestring(command);
        terminal_writestring("\n");
        terminal_writestring("Type 'help' for available commands\n");
    }
}

void setup_gdt_user_segments(void)
{
    // Простая функция-заглушка для GDT
    // В реальной реализации здесь была бы настройка GDT
}

void init_timer(int frequency)
{
    uint32_t divisor = PIT_FREQUENCY / frequency;

    outb(PIT_COMMAND, 0x36); // Канал 0, режим 3, бинарный
    outb(PIT_DATA0, divisor & 0xFF);
    outb(PIT_DATA0, (divisor >> 8) & 0xFF);

    terminal_writestring("Timer initialized at ");
    print_number(frequency);
    terminal_writestring(" Hz\n");
}

// === FRAMEBUFFER (Multiboot1) SUPPORT ===

typedef struct {
    uint32_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t type; // 0=text, 1=packed pixels, 2=RGB
    uint8_t red_mask_size, red_mask_pos;
    uint8_t green_mask_size, green_mask_pos;
    uint8_t blue_mask_size, blue_mask_pos;
} fb_info_t;

static fb_info_t fb;
static int fb_available = 0;
static uint8_t *fb_backbuffer = NULL;
static int graphics_mode = 0;

static inline void fb_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (!fb_available) return;
    if (x >= fb.width || y >= fb.height) return;
    uint32_t offset = y * fb.pitch + x * (fb.bpp / 8);
    volatile uint8_t *p = (volatile uint8_t *)(graphics_mode && fb_backbuffer ? (uintptr_t)fb_backbuffer + offset : fb.addr + offset);
    // Assume 32bpp X8R8G8B8 or similar; write little-endian color
    p[0] = (uint8_t)(color & 0xFF);
    p[1] = (uint8_t)((color >> 8) & 0xFF);
    p[2] = (uint8_t)((color >> 16) & 0xFF);
    p[3] = 0xFF;
}

static void fb_clear(uint32_t color)
{
    if (!fb_available) return;
    uint32_t bytes_per_pixel = fb.bpp / 8;
    if (graphics_mode && fb_backbuffer) {
        for (uint32_t y = 0; y < fb.height; y++) {
            uint8_t *row = fb_backbuffer + y * fb.pitch;
            for (uint32_t x = 0; x < fb.width; x++) {
                uint8_t *p = row + x * bytes_per_pixel;
                p[0] = (uint8_t)(color & 0xFF);
                p[1] = (uint8_t)((color >> 8) & 0xFF);
                p[2] = (uint8_t)((color >> 16) & 0xFF);
                if (bytes_per_pixel == 4) p[3] = 0xFF;
            }
        }
    } else {
        for (uint32_t y = 0; y < fb.height; y++) {
            for (uint32_t x = 0; x < fb.width; x++) {
                fb_putpixel(x, y, color);
            }
        }
    }
}

static void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (!fb_available) return;
    if (x >= fb.width || y >= fb.height) return;
    uint32_t x2 = x + w; if (x2 > fb.width) x2 = fb.width;
    uint32_t y2 = y + h; if (y2 > fb.height) y2 = fb.height;
    for (uint32_t yy = y; yy < y2; yy++)
        for (uint32_t xx = x; xx < x2; xx++)
            fb_putpixel(xx, yy, color);
}

// Минимальные структуры Multiboot1
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint8_t  color_info[6];
} __attribute__((packed)) multiboot_info_t;

static void fb_parse_multiboot(uint32_t magic, uint32_t mbi)
{
    (void)magic;
    const uint32_t MB_FLAG_FRAMEBUFFER = (1u << 12);
    multiboot_info_t *info = (multiboot_info_t *)mbi;
    if (!(info->flags & MB_FLAG_FRAMEBUFFER)) return;
    fb.addr = (uint32_t)info->framebuffer_addr; // низ 32 бит
    fb.pitch = info->framebuffer_pitch;
    fb.width = info->framebuffer_width;
    fb.height = info->framebuffer_height;
    fb.bpp = info->framebuffer_bpp;
    fb.type = info->framebuffer_type;
    if (fb.type == 1 || (fb.type == 2 && fb.bpp == 32)) {
        fb_available = 1;
    }
}

static void enter_graphics(void)
{
    if (!fb_available) {
        terminal_writestring("Framebuffer not available.\n");
        return;
    }
    graphics_mode = 1;
    if (!fb_backbuffer) {
        uint32_t size = fb.pitch * fb.height;
        fb_backbuffer = (uint8_t *)kmalloc(size);
        if (!fb_backbuffer) {
            terminal_writestring("Failed to allocate backbuffer.\n");
            graphics_mode = 0;
            return;
        }
    }
    fb_clear(0x00202020);
    fb_fill_rect(50, 50, 200, 120, 0x000080FF); // orange-ish rectangle
}

static void exit_graphics(void)
{
    graphics_mode = 0;
    terminal_clear();
}

static void fb_present(void)
{
    if (!graphics_mode || !fb_backbuffer) return;
    uint32_t size = fb.pitch * fb.height;
    memcpy((void *)fb.addr, fb_backbuffer, size);
}

// Главная функция
void kernel_main(uint32_t mb_magic, uint32_t mb_info)
{
    // Early marker: put 'K' at text cell 10
    volatile uint16_t *vgamem = (uint16_t *)0xB8000;
    vgamem[10] = (uint16_t)('K') | (0x07 << 8);
    // Early serial
    serial_init();
    serial_write_string("[BOOT] Enter kernel_main\n");
    serial_write_string("[BOOT] MB magic=");
    // print_hex uses terminal; write basic hex to serial
    const char *hex="0123456789ABCDEF";
    for (int i=28;i>=0;i-=4){ char c=hex[(mb_magic>>i)&0xF]; serial_write_char(c);} serial_write_string(" MBI=");
    for (int i=28;i>=0;i-=4){ char c=hex[(mb_info>>i)&0xF]; serial_write_char(c);} serial_write_string("\n");
    terminal_clear();
    serial_write_string("[BOOT] Cleared VGA\n");

    // Включаем VGA курсор
    enable_cursor(14, 15); // Обычный курсор

    terminal_writestring("MyOS v0.7 - Operating System with ELF Loader\n");
    serial_write_string("[BOOT] Printed banner\n");
    terminal_writestring("==============================================\n\n");

    // Настройка GDT для поддержки пользовательского режима
    terminal_writestring("Setting up GDT for user mode...\n");
    setup_gdt_user_segments();
    serial_write_string("[BOOT] GDT user segments set\n");
    terminal_writestring("GDT configured\n");

    // Сначала парсим Multiboot framebuffer
    terminal_writestring("MB magic=");
    print_hex(mb_magic);
    terminal_writestring(" MBI=");
    print_hex(mb_info);
    terminal_putchar('\n');
    fb_parse_multiboot(mb_magic, mb_info);
    serial_write_string("[BOOT] Parsed multiboot FB\n");
    terminal_writestring("FB avail=");
    print_number(fb_available);
    terminal_writestring("  ");
    print_number(fb.width);
    terminal_writestring("x");
    print_number(fb.height);
    terminal_writestring("x");
    print_number(fb.bpp);
    terminal_putchar('\n');

    // Сначала настраиваем IDT ДО включения прерываний
    terminal_writestring("Setting up interrupt handlers...\n");
    serial_write_string("[BOOT] Building IDT\n");

    // Настройка IDT
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)&idt;

    // Инициализируем все записи IDT как пустые
    for (int i = 0; i < 256; i++)
    {
        idt_set_gate(i, 0, 0, 0);
    }

    // Устанавливаем обработчики исключений (0-31)
    for (int i = 0; i < 32; i++)
    {
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
    serial_write_string("[BOOT] IDT loaded\n");

    terminal_writestring("IDT configured with system calls and timer\n");

    // Инициализация управления памятью
    init_memory_management();
    serial_write_string("[BOOT] Memory manager inited\n");
    terminal_writestring("Memory management initialized\n");

    // Инициализация файловой системы
    init_filesystem();
    serial_write_string("[BOOT] FS inited\n");
    terminal_writestring("File system ready\n");

    // Инициализация планировщика задач
    init_scheduler();
    serial_write_string("[BOOT] Scheduler inited\n");
    terminal_writestring("Task scheduler ready\n");

    // Инициализация управления процессами
    init_process_management();
    serial_write_string("[BOOT] Process mgmt inited\n");
    terminal_writestring("Process management ready\n");

    // Инициализация PIC (Programmable Interrupt Controller)
    terminal_writestring("Initializing PIC...\n");
    serial_write_string("[BOOT] PIC init\n");
    outb(PIC1_COMMAND, 0x11); // ICW1: Инициализация
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20); // ICW2: Смещение векторов прерываний (IRQ0-7 -> 32-39)
    outb(PIC2_DATA, 0x28); // ICW2: Смещение векторов прерываний (IRQ8-15 -> 40-47)
    outb(PIC1_DATA, 0x04); // ICW3: Подключение PIC2 к IRQ2
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01); // ICW4: Режим 8086
    outb(PIC2_DATA, 0x01);
    // ВРЕМЕННО: маскируем ВСЕ IRQ для локализации перезагрузки после STI
    outb(PIC1_DATA, 0xFF); // Маска: отключаем все IRQ на PIC1
    outb(PIC2_DATA, 0xFF); // Маска: отключаем все IRQ на PIC2
    terminal_writestring("PIC configured (all IRQ masked)\n");
    serial_write_string("[BOOT] PIC configured (all masked)\n");

    // Инициализация таймера (100 Hz)
    init_timer(TIMER_FREQUENCY);
    serial_write_string("[BOOT] PIT timer inited\n");

    // Инициализация модуля клавиатуры
    terminal_writestring("Initializing keyboard module...\n");
    keyboard_init();
    serial_write_string("[BOOT] Keyboard inited\n");
    keyboard_set_callback(shell_keyboard_callback);
    serial_write_string("[BOOT] Keyboard callback set\n");
    terminal_writestring("Keyboard module ready\n");

    // Включаем прерывания
    terminal_writestring("Enabling interrupts...\n");
    serial_write_string("[BOOT] STI...\n");
    asm volatile("sti");
    terminal_writestring("Interrupts enabled.\n");
    serial_write_string("[BOOT] Interrupts enabled\n");
    // Короткая задержка и метка, чтобы увидеть перезапуск
    for (volatile int i=0;i<1000000;i++);
    serial_write_string("[BOOT] Post-STI alive\n");

    // Создаем демонстрационные задачи
    terminal_writestring("Creating demo tasks...\n");
    create_task("idle", idle_task, 255);
    create_task("demo1", demo_task1, 10);
    create_task("demo2", demo_task2, 20);
    terminal_writestring("Demo tasks created\n");
    serial_write_string("[BOOT] Demo tasks created\n");

    // Инициализация шелла
    terminal_writestring("Starting ELF-enabled shell...\n");
    enable_cursor(14, 15);
    terminal_writestring("\n=== MyOS v0.7 ===\n");
    terminal_writestring("Type 'help' for available commands.\n\n");

    shell_ready = 1;
    serial_write_string("[BOOT] Shell ready\n");
    shell_prompt();

    while (1)
    {
        asm volatile("hlt");
    }
}