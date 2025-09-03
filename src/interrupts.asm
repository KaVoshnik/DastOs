; Обработчики прерываний
bits 32

section .text

global irq1_handler
irq1_handler:
    ; Отключаем прерывания (уже отключены при входе в обработчик)
    ; Сохраняем все регистры
    pusha
    push ds
    push es
    push fs
    push gs
    
    ; Устанавливаем правильные сегменты данных
    mov ax, 0x10      ; Селектор сегмента данных ядра  
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Вызываем обработчик C
    extern keyboard_handler
    call keyboard_handler
    
    ; Восстанавливаем сегментные регистры
    pop gs
    pop fs
    pop es
    pop ds
    ; Восстанавливаем общие регистры
    popa
    
    ; Выходим из прерывания
    iret

global idt_flush
idt_flush:
    extern idtp
    lidt [idtp]
    ret