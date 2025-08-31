BITS 16
ORG 0x7C00

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Загрузка ядра ДО перехода в защищённый режим
    call load_kernel

    ; Включение A20
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

; Загрузка ядра через INT 0x13 (реальный режим)
load_kernel:
    mov ah, 0x02
    mov al, 20       ; Увеличьте число секторов при росте ядра
    mov ch, 0
    mov cl, 2        ; Начинаем с сектора №2
    mov dh, 0
    mov dl, 0x00
    mov bx, 0x1000   ; ES:BX = 0x1000:0x0000 -> 0x10000
    mov es, bx
    xor bx, bx
    int 0x13
    jc .error        ; Проверка ошибок
    ret
.error:
    mov si, error_msg
    call print_real
    hlt

; Вывод сообщения в реальном режиме
print_real:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

error_msg db "Disk error!", 0

; GDT и данные...
; ... (остальной код без изменений)

BITS 32
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Прыжок на ядро
    jmp 0x100000  ; Теперь ядро загружено правильно

times 510 - ($-$$) db 0
dw 0xAA55