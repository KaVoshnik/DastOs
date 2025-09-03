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
    struct task* next;                   // Следующая задача в списке
} task_t;

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

// Состояние клавиатуры
int shift_pressed = 0;
int ctrl_pressed = 0;
int alt_pressed = 0;
int caps_lock_on = 0;

// Предварительные объявления всех функций
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void print_number(uint32_t num);
void terminal_clear(void);
void shell_prompt(void);

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

// Функции для работы с виртуальной памятью (из interrupts.asm)
extern void enable_paging(uint32_t page_directory_addr);
extern void disable_paging(void);
extern uint32_t get_page_fault_address(void);
extern void flush_tlb(void);
extern void page_fault_handler(void);
extern void irq1_handler(void);
extern void exception_handler(void);
extern void idt_flush(void);

// Scancode таблицы для разных режимов
const char scancode_normal[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const char scancode_shift[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,
    'A','S','D','F','G','H','J','K','L',':','"','~',0,'|',
    'Z','X','C','V','B','N','M','<','>','?',0,'*',0,' ',
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
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

// === ФУНКЦИИ ФАЙЛОВОЙ СИСТЕМЫ ===

// Вспомогательная функция для копирования строк
void fs_strcpy(char* dest, const char* src, int max_len) {
    int i = 0;
    while (i < max_len - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Вспомогательная функция для сравнения строк
int fs_strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

// Инициализация файловой системы
void init_filesystem(void) {
    terminal_writestring("Initializing simple file system...\n");
    
    // Выделяем память для блоков данных
    filesystem.data_blocks = (uint8_t*)kmalloc(FS_MAX_BLOCKS * FS_BLOCK_SIZE);
    if (!filesystem.data_blocks) {
        terminal_writestring("ERROR: Failed to allocate memory for filesystem!\n");
        return;
    }
    
    // Инициализация суперблока
    filesystem.superblock.magic = 0xDEADBEEF;
    filesystem.superblock.total_inodes = FS_MAX_FILES;
    filesystem.superblock.free_inodes = FS_MAX_FILES;
    filesystem.superblock.total_blocks = FS_MAX_BLOCKS;
    filesystem.superblock.free_blocks = FS_MAX_BLOCKS;
    filesystem.superblock.block_size = FS_BLOCK_SIZE;
    
    // Очищаем все inode-ы
    for (int i = 0; i < FS_MAX_FILES; i++) {
        filesystem.inodes[i].type = FS_INODE_FREE;
        filesystem.inodes[i].filename[0] = '\0';
        filesystem.inodes[i].size = 0;
        filesystem.inodes[i].created_time = 0;
        filesystem.inodes[i].modified_time = 0;
        for (int j = 0; j < 16; j++) {
            filesystem.inodes[i].blocks[j] = 0;
        }
        filesystem.inode_bitmap[i] = 0; // Свободен
    }
    
    // Очищаем битовую карту блоков
    for (int i = 0; i < FS_MAX_BLOCKS; i++) {
        filesystem.block_bitmap[i] = 0; // Свободен
    }
    
    // Очищаем область данных
    for (int i = 0; i < FS_MAX_BLOCKS * FS_BLOCK_SIZE; i++) {
        filesystem.data_blocks[i] = 0;
    }
    
    filesystem.initialized = 1;
    fs_time_counter = 1; // Начальное время
    
    terminal_writestring("File system initialized successfully!\n");
}

// Поиск свободного inode
int fs_find_free_inode(void) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (filesystem.inodes[i].type == FS_INODE_FREE) {
            return i;
        }
    }
    return -1; // Нет свободных
}

// Поиск свободного блока данных
int fs_find_free_block(void) {
    for (int i = 0; i < FS_MAX_BLOCKS; i++) {
        if (filesystem.block_bitmap[i] == 0) {
            return i;
        }
    }
    return -1; // Нет свободных
}

// Поиск файла по имени
fs_inode_t* fs_find_inode(const char* filename) {
    if (!filesystem.initialized) return NULL;
    
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (filesystem.inodes[i].type == FS_INODE_FILE && 
            fs_strcmp(filesystem.inodes[i].filename, filename) == 0) {
            return &filesystem.inodes[i];
        }
    }
    return NULL;
}

// Проверка существования файла
int fs_file_exists(const char* filename) {
    return fs_find_inode(filename) != NULL;
}

// Создание нового файла
int fs_create_file(const char* filename) {
    if (!filesystem.initialized) {
        terminal_writestring("ERROR: File system not initialized!\n");
        return -1;
    }
    
    // Проверяем, не существует ли файл уже
    if (fs_file_exists(filename)) {
        terminal_writestring("ERROR: File already exists!\n");
        return -1;
    }
    
    // Ищем свободный inode
    int inode_idx = fs_find_free_inode();
    if (inode_idx == -1) {
        terminal_writestring("ERROR: No free inodes available!\n");
        return -1;
    }
    
    // Инициализируем inode
    fs_inode_t* inode = &filesystem.inodes[inode_idx];
    inode->type = FS_INODE_FILE;
    fs_strcpy(inode->filename, filename, FS_MAX_FILENAME);
    inode->size = 0;
    inode->created_time = fs_time_counter++;
    inode->modified_time = inode->created_time;
    
    for (int i = 0; i < 16; i++) {
        inode->blocks[i] = 0;
    }
    
    // Помечаем inode как занятый
    filesystem.inode_bitmap[inode_idx] = 1;
    filesystem.superblock.free_inodes--;
    
    return inode_idx;
}

// Удаление файла
int fs_delete_file(const char* filename) {
    if (!filesystem.initialized) {
        terminal_writestring("ERROR: File system not initialized!\n");
        return -1;
    }
    
    fs_inode_t* inode = fs_find_inode(filename);
    if (!inode) {
        terminal_writestring("ERROR: File not found!\n");
        return -1;
    }
    
    // Освобождаем все блоки данных файла
    for (int i = 0; i < 16; i++) {
        if (inode->blocks[i] != 0) {
            filesystem.block_bitmap[inode->blocks[i]] = 0;
            filesystem.superblock.free_blocks++;
            inode->blocks[i] = 0;
        }
    }
    
    // Освобождаем inode
    inode->type = FS_INODE_FREE;
    inode->filename[0] = '\0';
    inode->size = 0;
    
    // Находим индекс inode-а для освобождения битовой карты
    int inode_idx = inode - filesystem.inodes;
    filesystem.inode_bitmap[inode_idx] = 0;
    filesystem.superblock.free_inodes++;
    
    return 0;
}

// Запись данных в файл
int fs_write_file(const char* filename, const char* data, uint32_t size) {
    if (!filesystem.initialized) {
        terminal_writestring("ERROR: File system not initialized!\n");
        return -1;
    }
    
    if (size > FS_MAX_FILESIZE) {
        terminal_writestring("ERROR: File too large!\n");
        return -1;
    }
    
    fs_inode_t* inode = fs_find_inode(filename);
    if (!inode) {
        terminal_writestring("ERROR: File not found!\n");
        return -1;
    }
    
    // Освобождаем старые блоки, если они есть
    for (int i = 0; i < 16; i++) {
        if (inode->blocks[i] != 0) {
            filesystem.block_bitmap[inode->blocks[i]] = 0;
            filesystem.superblock.free_blocks++;
            inode->blocks[i] = 0;
        }
    }
    
    // Вычисляем необходимое количество блоков
    uint32_t blocks_needed = (size + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
    if (blocks_needed > 16) {
        terminal_writestring("ERROR: File requires too many blocks!\n");
        return -1;
    }
    
    // Выделяем новые блоки
    for (uint32_t i = 0; i < blocks_needed; i++) {
        int block_idx = fs_find_free_block();
        if (block_idx == -1) {
            terminal_writestring("ERROR: No free blocks available!\n");
            return -1;
        }
        
        inode->blocks[i] = block_idx;
        filesystem.block_bitmap[block_idx] = 1;
        filesystem.superblock.free_blocks--;
    }
    
    // Записываем данные в блоки
    for (uint32_t i = 0; i < blocks_needed; i++) {
        uint32_t block_offset = inode->blocks[i] * FS_BLOCK_SIZE;
        uint32_t data_offset = i * FS_BLOCK_SIZE;
        uint32_t bytes_to_copy = FS_BLOCK_SIZE;
        
        if (data_offset + bytes_to_copy > size) {
            bytes_to_copy = size - data_offset;
        }
        
        for (uint32_t j = 0; j < bytes_to_copy; j++) {
            filesystem.data_blocks[block_offset + j] = data[data_offset + j];
        }
    }
    
    inode->size = size;
    inode->modified_time = fs_time_counter++;
    
    return 0;
}

// Чтение данных из файла
int fs_read_file(const char* filename, char* buffer, uint32_t max_size) {
    if (!filesystem.initialized) {
        terminal_writestring("ERROR: File system not initialized!\n");
        return -1;
    }
    
    fs_inode_t* inode = fs_find_inode(filename);
    if (!inode) {
        terminal_writestring("ERROR: File not found!\n");
        return -1;
    }
    
    uint32_t bytes_to_read = inode->size;
    if (bytes_to_read > max_size) {
        bytes_to_read = max_size;
    }
    
    uint32_t blocks_to_read = (bytes_to_read + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
    
    for (uint32_t i = 0; i < blocks_to_read; i++) {
        if (inode->blocks[i] == 0) break;
        
        uint32_t block_offset = inode->blocks[i] * FS_BLOCK_SIZE;
        uint32_t buffer_offset = i * FS_BLOCK_SIZE;
        uint32_t bytes_to_copy = FS_BLOCK_SIZE;
        
        if (buffer_offset + bytes_to_copy > bytes_to_read) {
            bytes_to_copy = bytes_to_read - buffer_offset;
        }
        
        for (uint32_t j = 0; j < bytes_to_copy; j++) {
            buffer[buffer_offset + j] = filesystem.data_blocks[block_offset + j];
        }
    }
    
    return bytes_to_read;
}

// Список файлов
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

// === ФУНКЦИИ ПЛАНИРОВЩИКА ЗАДАЧ ===

// Инициализация планировщика
void init_scheduler(void) {
    current_task = NULL;
    task_list = NULL;
    next_task_id = 1;
    scheduler_ticks = 0;
    terminal_writestring("Task scheduler initialized\n");
}

// Создание новой задачи
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

// Планировщик (простой round-robin)
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
        task_t* prev_task = current_task;
        current_task = next_task;
        switch_to_task(current_task);
    }
}

// Переключение контекста задачи (упрощенная версия)
void switch_to_task(task_t* task) {
    if (!task) return;
    
    // В реальной ОС здесь было бы переключение контекста
    // Пока что просто выводим информацию
    terminal_writestring("Switching to task: ");
    terminal_writestring(task->name);
    terminal_writestring(" (ID: ");
    print_number(task->id);
    terminal_writestring(")\n");
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

// === IDT И ОБРАБОТЧИКИ ПРЕРЫВАНИЙ ===

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void handle_exception(void) {
    terminal_writestring("EXCEPTION occurred! System halted.\n");
    while(1) asm volatile("hlt");
}

// Инициализация PIC
void initialize_system(void) {
    terminal_writestring("Initializing system...\n");
    
    // Инициализация PIC (упрощенная)
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0xFD); // Разрешаем только IRQ1 (клавиатура)
    outb(PIC2_DATA, 0xFF);
    
    terminal_writestring("PIC initialized\n");
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
    terminal_writestring("  vmem       - Virtual memory status\n");
    terminal_writestring("  paging     - Enable/check paging\n");
    terminal_writestring("  keyboard   - Keyboard status\n");
    terminal_writestring("  tasks      - List tasks\n");
    terminal_writestring("  schedule   - Trigger scheduler\n");
    terminal_writestring("  ls         - List files\n");
    terminal_writestring("  touch <f>  - Create file\n");
    terminal_writestring("  cat <f>    - Show file content\n");
    terminal_writestring("  rm <f>     - Delete file\n");
    terminal_writestring("  echo <t> > <f> - Write text to file\n");
    terminal_writestring("  reboot     - Restart system\n");
    terminal_writestring("  poweroff   - Shutdown system\n");
    terminal_writestring("\nKeyboard features: Shift, Ctrl, Alt, Caps Lock support\n");
    terminal_writestring("Special keys: Ctrl+C (cancel), Ctrl+L (clear)\n");
    terminal_writestring("\n");
}

void command_clear(void) {
    terminal_clear();
}

void command_about(void) {
    terminal_writestring("MyOS v0.7 - Enhanced Operating System\n");
    terminal_writestring("====================================\n");
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
    terminal_writestring("\nNew in v0.7:\n");
    terminal_writestring("  - Full keyboard modifier support\n");
    terminal_writestring("  - Proper VGA cursor positioning\n");
    terminal_writestring("  - Shift/Ctrl/Alt key combinations\n");
    terminal_writestring("  - Caps Lock functionality\n");
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

void command_keyboard(void) {
    terminal_writestring("Keyboard Status:\n");
    
    terminal_writestring("  Modifier Keys:\n");
    terminal_writestring("    Shift: ");
    if (shift_pressed) {
        terminal_writestring("PRESSED\n");
    } else {
        terminal_writestring("Released\n");
    }
    
    terminal_writestring("    Ctrl:  ");
    if (ctrl_pressed) {
        terminal_writestring("PRESSED\n");
    } else {
        terminal_writestring("Released\n");
    }
    
    terminal_writestring("    Alt:   ");
    if (alt_pressed) {
        terminal_writestring("PRESSED\n");
    } else {
        terminal_writestring("Released\n");
    }
    
    terminal_writestring("  Caps Lock: ");
    if (caps_lock_on) {
        terminal_writestring("ON\n");
    } else {
        terminal_writestring("OFF\n");
    }
    
    terminal_writestring("\n  Features:\n");
    terminal_writestring("    - Full scancode support\n");
    terminal_writestring("    - Shift for uppercase and symbols\n");
    terminal_writestring("    - Caps Lock toggle\n");
    terminal_writestring("    - Ctrl+C to cancel commands\n");
    terminal_writestring("    - Ctrl+L to clear screen\n");
    terminal_writestring("\n  Try typing with different modifiers!\n\n");
}

// === КОМАНДЫ ПЛАНИРОВЩИКА ===

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

// === КОМАНДЫ ФАЙЛОВОЙ СИСТЕМЫ ===

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
    } else if (strcmp(cmd, "vmem") == 0) {
        command_vmem();
    } else if (strcmp(cmd, "paging") == 0) {
        command_paging();
    } else if (strcmp(cmd, "keyboard") == 0) {
        command_keyboard();
    } else if (strcmp(cmd, "tasks") == 0) {
        command_tasks();
    } else if (strcmp(cmd, "schedule") == 0) {
        command_schedule();
    } else if (strcmp(cmd, "ls") == 0) {
        command_ls();
    } else if (strcmp(cmd, "touch") == 0) {
        command_touch(args[0] ? args : NULL);
    } else if (strcmp(cmd, "cat") == 0) {
        command_cat(args[0] ? args : NULL);
    } else if (strcmp(cmd, "rm") == 0) {
        command_rm(args[0] ? args : NULL);
    } else if (strcmp(cmd, "echo") == 0) {
        command_echo(args[0] ? args : NULL);
    } else if (strcmp(cmd, "reboot") == 0) {
        command_reboot();
    } else if (strcmp(cmd, "poweroff") == 0) {
        command_poweroff();
    } else {
        terminal_writestring("Unknown command: ");
        terminal_writestring(cmd);
        terminal_writestring("\n");
        terminal_writestring("Type 'help' for available commands.\n");
    }
}

// Получение символа с учетом модификаторов
char get_ascii_char(uint8_t scancode) {
    if (scancode >= 128) return 0;
    
    char base_char;
    int use_shift = shift_pressed;
    
    // Проверяем Caps Lock для букв
    if (caps_lock_on && scancode >= 16 && scancode <= 50) {
        char c = scancode_normal[scancode];
        if (c >= 'a' && c <= 'z') {
            use_shift = !use_shift; // Инвертируем shift для букв
        }
    }
    
    if (use_shift) {
        base_char = scancode_shift[scancode];
    } else {
        base_char = scancode_normal[scancode];
    }
    
    return base_char;
}

// Обработка модификаторов
void handle_modifier_keys(uint8_t scancode) {
    switch (scancode) {
        case KEY_LSHIFT_PRESSED:
        case KEY_RSHIFT_PRESSED:
            shift_pressed = 1;
            break;
        case KEY_LSHIFT_RELEASED:
        case KEY_RSHIFT_RELEASED:
            shift_pressed = 0;
            break;
        case KEY_CTRL_PRESSED:
            ctrl_pressed = 1;
            break;
        case KEY_CTRL_RELEASED:
            ctrl_pressed = 0;
            break;
        case KEY_ALT_PRESSED:
            alt_pressed = 1;
            break;
        case KEY_ALT_RELEASED:
            alt_pressed = 0;
            break;
        case KEY_CAPS_LOCK:
            caps_lock_on = !caps_lock_on;
            break;
    }
}

// Обработчик клавиатуры с полной поддержкой
void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Обрабатываем модификаторы
    handle_modifier_keys(scancode);
    
    // Проверяем, что это нажатие клавиши (не отпускание)
    if (!(scancode & 0x80)) {
        char ascii = get_ascii_char(scancode);
        
        if (ascii) {
            // Обработка специальных комбинаций
            if (ctrl_pressed) {
                switch (ascii) {
                    case 'c':
                    case 'C':
                        // Ctrl+C - прервать команду
                        terminal_writestring("\n^C\n");
                        command_length = 0;
                        if (shell_ready) shell_prompt();
                        break;
                    case 'l':
                    case 'L':
                        // Ctrl+L - очистить экран
                        command_clear();
                        if (shell_ready) shell_prompt();
                        break;
                }
            } else if (ascii == '\n' || ascii == '\r') { // Enter
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
    
    // Включаем VGA курсор
    enable_cursor(14, 15); // Обычный курсор
    
    terminal_writestring("MyOS v0.7 - Advanced Operating System!\n");
    terminal_writestring("=====================================\n\n");

    // Инициализация управления памятью
    init_memory_management();
    terminal_writestring("Memory management initialized\n");
    
    // Инициализация виртуальной памяти
    terminal_writestring("Initializing virtual memory system...\n");
    init_virtual_memory();
    enable_virtual_memory();
    terminal_writestring("Virtual memory system ready\n");
    
    // Инициализация файловой системы
    init_filesystem();
    terminal_writestring("File system ready\n");
    
    // Инициализация планировщика задач
    init_scheduler();

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

    // Инициализация системы для работы с GRUB
    initialize_system();

    // Создание демонстрационных задач
    create_task("idle", idle_task, 255);
    create_task("demo1", demo_task1, 10);
    create_task("demo2", demo_task2, 20);
    terminal_writestring("Demo tasks created\n");

    // Включаем прерывания
    asm volatile("sti");

    // Инициализация шелла
    command_clear();
    enable_cursor(14, 15); // Повторно включаем курсор после очистки
    terminal_writestring("Welcome to MyOS v0.7!\n");
    terminal_writestring("Features: Enhanced keyboard, VGA cursor, Memory management, Virtual memory\n");
    terminal_writestring("New: Full keyboard support with Shift, Ctrl, Alt, Caps Lock\n");
    terminal_writestring("Type 'help' for available commands.\n\n");
    
    shell_ready = 1;
    shell_prompt();

    while (1) asm volatile("hlt");
}