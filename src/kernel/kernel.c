#include "../../include/types.h"
#include "../../include/terminal.h"
#include "../../include/memory.h"
#include "../../include/keyboard.h"
#include "../../include/interrupts.h"
#include "../../include/timer.h"
#include "../../include/scheduler.h"
#include "../../include/filesystem.h"
#include "../../include/shell.h"
#include "../../include/utils.h"
#include "../../include/syscalls.h"

// Глобальные переменные IDT и GDT
struct idt_entry idt[256];
struct idt_ptr idtp;
struct gdt_entry gdt[GDT_ENTRIES];
struct gdt_ptr gdtp;

// Переменные таймера
uint32_t timer_ticks = 0;
uint32_t timer_frequency = TIMER_FREQUENCY;

// Переменные шелла
char command_buffer[COMMAND_BUFFER_SIZE];
int command_length = 0;
int shell_ready = 0;

// Инициализация таймера
void init_timer(uint32_t frequency) {
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    outb(PIT_COMMAND, 0x36);
    outb(PIT_DATA0, divisor & 0xFF);
    outb(PIT_DATA0, divisor >> 8);
}

// Демонстрационные задачи
void idle_task(void) {
    while (1) {
        asm volatile("hlt");
    }
}

void demo_task1(void) {
    for (int i = 0; i < 5; i++) {
        terminal_writestring("Task 1 running... ");
        print_number(i);
        terminal_putchar('\n');
        for (volatile int j = 0; j < 1000000; j++);
    }
}

void demo_task2(void) {
    for (int i = 0; i < 3; i++) {
        terminal_writestring("Task 2 executing... ");
        print_number(i);
        terminal_putchar('\n');
        for (volatile int j = 0; j < 500000; j++);
    }
}

// Главная функция ядра
void kernel_main(void) {
    terminal_clear();
    terminal_writestring("MyOS Kernel v0.7 - Modular Version\n");
    terminal_writestring("================================\n");
    
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
    
    idt_set_gate(14, (uint32_t)page_fault_handler, 0x08, 0x8E);
    idt_set_gate(128, (uint32_t)syscall_handler, 0x08, 0x8E);
    idt_set_gate(32, (uint32_t)timer_handler, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1_handler, 0x08, 0x8E);
    idt_flush();
    
    terminal_writestring("IDT configured\n");

    // Инициализация подсистем
    init_memory_management();
    terminal_writestring("Memory management initialized\n");
    
    init_filesystem();
    terminal_writestring("File system ready\n");
    
    init_scheduler();
    terminal_writestring("Task scheduler ready\n");
    
    // Инициализация PIC
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, 0xFF);
    terminal_writestring("PIC configured\n");
    
    init_timer(TIMER_FREQUENCY);
    
    keyboard_init();
    keyboard_set_callback(shell_keyboard_callback);
    terminal_writestring("Keyboard module ready\n");
    
    asm volatile("sti");
    terminal_writestring("Interrupts enabled\n");

    // Создаем демонстрационные задачи
    create_task("idle", idle_task, 255);
    create_task("demo1", demo_task1, 10);
    create_task("demo2", demo_task2, 20);
    terminal_writestring("Demo tasks created\n");

    // Инициализация шелла
    enable_cursor(14, 15);
    terminal_writestring("\n=== MyOS v0.7 - Modular ===\n");
    terminal_writestring("Type 'help' for available commands.\n\n");
    
    shell_ready = 1;
    shell_prompt();

    while (1) {
        asm volatile("hlt");
    }
}