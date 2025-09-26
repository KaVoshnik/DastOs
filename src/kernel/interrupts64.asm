; x86_64 IDT handlers for IRQ0 (timer) and IRQ1 (keyboard)
bits 64

section .text

global isr_stub_default
isr_stub_default:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    ; send EOI for PIC1 by default
    mov al, 0x20
    out 0x20, al
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    leave
    iretq

extern irq0_handler_c
global irq0_handler
irq0_handler:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    call irq0_handler_c
    ; EOI to PIC1
    mov al, 0x20
    out 0x20, al
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    leave
    iretq

extern irq1_handler_c
global irq1_handler
irq1_handler:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    call irq1_handler_c
    ; EOI to PIC1
    mov al, 0x20
    out 0x20, al
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    leave
    iretq


