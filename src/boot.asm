bits 32

; Multiboot заголовок
section .multiboot
align 4
    dd 0x1BADB002      ; magic number
    dd 0x00000000      ; flags  
    dd -(0x1BADB002 + 0x00000000)  ; checksum

; Стек
section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KB стек
stack_top:

; Точка входа
section .text
global _start

_start:
    ; Настраиваем стек
    mov esp, stack_top
    
    ; Вызываем главную функцию C
    extern kernel_main
    call kernel_main
    
    ; Если ядро вернулось, зависаем
    cli
.hang:
    hlt
    jmp .hang