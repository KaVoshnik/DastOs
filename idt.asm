; idt.asm
BITS 32

extern keyboard_handler
section .text

; Макрос для создания дескриптора прерывания
%macro IDT_ENTRY 2
    dw %1 & 0xFFFF          ; Низкие 16 бит адреса обработчика
    dw 0x08                 ; Сегмент кода (из GDT)
    db 0                    ; Зарезервировано
    db 0x8E                 ; Тип: 32-битный прерывание (Present, DPL=0, Type=Interrupt Gate)
    dw (%1 >> 16) & 0xFFFF  ; Высокие 16 бит адреса обработчика
%endmacro

; Таблица дескрипторов прерываний (IDT)
section .data
align 8
idt_start:
    times 33 * 8 db 0       ; Первые 32 вектора + IRQ1 (0x21)
    IDT_ENTRY keyboard_isr, 0x08  ; Вектор 0x21 для IRQ1 (клавиатура)
idt_end:

; Дескриптор IDT
idt_descriptor:
    dw idt_end - idt_start - 1  ; Размер IDT
    dd idt_start                ; Адрес IDT

; Обработчик прерывания клавиатуры
section .text
keyboard_isr:
    pushad                      ; Сохранить регистры
    call keyboard_handler       ; Вызвать обработчик на C
    mov al, 0x20                ; Отправить EOI (End of Interrupt) контроллеру PIC
    out 0x20, al
    popad                       ; Восстановить регистры
    iretd                       ; Вернуться из прерывания

global load_idt
load_idt:
    lidt [idt_descriptor]       ; Загрузить IDT
    ret