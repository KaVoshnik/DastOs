; boot.asm - Bootloader for DastOs
; Assemble with: nasm -f bin boot.asm -o boot.bin

BITS 16
ORG 0x7C00

start:
    cli                     ; Отключаем прерывания
    xor ax, ax              ; Обнуляем регистры
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00          ; Устанавливаем стек ниже загрузчика

    ; Загружаем ядро (предполагаем, что ядро в секторах 2-10)
    mov ah, 0x02            ; Функция чтения сектора BIOS
    mov al, 10              ; Читаем 10 секторов (достаточно для небольшого ядра)
    mov ch, 0               ; Цилиндр 0
    mov cl, 2               ; Сектор 2 (после загрузочного)
    mov dh, 0               ; Головка 0
    mov bx, 0x1000          ; Загружаем ядро по адресу 0x1000
    int 0x13                ; Вызов BIOS
    jc disk_error           ; Ошибка чтения

    ; Устанавливаем GDT и переходим в защищённый режим
    lgdt [gdt_descriptor]   ; Загружаем GDT
    mov eax, cr0
    or eax, 1               ; Устанавливаем бит Protected Mode
    mov cr0, eax

    ; Дальний прыжок для установки CS и перехода в 32-битный код
    jmp 0x08:protected_mode

disk_error:
    mov si, error_msg
    call print_string
    jmp $                   ; Зацикливаемся при ошибке

print_string:
    lodsb                   ; Читаем байт из SI
    or al, al               ; Проверяем конец строки
    jz .done
    mov ah, 0x0E            ; Вывод символа через BIOS
    int 0x10
    jmp print_string
.done:
    ret

error_msg db 'Disk read error!', 0x0D, 0x0A, 0

; GDT (Global Descriptor Table)
gdt_start:
    ; Нулевой дескриптор (обязателен)
    dd 0x00000000
    dd 0x00000000

    ; Сегмент кода: 32-бит, 4GB, executable
gdt_code:
    dw 0xFFFF               ; Limit (0-15)
    dw 0x0000               ; Base (0-15)
    db 0x00                 ; Base (16-23)
    db 0x9A                 ; Access: Present, ring 0, executable
    db 0xCF                 ; Granularity: 4KB, 32-bit
    db 0x00                 ; Base (24-31)

    ; Сегмент данных: 32-бит, 4GB, writable
gdt_data:
    dw 0xFFFF               ; Limit (0-15)
    dw 0x0000               ; Base (0-15)
    db 0x00                 ; Base (16-23)
    db 0x92                 ; Access: Present, ring 0, writable
    db 0xCF                 ; Granularity: 4KB, 32-bit
    db 0x00                 ; Base (24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Размер GDT
    dd gdt_start                ; Адрес GDT

[BITS 32]
protected_mode:
    ; Устанавливаем сегментные регистры
    mov ax, 0x10            ; Сегмент данных
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9000         ; Устанавливаем стек

    ; Передаём управление ядру
    jmp 0x1000              ; Прыгаем на адрес ядра

times 510 - ($ - $$) db 0   ; Заполняем до 510 байт
dw 0xAA55                   ; Сигнатура загрузочного сектора