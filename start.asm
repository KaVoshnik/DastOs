BITS 32
extern kernel_main

global start
start:
    ; Инициализация сегментов данных
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Установка обработчиков прерываний
    call setup_idt
    
    ; Разрешение прерываний
    sti
    
    ; Вызов основной функции ядра на C
    call kernel_main
    
    ; Бесконечный цикл
    cli
    hlt
    jmp $

; Простая заглушка для IDT
setup_idt:
    ; Заполняем IDT заглушками
    mov eax, default_handler
    mov ebx, 0x08
    mov ecx, 256
    mov edi, idt_start
.loop:
    mov [edi], ax
    mov [edi + 2], bx
    mov word [edi + 4], 0x8E00
    shr eax, 16
    mov [edi + 6], ax
    shr eax, 16
    add edi, 8
    loop .loop
    ret

default_handler:
    iretd

section .bss
idt_start:
    resq 256
idt_end: