; start.asm (исправленный фрагмент)
BITS 32
extern kernel_main

global start
start:
    ; Инициализация сегментов данных
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Вызов основной функции ядра на C
    call kernel_main

    ; --- НАЧАЛО: Индикация жизни ---
    ; Убираем cli! Это позволяет системе реагировать на прерывания (если бы они были)
    ; mov eax, cr0
    ; or eax, 1
    ; mov cr0, eax  ; (уже сделано в загрузчике)

    ; Простая индикация жизни: мигающий символ в правом нижнем углу
    mov edi, 0xB8000          ; Адрес видеопамяти
    mov ebx, 0xF9E            ; Смещение до последнего символа (строка 24, столбец 79)
    mov ecx, 0                ; Счетчик для анимации
    mov edx, 0                ; Таймер задержки

.indicator_loop:
    ; Меняем символ на каждой итерации
    mov al, [indicator_chars + ecx]
    mov [edi + ebx], al       ; Записываем символ
    mov byte [edi + ebx + 1], 0x07  ; Записываем атрибут (светло-серый)

    ; Делаем задержку
    mov edx, 0xFFFFF
.delay_loop:
    dec edx
    jnz .delay_loop

    ; Переходим к следующему символу
    inc ecx
    cmp ecx, 4
    je .reset_count
    jmp .indicator_loop

.reset_count:
    mov ecx, 0
    jmp .indicator_loop

; Данные для индикатора
indicator_chars db '|', '/', '-', '\\', 0

; --- КОНЕЦ: Индикация жизни ---

    ; Если мы каким-то чудом вышли из цикла индикации,
    ; ставим страховочный бесконечный цикл
    jmp $