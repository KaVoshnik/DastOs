BITS 32
extern kernel_main
extern load_idt

global start
start:
    ; Инициализация сегментов данных
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Загрузка IDT
    call load_idt
    
    ; Вызов основной функции ядра на C
    call kernel_main
    
    ; Бесконечный цикл
    cli
    hlt
    jmp $