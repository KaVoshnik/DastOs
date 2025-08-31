BITS 16

ORG 0x7C00

start:
    ; Установим сегментные регистры
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Включим A20 линию
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Загрузим GDT
    lgdt [gdt_descriptor]

    ; Переход в Protected Mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Длинный переход в 32-битный код
    jmp 0x08:protected_mode

; ========================
; GDT
; ========================
gdt_start:
    dq 0x0 ; NULL дескриптор

gdt_code:
    dw 0xFFFF    ; Лимит
    dw 0x0000    ; База
    db 0x00      ; База
    db 10011010b ; Флаги доступа
    db 11001111b ; Флаги + лимит
    db 0x00      ; База

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

; ========================
; 32-битный режим
; ========================
BITS 32

protected_mode:
    ; Установим сегментные регистры
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Загрузим ядро с диска (сектор 2+)
    call load_kernel

    ; Передадим управление ядру
    jmp 0x100000

; ========================
; Загрузка ядра
; ========================
load_kernel:
    mov ah, 0x02    ; Чтение секторов
    mov al, 10      ; Сколько секторов читать
    mov ch, 0       ; Цилиндр
    mov cl, 2       ; Сектор (начинаем с 2)
    mov dh, 0       ; Головка
    mov dl, 0x00    ; Флоппи диск
    mov bx, 0x0000  ; Адрес загрузки (0x100000)
    mov es, bx
    mov bx, 0x0000
    int 0x13
    ret

; Заполнение до 510 байт
times 510 - ($ - $$) db 0
dw 0xAA55