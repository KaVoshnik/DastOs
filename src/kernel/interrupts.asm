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
    ; Сохраняем код ошибки немедленно (он на вершине стека)
    mov eax, [esp]
    push eax                    ; сохраним error_code отдельно для передачи в C

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
    
    ; Передаем error_code как аргумент в C-обработчик
    extern handle_page_fault
    push dword [esp + 4 + 4*4 + 4*1] ; взять ранее сохранённый error_code
    ; Объяснение смещения: на стеке сейчас gs,fs,es,ds (4*2 байта push? здесь 4*2? нет, 4 регистра по 4 байта = 16),
    ; затем pusha (8 регистров, 32 байта), затем сверху лежит наш сохранённый error_code (4 байта),
    ; но мы уже сделали дополнительные push'и сегментов, потому прибавляем 4 (для сохранить экстра) + 32 (pusha) + 16 (сегменты) = 52 (0x34)
    ; Однако адрес выше вычислен в инструкции непосредственно как [esp + ...]. Мы используем явное смещение ниже после выравнивания.
    call handle_page_fault
    add esp, 4
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 4                 ; убрать сохранённый ранее error_code
    
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

; Обработчик таймера (IRQ0) для планировщика задач
global timer_handler
timer_handler:
    ; Сохраняем все регистры
    pusha
    push ds
    push es
    push fs
    push gs
    
    ; Устанавливаем сегменты ядра
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Вызываем планировщик
    extern timer_interrupt_handler
    call timer_interrupt_handler
    
    ; Восстанавливаем регистры
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    iret

; Обработчик системных вызовов (INT 0x80)
global syscall_handler
syscall_handler:
    ; Сохраняем все регистры
    pusha
    push ds
    push es
    push fs
    push gs
    
    ; Устанавливаем сегменты ядра
    mov ax, 0x10        ; Селектор сегмента данных ядра
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Номер системного вызова в EAX
    ; Аргументы в EBX, ECX, EDX, ESI, EDI
    
    ; Вызываем обработчик системных вызовов C
    push edi            ; arg4
    push esi            ; arg3
    push edx            ; arg2
    push ecx            ; arg1
    push ebx            ; arg0
    push eax            ; syscall_number
    
    extern handle_syscall
    call handle_syscall
    
    ; Убираем аргументы из стека
    add esp, 24
    
    ; Результат возвращается в EAX, сохраняем его
    mov [esp + 28], eax ; Перезаписываем сохраненное значение EAX
    
    ; Восстанавливаем регистры
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    ; Возвращаемся к пользовательскому коду
    iret