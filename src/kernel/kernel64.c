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

void kernel64_main(void)
{
    // clear screen
    for (uint32_t i = 0, total = (uint32_t)VGA_WIDTH64 * (uint32_t)VGA_HEIGHT64; i < total; i++)
        VGA_MEMORY64[i] = 0x0720;
    cursor_x = 0; cursor_y = 0;

    puts("MyOS x86_64 kernel booted\n");
    puts("Long Mode active.\n");

    // Halt for now
    for(;;) __asm__ __volatile__("hlt");
}


