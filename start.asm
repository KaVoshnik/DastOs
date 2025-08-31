BITS 32
extern kernel_main

global start
start:
    ; Установка стека
    mov esp, 0x90000
    
    ; Вызов основной функции ядра на C
    call kernel_main
    
    ; Бесконечный цикл, если ядро вернется
    jmp $