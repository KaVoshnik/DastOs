BITS 16
ORG 0x7C00

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [boot_drive], dl

    mov si, msg_loading
    call print_string

    ; Загрузка ядра
    mov ah, 0x02    ; Функция чтения секторов
    mov al, 10      ; Количество секторов (должно быть достаточно для ядра)
    mov ch, 0       ; Цилиндр 0
    mov cl, 2       ; Сектор 2 (после загрузочного)
    mov dh, 0       ; Головка 0
    mov dl, [boot_drive]
    mov bx, 0x1000  ; Адрес загрузки ядра
    int 0x13
    jc error        ; Переход на ошибку при сбое

    ; Включение линии A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Настройка GDT
    lgdt [gdt_descriptor]

    ; Переход в защищённый режим
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode

print_string:
    mov ah, 0x0E
    mov bx, 0x0007
.print_loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .print_loop
.done:
    ret

error:
    mov si, msg_error
    call print_string
    jmp $

msg_loading: db "Loading MyOS...", 0
msg_error: db "Failed to load kernel", 0
boot_drive: db 0

gdt_start:
    dd 0, 0             ; Нулевой дескриптор
    dw 0xFFFF           ; Лимит сегмента кода
    dw 0                ; База (низкие 16 бит)
    db 0                ; База (средний байт)
    db 0x9A             ; Доступ (код, читаемый)
    db 0xCF             ; Флаги (4 ГБ, 32-бит)
    db 0                ; База (высокий байт)
    dw 0xFFFF           ; Лимит сегмента данных
    dw 0                ; База (низкие 16 бит)
    db 0                ; База (средний байт)
    db 0x92             ; Доступ (данные, записываемые)
    db 0xCF             ; Флаги (4 ГБ, 32-бит)
    db 0                ; База (высокий байт)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Размер GDT
    dd gdt_start                ; Адрес GDT

protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    jmp 0x1000:0         ; Прыжок в защищённый режим на адрес 0x1000

times 510-($-$$) db 0
dw 0xAA55