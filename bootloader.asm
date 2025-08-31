; bootloader.asm
BITS 16
ORG 0x7C00

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Сохраняем номер загрузочного диска
    mov [boot_drive], dl

    ; Сообщение о начале загрузки
    mov si, msg_loading
    call print_string

    ; Загрузка ядра (упрощённый пример)
    mov ah, 0x02    ; Функция BIOS: чтение секторов
    mov al, 10      ; Количество секторов
    mov ch, 0       ; Цилиндр 0
    mov cl, 2       ; Сектор 2
    mov dh, 0       ; Головка 0
    mov dl, [boot_drive]
    mov bx, 0x10000 ; Адрес загрузки ядра
    int 0x13
    jc error        ; Прыжок на ошибку при сбое

    ; Включение A20 (упрощённый пример)
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Загрузка GDT
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
    dd 0, 0
    dw 0xFFFF, 0
    db 0, 0x9A, 0xCF, 0
    dw 0xFFFF, 0
    db 0, 0x92, 0xCF, 0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    jmp 0x10000

times 510-($-$$) db 0
dw 0xAA55