// Minimal 64-bit kernel main: text mode VGA init and hello (no libc)
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long  uint64_t;
// ---------------- Simple tasks (for ps command) ----------------
typedef struct {
    uint32_t id;
    char name[32];
    uint8_t running;
} task64_t;

#define MAX_TASKS64 8
static task64_t tasks64[MAX_TASKS64];
static uint32_t next_task_id64 = 1;

static task64_t* task64_create(const char *name)
{
    for (int i = 0; i < MAX_TASKS64; ++i) {
        if (tasks64[i].id == 0) {
            tasks64[i].id = next_task_id64++;
            strncpy(tasks64[i].name, name ? name : "task", sizeof(tasks64[i].name)-1);
            tasks64[i].running = 1;
            return &tasks64[i];
        }
    }
    return 0;
}

static void task64_list(void)
{
    terminal_writestring64("Process List:\n");
    for (int i = 0; i < MAX_TASKS64; ++i) {
        if (tasks64[i].id) {
            terminal_writestring64("  ");
            // print id
            // simple decimal print
            uint32_t id = tasks64[i].id;
            char buf[12]; int p = 0; if (id == 0) buf[p++]='0';
            char tmp[12]; int q=0; while (id){ tmp[q++] = (char)('0' + (id%10)); id/=10; }
            while (q) buf[p++] = tmp[--q]; buf[p]='\0';
            terminal_writestring64(buf);
            terminal_writestring64("  ");
            terminal_writestring64(tasks64[i].name);
            terminal_writestring64(tasks64[i].running?"  running\n":"  stopped\n");
        }
    }
}

static volatile uint16_t* const VGA_MEMORY64 = (uint16_t*)0xB8000;
static const uint8_t VGA_WIDTH64 = 80;
static const uint8_t VGA_HEIGHT64 = 25;

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

// ---------------- Minimal libc ----------------
static void *memset(void *dst, int c, uint64_t n)
{
    unsigned char *p = (unsigned char*)dst;
    for (uint64_t i = 0; i < n; ++i) p[i] = (unsigned char)c;
    return dst;
}

static void *memcpy(void *dst, const void *src, uint64_t n)
{
    unsigned char *d = (unsigned char*)dst;
    const unsigned char *s = (const unsigned char*)src;
    for (uint64_t i = 0; i < n; ++i) d[i] = s[i];
    return dst;
}

static uint64_t strlen(const char *s)
{
    uint64_t n = 0; while (s[n]) ++n; return n;
}

static int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) { ++a; ++b; }
    return (unsigned char)*a - (unsigned char)*b;
}

static char *strcpy(char *dst, const char *src)
{
    char *d = dst; while ((*d++ = *src++)) {} return dst;
}

static char *strncpy(char *dst, const char *src, uint64_t n)
{
    uint64_t i = 0; for (; i < n && src[i]; ++i) dst[i] = src[i];
    for (; i < n; ++i) dst[i] = '\0';
    return dst;
}

static inline void putc(char c)
{
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            VGA_MEMORY64[cursor_y * VGA_WIDTH64 + cursor_x] = 0x0720;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH64 - 1;
            VGA_MEMORY64[cursor_y * VGA_WIDTH64 + cursor_x] = 0x0720;
        }
    } else {
        uint16_t entry = (uint16_t)c | (uint16_t)(0x07 << 8);
        VGA_MEMORY64[cursor_y * VGA_WIDTH64 + cursor_x] = entry;
        cursor_x++;
        if (cursor_x >= VGA_WIDTH64) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    if (cursor_y >= VGA_HEIGHT64) {
        // scroll
        uint32_t row_stride = (uint32_t)VGA_WIDTH64;
        uint32_t vis_rows = (uint32_t)VGA_HEIGHT64 - 1u;
        uint32_t move_count = vis_rows * row_stride;
        for (uint32_t i = 0; i < move_count; i++)
            ((volatile uint16_t*)VGA_MEMORY64)[i] = ((volatile uint16_t*)VGA_MEMORY64)[i + row_stride];
        uint32_t total_cells = (uint32_t)VGA_HEIGHT64 * row_stride;
        for (uint32_t i = move_count; i < total_cells; i++)
            ((volatile uint16_t*)VGA_MEMORY64)[i] = 0x0720;
        cursor_y = VGA_HEIGHT64 - 1;
    }
}

static void puts(const char* s)
{
    while (*s) putc(*s++);
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// ---------------- Serial (COM1) debug ----------------
static void serial_init(void)
{
    // Disable interrupts
    outb(0x3F8 + 1, 0x00);
    // Enable DLAB
    outb(0x3F8 + 3, 0x80);
    // Set baud to 115200/3 = 38400 (divisor = 3)
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    // 8 bits, no parity, one stop bit
    outb(0x3F8 + 3, 0x03);
    // Enable FIFO, clear them, 14-byte threshold
    outb(0x3F8 + 2, 0xC7);
    // IRQs disabled, RTS/DSR set
    outb(0x3F8 + 4, 0x0B);
}

static int serial_is_transmit_empty(void)
{
    return inb(0x3F8 + 5) & 0x20;
}

static void serial_write_char(char c)
{
    while (!serial_is_transmit_empty()) { }
    outb(0x3F8, (uint8_t)c);
}

static void serial_write_string(const char* s)
{
    while (*s) {
        char c = *s++;
        if (c == '\n') serial_write_char('\r');
        serial_write_char(c);
    }
}

// PIC remap and unmask timer(IRQ0) and keyboard(IRQ1)
static void pic_init(void)
{
    outb(0x20, 0x11); // ICW1
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // ICW2: Master offset 0x20
    outb(0xA1, 0x28); // ICW2: Slave offset 0x28
    outb(0x21, 0x04); // ICW3
    outb(0xA1, 0x02); // ICW3
    outb(0x21, 0x01); // ICW4
    outb(0xA1, 0x01); // ICW4
    // Unmask IRQ0, IRQ1; mask others
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);
}

static void pit_init(uint32_t hz)
{
    uint32_t divisor = 1193182u / (hz ? hz : 100u);
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

// IDT structures (x86_64)
struct idt_entry64 {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_ptr64 {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry64 idt[256];

extern void irq0_handler(void);
extern void irq1_handler(void);
extern void isr_stub_default(void);
extern void syscall_handler(void);
extern void switch_to_user_mode64(uint64_t user_rsp, uint64_t user_rip);
void syscall_exit_to_kernel(void);

static void idt_set_gate(int n, void* handler)
{
    uint64_t h = (uint64_t)handler;
    idt[n].offset_low  = (uint16_t)(h & 0xFFFF);
    idt[n].selector    = 0x08;
    idt[n].ist         = 0;
    idt[n].type_attr   = 0x8E; // present, DPL=0, interrupt gate
    idt[n].offset_mid  = (uint16_t)((h >> 16) & 0xFFFF);
    idt[n].offset_high = (uint32_t)((h >> 32) & 0xFFFFFFFF);
    idt[n].zero        = 0;
}

static void idt_set_gate_dpl3(int n, void* handler)
{
    uint64_t h = (uint64_t)handler;
    idt[n].offset_low  = (uint16_t)(h & 0xFFFF);
    idt[n].selector    = 0x08;
    idt[n].ist         = 0;
    idt[n].type_attr   = 0xEE; // present, DPL=3, interrupt gate (0x8E | 0x60)
    idt[n].offset_mid  = (uint16_t)((h >> 16) & 0xFFFF);
    idt[n].offset_high = (uint32_t)((h >> 32) & 0xFFFFFFFF);
    idt[n].zero        = 0;
}

static inline void lidt(void* base, uint16_t size)
{
    struct idt_ptr64 idtp = { size, (uint64_t)base };
    __asm__ __volatile__("lidt %0" : : "m"(idtp));
}

// 64-bit syscall handler (C side)
uint64_t handle_syscall64(uint64_t num, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3)
{
    // Syscall numbers
    // 0: write_console(char* ptr, size)
    // 1: get_ticks() -> uint64
    // 2: yield()
    // 3: reboot()
    // 4: exit(status)
    switch (num) {
        case 0: {
            const char* p = (const char*)a0;
            uint64_t n = a1;
            for (uint64_t i = 0; i < n && p[i]; ++i) {
                char c = p[i];
                putc(c);
                serial_write_char(c);
            }
            return n;
        }
        case 1:
            return ticks;
        case 2:
            __asm__ __volatile__("hlt");
            return 0;
        case 3:
            outb(0x64, 0xFE);
            for(;;) { __asm__ __volatile__("hlt"); }
        case 4:
            // Mark exit flag; return to kernel shell from ISR stub
            extern volatile uint8_t proc_exit_flag;
            proc_exit_flag = 1;
            return 0;
        default:
            return (uint64_t)-1;
    }
}

volatile uint8_t proc_exit_flag = 0;

void syscall_exit_to_kernel(void)
{
    // After a user program calls exit, we land here (interrupt context unwound)
    terminal_writestring64("\n[ user exited ]\n");
    shell_prompt64();
}

// ---------------- Simple user-mode demo ----------------
static uint8_t user_stack[4096] __attribute__((aligned(16)));

static void user_entry64(void)
{
    // Prepare message and use syscall 0 (write_console)
    const char *msg = "[user] hello from ring3!\n";
    uint64_t len = 24;
    __asm__ __volatile__(
        "mov $0, %%rax\n\t"      /* syscall num */
        "mov %0, %%rdi\n\t"      /* arg0 ptr */
        "mov %1, %%rsi\n\t"      /* arg1 len */
        "int $0x80\n\t"
        :
        : "r"(msg), "r"(len)
        : "rax","rdi","rsi","memory");

    // Query ticks (syscall 1)
    uint64_t t;
    __asm__ __volatile__(
        "mov $1, %%rax\n\t"
        "int $0x80\n\t"
        : "=a"(t)
        :
        : "memory");

    (void)t; // not printed to keep demo simple

    // Loop forever
    for(;;) { __asm__ __volatile__("hlt"); }
}

// Timer tick counter
static volatile uint64_t ticks = 0;

void irq0_handler_c(void) { ticks++; }

// ---------------- Keyboard (Set 1) minimal handler ----------------
static int kbd_shift = 0; // bit0: left/right shift active
static char command_buffer[256];
static int command_length = 0;
static char working_dir[64] = "/";

static const char scancode_to_ascii[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'', '`',  0,  '\\',
    'z','x','c','v','b','n','m',',','.','/',  0,   '*',  0,  ' ',
    0,  0,   0,  0,  0,  0,  0,  0,  0,  0,   0,   0,   0,   0,  0,  0,
    0,  0,   0,  0,  0,  0,  0,  0,  0,  0,   0,   0,   0,   0,  0,  0,
    0,  0,   0,  0,  0,  0,  0,  0,  0,  0,   0,   0,   0,   0,  0,  0,
    0,  0,   0,  0,  0,  0,  0,  0
};

static char to_upper(char c)
{
    if (c >= 'a' && c <= 'z') return (char)(c - 'a' + 'A');
    if (c == '`') return '~';
    if (c == '-') return '_';
    if (c == '=') return '+';
    if (c == '[') return '{';
    if (c == ']') return '}';
    if (c == '\\') return '|';
    if (c == ';') return ':';
    if (c == '\'') return '"';
    if (c == ',') return '<';
    if (c == '.') return '>';
    if (c == '/') return '?';
    if (c >= '0' && c <= '9') {
        static const char num_shift[] = {')','!','@','#','$','%','^','&','*','('};
        return num_shift[c - '0'];
    }
    return c;
}

// ---------------- Simple RAM Filesystem (like 32-bit) ----------------
#define FS_MAX_FILES 64
#define FS_MAX_FILENAME 32
#define FS_MAX_FILESIZE 1024
#define FS_BLOCK_SIZE 64
#define FS_MAX_BLOCKS 256

#define FS_INODE_FREE 0
#define FS_INODE_FILE 1

typedef struct {
    uint32_t magic;
    uint32_t total_inodes;
    uint32_t free_inodes;
    uint32_t total_blocks;
    uint32_t free_blocks;
    uint32_t block_size;
} fs_superblock_t;

typedef struct {
    uint8_t type;
    char filename[FS_MAX_FILENAME];
    uint32_t size;
    uint32_t blocks[4];
    uint32_t created_time;
    uint32_t modified_time;
} fs_inode_t;

typedef struct {
    uint8_t initialized;
    fs_superblock_t superblock;
    fs_inode_t inodes[FS_MAX_FILES];
    uint8_t inode_bitmap[FS_MAX_FILES];
    uint8_t block_bitmap[FS_MAX_BLOCKS];
    uint8_t data_blocks[FS_MAX_BLOCKS * FS_BLOCK_SIZE];
} fs_state_t;

static fs_state_t filesystem;
static uint32_t fs_time_counter = 1;

static void fs_list_files(void)
{
    puts("Files:\n");
    for (int i = 0; i < FS_MAX_FILES; ++i) {
        if (filesystem.inode_bitmap[i] && filesystem.inodes[i].type == FS_INODE_FILE) {
            puts(" - "); puts(filesystem.inodes[i].filename); puts("\n");
        }
    }
}

static void init_filesystem(void)
{
    memset(&filesystem, 0, sizeof(filesystem));
    filesystem.superblock.magic = 0x12345678;
    filesystem.superblock.total_inodes = FS_MAX_FILES;
    filesystem.superblock.free_inodes = FS_MAX_FILES;
    filesystem.superblock.total_blocks = FS_MAX_BLOCKS;
    filesystem.superblock.free_blocks = FS_MAX_BLOCKS;
    filesystem.superblock.block_size = FS_BLOCK_SIZE;
    puts("Filesystem initialized\n");
}

static int fs_file_exists(const char *filename)
{
    for (int i = 0; i < FS_MAX_FILES; ++i) {
        if (filesystem.inode_bitmap[i] &&
            filesystem.inodes[i].type == FS_INODE_FILE &&
            strcmp(filesystem.inodes[i].filename, filename) == 0) return 1;
    }
    return 0;
}

static fs_inode_t* fs_find_inode(const char *filename)
{
    for (int i = 0; i < FS_MAX_FILES; ++i) {
        if (filesystem.inode_bitmap[i] &&
            filesystem.inodes[i].type == FS_INODE_FILE &&
            strcmp(filesystem.inodes[i].filename, filename) == 0) return &filesystem.inodes[i];
    }
    return 0;
}

static int fs_create_file(const char *filename)
{
    if (!filename || !*filename) return -1;
    if (fs_file_exists(filename)) return -1;
    for (int i = 0; i < FS_MAX_FILES; ++i) {
        if (!filesystem.inode_bitmap[i]) {
            filesystem.inode_bitmap[i] = 1;
            filesystem.superblock.free_inodes--;
            fs_inode_t *inode = &filesystem.inodes[i];
            memset(inode, 0, sizeof(*inode));
            inode->type = FS_INODE_FILE;
            strncpy(inode->filename, filename, FS_MAX_FILENAME - 1);
            inode->created_time = inode->modified_time = fs_time_counter++;
            return i;
        }
    }
    return -1;
}

static int fs_delete_file(const char *filename)
{
    for (int i = 0; i < FS_MAX_FILES; ++i) {
        if (filesystem.inode_bitmap[i] && strcmp(filesystem.inodes[i].filename, filename) == 0) {
            // free first block if any
            if (filesystem.inodes[i].blocks[0]) {
                uint32_t b = filesystem.inodes[i].blocks[0];
                if (b < FS_MAX_BLOCKS) { filesystem.block_bitmap[b] = 0; filesystem.superblock.free_blocks++; }
            }
            filesystem.inode_bitmap[i] = 0;
            filesystem.superblock.free_inodes++;
            return 0;
        }
    }
    return -1;
}

static int fs_write_file(const char *filename, const char *data, uint32_t size)
{
    fs_inode_t *inode = fs_find_inode(filename);
    if (!inode || !data || size == 0) return -1;
    if (size > FS_MAX_FILESIZE) size = FS_MAX_FILESIZE;
    if (inode->blocks[0] == 0) {
        for (uint32_t i = 0; i < FS_MAX_BLOCKS; ++i) {
            if (filesystem.block_bitmap[i] == 0) {
                filesystem.block_bitmap[i] = 1;
                filesystem.superblock.free_blocks--;
                inode->blocks[0] = i;
                break;
            }
        }
    }
    if (inode->blocks[0] > 0) {
        uint8_t *blk = filesystem.data_blocks + inode->blocks[0] * FS_BLOCK_SIZE;
        memcpy(blk, data, size);
        inode->size = size;
        inode->modified_time = fs_time_counter++;
        return 0;
    }
    return -1;
}

static int fs_read_file(const char *filename, char *buffer, uint32_t max_size)
{
    fs_inode_t *inode = fs_find_inode(filename);
    if (!inode || !buffer || inode->size == 0) return -1;
    uint32_t n = inode->size < max_size ? inode->size : max_size;
    if (inode->blocks[0] > 0) {
        uint8_t *blk = filesystem.data_blocks + inode->blocks[0] * FS_BLOCK_SIZE;
        memcpy(buffer, blk, n);
        return (int)n;
    }
    return -1;
}

static int fs_append_file(const char *filename, const char *data, uint32_t size)
{
    fs_inode_t *inode = fs_find_inode(filename);
    if (!inode || !data || size == 0) return -1;
    if (size > FS_MAX_FILESIZE) size = FS_MAX_FILESIZE;
    if (inode->blocks[0] == 0) {
        // allocate first block
        for (uint32_t i = 0; i < FS_MAX_BLOCKS; ++i) {
            if (filesystem.block_bitmap[i] == 0) {
                filesystem.block_bitmap[i] = 1;
                filesystem.superblock.free_blocks--;
                inode->blocks[0] = i;
                break;
            }
        }
    }
    if (inode->blocks[0] > 0) {
        uint8_t *blk = filesystem.data_blocks + inode->blocks[0] * FS_BLOCK_SIZE;
        uint32_t offset = inode->size;
        uint32_t room = FS_MAX_FILESIZE - offset;
        uint32_t n = size < room ? size : room;
        memcpy(blk + offset, data, n);
        inode->size = offset + n;
        inode->modified_time = fs_time_counter++;
        return (int)n;
    }
    return -1;
}

// ---------------- Minimal ELF64 loader (no VM, identity mapping assumed) ----------------
typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

#define PT_LOAD 1

static int elf64_validate(const uint8_t *data, uint64_t size)
{
    if (size < sizeof(Elf64_Ehdr)) return 0;
    const Elf64_Ehdr *h = (const Elf64_Ehdr*)data;
    if (h->e_ident[0] != 0x7F || h->e_ident[1] != 'E' || h->e_ident[2] != 'L' || h->e_ident[3] != 'F') return 0;
    if (h->e_ident[4] != 2) return 0; // 64-bit
    return 1;
}

static uint64_t elf64_load_and_get_entry(uint8_t *data, uint64_t size)
{
    if (!elf64_validate(data, size)) return 0;
    Elf64_Ehdr *eh = (Elf64_Ehdr*)data;
    if (eh->e_phoff == 0 || eh->e_phnum == 0) return eh->e_entry;
    for (uint16_t i = 0; i < eh->e_phnum; ++i) {
        Elf64_Phdr *ph = (Elf64_Phdr*)(data + eh->e_phoff + (uint64_t)i * eh->e_phentsize);
        if (ph->p_type != PT_LOAD) continue;
        // naive copy to p_vaddr (assumes identity map and writable)
        if (ph->p_filesz) memcpy((void*)(uint64_t)ph->p_vaddr, data + ph->p_offset, ph->p_filesz);
        if (ph->p_memsz > ph->p_filesz) memset((void*)(uint64_t)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
    }
    return eh->e_entry;
}

void irq1_handler_c(void)
{
    uint8_t sc = inb(0x60);

    if (sc == 0xE0) {
        // extended scancode prefix, ignore next for simplicity now
        return;
    }

    if (sc == 0x2A || sc == 0x36) { // Shift press
        kbd_shift = 1;
        return;
    }
    if (sc == 0xAA || sc == 0xB6) { // Shift release
        kbd_shift = 0;
        return;
    }

    if (sc & 0x80) {
        // key release for other keys – ignore
        return;
    }

    char c = 0;
    if (sc < 128) c = scancode_to_ascii[sc];
    if (c) {
        if (c == '\n') {
            serial_write_string("\r\n");
            putc('\n');
            command_buffer[command_length] = '\0';
            // execute
            extern void execute_command64(const char *cmd);
            execute_command64(command_buffer);
            command_length = 0;
            extern void shell_prompt64(void);
            shell_prompt64();
            return;
        }
        if (kbd_shift) c = to_upper(c);
        if (c == '\b') {
            if (command_length > 0) {
                command_length--;
                putc('\b');
                serial_write_char('\b');
            }
        } else {
            if (command_length < (int)sizeof(command_buffer) - 1) {
                command_buffer[command_length++] = c;
                putc(c);
                serial_write_char(c);
            }
        }
    }
}

static void terminal_writestring64(const char *s)
{
    while (*s) putc(*s++);
}

static void print_uint64_dec(uint64_t v)
{
    char buf[32]; int p = 0; if (v == 0) buf[p++]='0';
    char tmp[32]; int q=0; while (v){ tmp[q++] = (char)('0' + (v%10)); v/=10; }
    while (q) buf[p++] = tmp[--q]; buf[p]='\0';
    terminal_writestring64(buf);
}

static void print_uint64_hex(uint64_t v)
{
    terminal_writestring64("0x");
    for (int i = 15; i >= 0; --i) {
        uint8_t nib = (v >> (i*4)) & 0xF;
        char c = (nib < 10) ? ('0' + nib) : ('A' + (nib-10));
        putc(c);
    }
}

static void shell_prompt64(void)
{
    terminal_writestring64("myos64> ");
}

static void command_help64(void)
{
    terminal_writestring64("Available commands:\n");
    terminal_writestring64("  help        - Show this help\n");
    terminal_writestring64("  ls          - List files\n");
    terminal_writestring64("  touch NAME  - Create file\n");
    terminal_writestring64("  rm NAME     - Delete file\n");
    terminal_writestring64("  cat NAME    - Print file\n");
    terminal_writestring64("  echo TEXT>NAME - Write text to file\n");
    terminal_writestring64("  echo TEXT>>NAME - Append text to file\n");
    terminal_writestring64("  reboot      - Reboot\n");
    terminal_writestring64("  poweroff    - Power off\n");
    terminal_writestring64("  ps          - Show tasks\n");
    terminal_writestring64("  uptime      - Seconds since boot\n");
    terminal_writestring64("  clear       - Clear screen\n");
    terminal_writestring64("  pwd         - Print working directory\n");
    terminal_writestring64("  cd DIR      - Change directory (stub)\n");
    terminal_writestring64("  hexdump NAME- Hex view of file\n");
    terminal_writestring64("  run NAME    - Load and run ELF64\n");
    terminal_writestring64("  testelf     - Run test.elf if exists\n");
}

static void execute_command64(const char *command)
{
    // parse command and args
    char cmd[64];
    char args[192];
    int i = 0, j = 0;
    while (command[i] && command[i] != ' ' && i < 63) { cmd[i] = command[i]; i++; }
    cmd[i] = '\0';
    while (command[i] == ' ') i++;
    while (command[i] && j < 191) { args[j++] = command[i++]; }
    args[j] = '\0';

    if (strcmp(cmd, "help") == 0) { command_help64(); return; }
    if (strcmp(cmd, "ls") == 0) { fs_list_files(); return; }
    if (strcmp(cmd, "touch") == 0) { if (*args) fs_create_file(args); return; }
    if (strcmp(cmd, "rm") == 0) { if (*args) fs_delete_file(args); return; }
    if (strcmp(cmd, "cat") == 0) {
        if (*args) {
            char buf[FS_MAX_FILESIZE+1];
            int n = fs_read_file(args, buf, FS_MAX_FILESIZE);
            if (n > 0) { buf[n] = '\0'; terminal_writestring64(buf); }
        }
        return;
    }
    if (strcmp(cmd, "echo") == 0) {
        // format: echo TEXT>NAME
        char *gt = 0; char *gt2 = 0; for (int k = 0; args[k]; ++k) { if (args[k] == '>' && args[k+1] != '>') { gt = (char*)&args[k]; break; } if (args[k] == '>' && args[k+1] == '>') { gt2 = (char*)&args[k]; break; } }
        if (gt2) {
            *gt2 = '\0';
            const char *text = args;
            const char *name = gt2 + 2;
            while (*name == ' ') name++;
            uint64_t len = strlen(text);
            if (!fs_file_exists(name)) fs_create_file(name);
            fs_append_file(name, text, (uint32_t)len);
        } else if (gt) {
            *gt = '\0';
            const char *text = args;
            const char *name = gt + 1;
            while (*name == ' ') name++;
            uint64_t len = strlen(text);
            if (!fs_file_exists(name)) fs_create_file(name);
            fs_write_file(name, text, (uint32_t)len);
        }
        return;
    }
    if (strcmp(cmd, "reboot") == 0) { outb(0x64, 0xFE); for(;;) __asm__ __volatile__("hlt"); }
    if (strcmp(cmd, "poweroff") == 0) {
        outb(0x604, 0x20); outb(0x605, 0x00);
        outb(0xB004, 0x20); outb(0xB005, 0x00);
        for(;;) { __asm__ __volatile__("cli; hlt"); }
    }

    if (strcmp(cmd, "ps") == 0) { task64_list(); return; }

    if (strcmp(cmd, "uptime") == 0) {
        // ticks at 100 Hz
        uint64_t seconds = ticks / 100ull;
        terminal_writestring64("uptime: "); print_uint64_dec(seconds); terminal_writestring64("s\n");
        return;
    }
    if (strcmp(cmd, "clear") == 0) {
        for (uint32_t i = 0, total = (uint32_t)VGA_WIDTH64 * (uint32_t)VGA_HEIGHT64; i < total; i++)
            VGA_MEMORY64[i] = 0x0720; cursor_x=0; cursor_y=0; shell_prompt64(); return;
    }
    if (strcmp(cmd, "pwd") == 0) {
        terminal_writestring64(working_dir);
        terminal_writestring64("\n");
        return;
    }
    if (strcmp(cmd, "cd") == 0) {
        // Заглушка: поддерживаем только корень
        if (*args == '\0' || (args[0] == '/' && args[1] == '\0')) {
            strncpy(working_dir, "/", sizeof(working_dir)-1);
        } else {
            terminal_writestring64("Only root '/' supported for now\n");
        }
        return;
    }

    if (strcmp(cmd, "hexdump") == 0) {
        if (*args) {
            char *buf = (char*)0x210000;
            int n = fs_read_file(args, buf, 1<<20);
            if (n > 0) {
                for (int i = 0; i < n; i += 16) {
                    print_uint64_hex((uint64_t)i);
                    terminal_writestring64(": ");
                    for (int j = 0; j < 16 && i+j < n; ++j) {
                        uint8_t b = (uint8_t)buf[i+j];
                        char hi = (b>>4) & 0xF; hi = hi<10?('0'+hi):('A'+hi-10);
                        char lo = b & 0xF; lo = lo<10?('0'+lo):('A'+lo-10);
                        putc(hi); putc(lo); putc(' ');
                    }
                    putc('\n');
                }
            } else {
                terminal_writestring64("file read failed\n");
            }
        }
        return;
    }

    if (strcmp(cmd, "sleep") == 0) {
        // sleep N seconds based on PIT @100Hz
        uint64_t n = 0;
        for (int k = 0; args[k]; ++k) { if (args[k] >= '0' && args[k] <= '9') { n = n*10 + (args[k]-'0'); } else break; }
        uint64_t target = ticks + n * 100ull;
        while (ticks < target) { __asm__ __volatile__("hlt"); }
        return;
    }

    if (strcmp(cmd, "yield") == 0) {
        __asm__ __volatile__(
            "mov $2, %rax\n\t"
            "int $0x80\n\t"
            : : : "rax", "memory");
        return;
    }

    if (strcmp(cmd, "testelf") == 0) {
        if (fs_file_exists("test.elf")) {
            terminal_writestring64("Running test.elf...\n");
            char *buf = (char*)0x200000;
            int n = fs_read_file("test.elf", buf, 1<<20);
            if (n > 0) {
                uint64_t entry = elf64_load_and_get_entry((uint8_t*)buf, (uint64_t)n);
                if (entry) {
                    terminal_writestring64("ELF64 loaded. Entry: "); print_uint64_hex(entry); terminal_writestring64("\nJumping to user...\n");
                    static uint8_t elf_user_stack[8192] __attribute__((aligned(16)));
                    uint64_t usp = (uint64_t)elf_user_stack + sizeof(elf_user_stack) - 16;
                    switch_to_user_mode64(usp, entry);
                } else {
                    terminal_writestring64("ELF64 parse/load failed\n");
                }
            } else {
                terminal_writestring64("file read failed\n");
            }
        } else {
            terminal_writestring64("test.elf not found\n");
        }
        return;
    }

    if (strcmp(cmd, "run") == 0) {
        if (*args) {
            // load file from FS into temp buffer
            char *buf = (char*)0x200000; // temporary fixed buffer
            int n = fs_read_file(args, buf, 1<<20);
            if (n > 0) {
                uint64_t entry = elf64_load_and_get_entry((uint8_t*)buf, (uint64_t)n);
                if (entry) {
                    terminal_writestring64("ELF64 loaded. Entry: "); print_uint64_hex(entry); terminal_writestring64("\nJumping to user...\n");
                    static uint8_t elf_user_stack[8192] __attribute__((aligned(16)));
                    uint64_t usp = (uint64_t)elf_user_stack + sizeof(elf_user_stack) - 16;
                    switch_to_user_mode64(usp, entry);
                } else {
                    terminal_writestring64("ELF64 parse/load failed\n");
                }
            } else {
                terminal_writestring64("file read failed\n");
            }
        }
        return;
    }
}

void kernel64_main(void)
{
    // clear screen
    for (uint32_t i = 0, total = (uint32_t)VGA_WIDTH64 * (uint32_t)VGA_HEIGHT64; i < total; i++)
        VGA_MEMORY64[i] = 0x0720;
    cursor_x = 0; cursor_y = 0;

    serial_init();
    puts("MyOS x86_64 kernel booted\n");
    puts("Long Mode active.\n");
    serial_write_string("MyOS x86_64 kernel booted\r\n");
    serial_write_string("Long Mode active.\r\n");

    // Setup IDT: default handlers, then map IRQ0/IRQ1 and INT 0x80
    for (int i = 0; i < 256; ++i) idt_set_gate(i, isr_stub_default);
    idt_set_gate(32, irq0_handler);
    idt_set_gate(33, irq1_handler);
    // INT 0x80 available from user-mode
    idt_set_gate_dpl3(128, syscall_handler);
    lidt(idt, sizeof(idt) - 1);

    // PIC + PIT
    pic_init();
    pit_init(100);

    // Enable interrupts
    __asm__ __volatile__("sti");

    puts("Interrupts enabled.\n");
    serial_write_string("Interrupts enabled.\r\n");

    // Initialize simple RAM FS and create a demo file
    init_filesystem();
    fs_create_file("hello.txt");
    fs_write_file("hello.txt", "Hello from FS on x86_64!\n", 27);

    // Register a couple of tasks for ps output
    task64_create("init");
    task64_create("shell");

    // Start shell on kernel console
    shell_prompt64();
    for(;;) { __asm__ __volatile__("hlt"); }
}


