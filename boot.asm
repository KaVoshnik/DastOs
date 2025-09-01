bits 32

; === Multiboot Header ===
section .multiboot
    dd 0x1BADB002            ; magic number
    dd 0x00                  ; flags
    dd - (0x1BADB002 + 0x00) ; checksum (должен быть = -(magic + flags))

; === Bootloader code ===
section .text
    global _start

_start:
    mov esp, stack_top

    extern kernel_main
    call kernel_main

    hlt

section .bss
stack_bottom:
    resb 16384
stack_top: