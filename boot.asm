bits 32

; === Multiboot Header ===
section .multiboot
    dd 0x1BADB002
    dd 0x00
    dd - (0x1BADB002 + 0x00)

; === GDT ===
section .data
gdt_start:
    ; Null descriptor (обязательно первый)
    dq 0x0

gdt_code:
    ; Code segment descriptor
    ; Base = 0x0, Limit = 0xFFFFF
    ; Present = 1, Privilege = 0, Descriptor type = 1
    ; Type = 10 (code, non-conforming, readable)
    ; Granularity = 1, 32-bit default = 1, 64-bit code = 0
    dw 0xFFFF        ; Limit (bits 0-15)
    dw 0x0           ; Base (bits 0-15)
    db 0x0           ; Base (bits 16-23)
    db 10011010b     ; Present, Privilege (00), Descriptor type, Type
    db 11001111b     ; Granularity, 32-bit default, 64-bit code, Limit (bits 16-19)
    db 0x0           ; Base (bits 24-31)

gdt_data:
    ; Data segment descriptor
    ; Same as code segment, but type = 0010 (data, expand-up, writable)
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size of GDT (limit)
    dd gdt_start                ; Address of GDT

; Константы для селекторов
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

section .text
    global _start

_start:
    ; Загружаем GDT
    lgdt [gdt_descriptor]
    
    ; Обновляем сегментные регистры
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Устанавливаем стек
    mov esp, stack_top

    extern kernel_main
    call kernel_main

    hlt

section .bss
stack_bottom:
    resb 16384
stack_top: