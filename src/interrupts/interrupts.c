#include "../../include/interrupts.h"
#include "../../include/terminal.h"
#include "../../include/utils.h"

// Функция установки IDT записи
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    extern struct idt_entry idt[256];
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// Обработчик исключений
void handle_exception(void) {
    terminal_writestring("Exception occurred\n");
}

// Обработчик page fault
void handle_page_fault(void) {
    uint32_t fault_addr = get_page_fault_address();
    terminal_writestring("Page fault at address: ");
    print_hex(fault_addr);
    terminal_writestring("\n");
}

// Обработчик таймера
void timer_interrupt_handler(void) {
    extern uint32_t timer_ticks;
    timer_ticks++;
    outb(PIC1_COMMAND, PIC_EOI);
}

// Обработчик системных вызовов
int handle_syscall(int syscall_num, int arg0, int arg1, int arg2, int arg3, int arg4) {
    (void)arg3; (void)arg4; // suppress warnings
    
    switch (syscall_num) {
        case 0: // SYS_EXIT
            return 0;
        case 1: // SYS_WRITE
            if (arg0 == 1) { // stdout
                char* buffer = (char*)arg1;
                for (int i = 0; i < arg2; i++) {
                    terminal_putchar(buffer[i]);
                }
                return arg2;
            }
            return -1;
        case 8: // SYS_GETPID
            return 1;
        default:
            return -1;
    }
}