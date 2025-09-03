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

; Обработчик общих исключений
global exception_handler
exception_handler:
    pusha
    push ds
    push es
    push fs 
    push gs
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    extern handle_exception
    call handle_exception
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    ; Добавляем бесконечный цикл вместо iret для исключений
    cli
.hang:
    hlt
    jmp .hang

; Обработчик Page Fault (исключение 14)
global page_fault_handler
page_fault_handler:
    pusha
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    extern handle_page_fault
    call handle_page_fault
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    ; Page fault обычно приводит к остановке системы
    cli
.hang:
    hlt
    jmp .hang

global idt_flush
idt_flush:
    extern idtp
    lidt [idtp]
    ret

; Функции для работы с виртуальной памятью
global enable_paging
enable_paging:
    push ebp
    mov ebp, esp
    
    ; Получаем адрес Page Directory из аргумента
    mov eax, [ebp + 8]
    
    ; Загружаем адрес Page Directory в CR3
    mov cr3, eax
    
    ; Включаем пейджинг установкой бита PG в CR0
    mov eax, cr0
    or eax, 0x80000000    ; Устанавливаем бит 31 (PG)
    mov cr0, eax
    
    pop ebp
    ret

global disable_paging
disable_paging:
    push ebp
    mov ebp, esp
    
    ; Отключаем пейджинг сбросом бита PG в CR0
    mov eax, cr0
    and eax, 0x7FFFFFFF   ; Сбрасываем бит 31 (PG)
    mov cr0, eax
    
    pop ebp
    ret

global get_page_fault_address
get_page_fault_address:
    ; Возвращаем значение регистра CR2 (адрес page fault)
    mov eax, cr2
    ret

global flush_tlb
flush_tlb:
    ; Перезагружаем TLB перезаписью CR3
    mov eax, cr3
    mov cr3, eax
    ret