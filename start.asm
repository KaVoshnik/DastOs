BITS 32
extern kernel_main
extern load_idt

global start
start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x9000    ; Стек начинается выше адреса ядра

    call load_idt
    
    call kernel_main
    
    cli
    hlt
    jmp $