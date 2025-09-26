// Minimal 64-bit kernel main: text mode VGA init and hello (no libc)
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long  uint64_t;

static volatile uint16_t* const VGA_MEMORY64 = (uint16_t*)0xB8000;
static const uint8_t VGA_WIDTH64 = 80;
static const uint8_t VGA_HEIGHT64 = 25;

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

static inline void putc(char c)
{
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
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

static inline void lidt(void* base, uint16_t size)
{
    struct idt_ptr64 idtp = { size, (uint64_t)base };
    __asm__ __volatile__("lidt %0" : : "m"(idtp));
}

// Timer tick counter
static volatile uint64_t ticks = 0;

void irq0_handler_c(void) { ticks++; }
void irq1_handler_c(void) { /* placeholder: read scancode later */ }

void kernel64_main(void)
{
    // clear screen
    for (uint32_t i = 0, total = (uint32_t)VGA_WIDTH64 * (uint32_t)VGA_HEIGHT64; i < total; i++)
        VGA_MEMORY64[i] = 0x0720;
    cursor_x = 0; cursor_y = 0;

    puts("MyOS x86_64 kernel booted\n");
    puts("Long Mode active.\n");

    // Setup IDT: default handlers, then map IRQ0/IRQ1
    for (int i = 0; i < 256; ++i) idt_set_gate(i, isr_stub_default);
    idt_set_gate(32, irq0_handler);
    idt_set_gate(33, irq1_handler);
    lidt(idt, sizeof(idt) - 1);

    // PIC + PIT
    pic_init();
    pit_init(100);

    // Enable interrupts
    __asm__ __volatile__("sti");

    puts("Interrupts enabled.\n");

    // Halt for now
    for(;;) __asm__ __volatile__("hlt");
}


