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

    mov ah, 0x02
    mov al, 10      ; Количество секторов
    mov ch, 0       ; Цилиндр 0
    mov cl, 2       ; Сектор 2
    mov dh, 0       ; Головка 0
    mov dl, [boot_drive]
    mov bx, 0x1000  ; Адрес загрузки ядра (изменил с 0x10000 на 0x1000 для 16-битного режима)
    int 0x13
    jc error

    in al, 0x92
    or al, 2
    out 0x92, al

    lgdt [gdt_descriptor]

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
    dw 0xFFFF           ; Лимит сегмента кода (0xFFFF)
    dw 0x0000           ; База (низкие 16 бит)
    db 0x00             ; База (средний байт)
    db 0x9A             ; Доступ (код, читаемый, исполняемый)
    db 0xCF             ; Флаги (4 ГБ, 32-бит)
    db 0x00             ; База (высокий байт)
    dw 0xFFFF           ; Лимит сегмента данных (0xFFFF)
    dw 0x0000           ; База (низкие 16 бит)
    db 0x00             ; База (средний байт)
    db 0x92             ; Доступ (данные, читаемые/записываемые)
    db 0xCF             ; Флаги (4 ГБ, 32-бит)
    db 0x00             ; База (высокий байт)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Размер GDT (16 бит)
    dd gdt_start                ; Адрес GDT (32 бита, но в 16-битном режиме