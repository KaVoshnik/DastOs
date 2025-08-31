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

; GDT должна быть объявлена до её использования
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

; Загрузка ядра через INT 0x13
load_kernel:
    mov ah, 0x02
    mov al, 50      ; Увеличено до 50 секторов
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x00
    mov bx, 0x1000  ; ES:BX = 0x1000:0x0000 -> 0x10000
    mov es, bx
    xor bx, bx
    int 0x13
    jc disk_error
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

error_msg db "Disk error!", 0

BITS 32
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Копируем ядро из 0x10000 в 0x100000
    cld
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, 50 * 512  ; 50 секторов по 512 байт
    rep movsb

    ; Переход на ядро
    jmp 0x100000

times 510 - ($-$$) db 0
dw 0xAA55