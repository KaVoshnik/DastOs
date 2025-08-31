; bootloader.asm
bits 32

section .text
    global _start

_start:
    ; Установка стека
    mov esp, stack_top

    ; Вызов ядра
    extern kernel_main
    call kernel_main

    ; Бесконечный цикл
.hang:
    hlt
    jmp .hang

section .bss
align 4
stack_bottom:
    resb 16384 ; 16 KiB стек
stack_top: