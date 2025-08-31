BITS 32

ORG 0x100000

start:
    ; Очистим экран
    call clear_screen

    ; Приветствие
    mov esi, welcome_msg
    call print_string

main_loop:
    mov esi, prompt
    call print_string

    ; Читаем строку
    mov edi, input_buffer
    call read_string

    ; Проверим команду
    mov esi, input_buffer
    call parse_command

    jmp main_loop

; ========================
; Функции
; ========================

; Очистка экрана (просто вывод пробелов)
clear_screen:
    mov eax, 0xB8000
    mov ecx, 80*25*2
    mov edi, eax
    mov al, ' '
    mov ah, 0x07
    rep stosb
    ret

; Вывод строки
print_string:
    pusha
    mov ebx, 0xB8000    ; VGA текстовый буфер
    mov ecx, 0          ; Позиция курсора

print_loop:
    lodsb
    cmp al, 0
    je print_done
    mov [ebx + ecx*2], al
    mov byte [ebx + ecx*2 + 1], 0x07
    inc ecx
    jmp print_loop
print_done:
    popa
    ret

; Чтение строки с клавиатуры
read_string:
    pusha
    mov ecx, 0

read_loop:
    ; Ждём нажатия клавиши
    mov ah, 0
    int 0x16

    ; Enter?
    cmp al, 13
    je read_enter

    ; Backspace?
    cmp al, 8
    je read_backspace

    ; Сохраняем символ
    stosb
    inc ecx

    ; Покажем символ на экране
    mov ah, 0x0E
    int 0x10

    jmp read_loop

read_backspace:
    cmp ecx, 0
    je read_loop
    dec edi
    dec ecx
    mov ah, 0x0E
    mov al, 8
    int 0x10
    mov al, ' '
    int 0x10
    mov al, 8
    int 0x10
    jmp read_loop

read_enter:
    mov byte [edi], 0   ; Null-terminate
    mov ah, 0x0E
    mov al, 13
    int 0x10
    mov al, 10
    int 0x10
    popa
    ret

; Парсер команд
parse_command:
    pusha
    mov esi, input_buffer

    ; Сравним с "help"
    mov edi, help_cmd
    call strcmp
    test eax, eax
    jz cmd_help

    ; Сравним с "clear"
    mov edi, clear_cmd
    call strcmp
    test eax, eax
    jz cmd_clear

    ; Неизвестная команда
    mov esi, unknown_cmd_msg
    call print_string
    popa
    ret

cmd_help:
    mov esi, help_text
    call print_string
    popa
    ret

cmd_clear:
    call clear_screen
    popa
    ret

; Простое сравнение строк
strcmp:
    push esi
    push edi
strcmp_loop:
    mov al, [esi]
    mov bl, [edi]
    cmp al, bl
    jne strcmp_not_equal
    test al, al
    jz strcmp_equal
    inc esi
    inc edi
    jmp strcmp_loop
strcmp_equal:
    xor eax, eax
    pop edi
    pop esi
    ret
strcmp_not_equal:
    mov eax, 1
    pop edi
    pop esi
    ret

; ========================
; Данные
; ========================
welcome_msg db 'MyOS v0.1', 13, 10, 0
prompt db '> ', 0
input_buffer times 256 db 0

help_cmd db 'help', 0
clear_cmd db 'clear', 0

help_text db 'Available commands:', 13, 10
          db '  help  - Show this help', 13, 10
          db '  clear - Clear screen', 13, 10, 0

unknown_cmd_msg db 'Unknown command. Type "help" for available commands.', 13, 10, 0

; Заполнение до конца сектора
times 512 - ($ - $$) db 0