#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

// PIC константы
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

// GDT константы
#define GDT_ENTRIES     8
#define GDT_NULL        0x00         // Нулевой дескриптор
#define GDT_KERNEL_CODE 0x08         // Дескриптор кода ядра (Ring 0)
#define GDT_KERNEL_DATA 0x10         // Дескриптор данных ядра (Ring 0)
#define GDT_USER_CODE   0x1B         // Дескриптор кода пользователя (Ring 3)
#define GDT_USER_DATA   0x23         // Дескриптор данных пользователя (Ring 3)

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

// Функции прерываний
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void handle_exception(void);
void handle_page_fault(void);
void timer_interrupt_handler(void);

// Внешние функции из ASM
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

#endif // INTERRUPTS_H