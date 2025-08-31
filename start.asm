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
    
    ; Вызов основной функции ядра на C
    call kernel_main
    
    ; Бесконечный цикл
    cli
    hlt
    jmp $