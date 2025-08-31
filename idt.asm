; idt.asm
BITS 32

extern keyboard_handler

section .text

%macro IDT_ENTRY 1
    dw %1
    dw 0x08
    db 0
    db 0x8E
    dw 0
%endmacro

section .data
align 8
idt_start:
    times 33 * 8 db 0
    IDT_ENTRY keyboard_isr
    dw 0x0000
idt_end:

idt_descriptor:
    dw idt_end - idt_start - 1
    dd idt_start

section .text
keyboard_isr:
    pushad
    call keyboard_handler
    mov al, 0x20
    out 0x20, al
    popad
    iretd

global load_idt
load_idt:
    lidt [idt_descriptor]
    ret