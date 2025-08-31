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

    ; Загрузка ядра
    call load_kernel

    ; Включение A20
    call enable_a20

    ; Загрузка GDT
    lgdt [gdt_descriptor]

    ; Переход в защищённый режим
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Правильно: far jump в сегмент 0x08 и смещение 0x10000
    jmp 0x08:0x10000

enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

; Загрузка ядра через CHS (более надежно)
load_kernel:
    mov ah, 0x02
    mov al, 50       ; Количество секторов
    mov ch, 0        ; Цилиндр
    mov cl, 2        ; Сектор
    mov dh, 0        ; Головка
    mov dl, [boot_drive]
    mov bx, 0x1000   ; ES:BX = 0x1000:0x0000
    mov es, bx
    xor bx, bx
    int 0x13
    jc disk_error
    ret

disk_error:
    ; Простая обработка ошибки - просто останавливаемся
    cli
    hlt
    jmp $

; Данные
boot_drive db 0

; GDT
gdt_start:
    dq 0x0

gdt_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

BITS 32
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Переход на ядро
    jmp 0x10000

; Заполнение до 510 байт
times 510 - ($-$$) db 0
dw 0xAA55