BITS 16
ORG 0x7C00

start:
    ; Установим сегментные регистры
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Загрузка ядра ДО перехода в защищённый режим
    call load_kernel

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
; GDT (перемещено до использования)
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
; Загрузка ядра
; ========================
load_kernel:
    mov ah, 0x02    ; Чтение секторов
    mov al, 20      ; Увеличено количество секторов
    mov ch, 0       ; Цилиндр
    mov cl, 2       ; Сектор (начинаем с 2)
    mov dh, 0       ; Головка
    mov dl, 0x00    ; Флоппи диск
    mov bx, 0x1000  ; ES:BX = 0x1000:0x0000
    mov es, bx
    xor bx, bx
    int 0x13
    jc disk_error   ; Если ошибка
    ret

disk_error:
    mov si, error_msg
    call print_string
    hlt

print_string:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

error_msg db "Disk read error!", 0

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

    ; Передадим управление ядру
    jmp 0x100000

; Заполнение до 510 байт
times 510 - ($ - $$) db 0
dw 0xAA55