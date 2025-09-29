bits 32

; Multiboot заголовок
section .multiboot
align 4
    dd 0x1BADB002      ; magic number
    dd 0x00000003      ; flags: align modules | meminfo (no video request)
    dd -(0x1BADB002 + 0x00000003)  ; checksum

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
    
    ; VERY EARLY BOOT TRACE to VGA text memory (row 0)
    mov dword [0xB8000], 0x07420742   ; 'B' attr 0x07 twice -> BB
    mov dword [0xB8004], 0x074F074F   ; 'O''O' -> BOOT
    mov dword [0xB8008], 0x07540720   ; 'T' and space
    
    ; Вызываем главную функцию C
    extern kernel_main
    ; По протоколу Multiboot: eax = magic, ebx = addr multiboot info
    push ebx
    push eax
    call kernel_main
    add esp, 8
    
    ; Если ядро вернулось, зависаем
    cli
.hang:
    hlt
    jmp .hang