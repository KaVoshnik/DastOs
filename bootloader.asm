BITS 16         ; 16-битный режим
ORG 0x7C00      ; Загрузочный сектор загружается по адресу 0x7C00

start:
    ; Установим сегментные регистры
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00  ; Установим стек

    ; Выведем сообщение
    mov si, msg
    call print_string

    ; Бесконечный цикл
hang:
    jmp hang

; Функция вывода строки
print_string:
    lodsb           ; Загружаем байт из [SI] в AL
    or al, al       ; Проверяем, не ноль ли
    jz done         ; Если ноль — конец строки
    mov ah, 0x0E    ; Функция BIOS: вывод символа
    int 0x10        ; Вызов прерывания
    jmp print_string
done:
    ret

msg db 'Hello, OS World!', 0

; Заполняем оставшееся место до 510 байт
times 510 - ($ - $$) db 0

; Сигнатура загрузочного сектора
dw 0xAA55