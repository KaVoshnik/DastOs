; Обработчики прерываний
bits 32

section .text

global irq1_handler
irq1_handler:
    cli
    pusha
    
    extern keyboard_handler
    call keyboard_handler
    
    popa
    sti
    iret

global idt_flush
idt_flush:
    extern idtp
    lidt [idtp]
    ret