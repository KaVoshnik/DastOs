; Поддержка пользовательского режима (Ring 3)
bits 32

section .text

; Переключение в пользовательский режим
; void switch_to_user_mode(uint32_t user_esp, uint32_t user_eip)
global switch_to_user_mode
switch_to_user_mode:
    cli                         ; Отключаем прерывания
    
    ; Получаем параметры из стека
    mov eax, [esp + 4]          ; user_esp
    mov ebx, [esp + 8]          ; user_eip
    
    ; Настраиваем селекторы для пользовательского режима
    ; 0x23 = селектор данных пользователя (Ring 3)
    ; 0x1B = селектор кода пользователя (Ring 3)
    mov ax, 0x23                ; Пользовательский сегмент данных
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Подготавливаем стек для IRET (переход в Ring 3)
    ; Порядок в стеке (сверху вниз):
    ; SS (пользовательский селектор стека)
    ; ESP (пользовательский стек)
    ; EFLAGS (с установленным IF для прерываний)
    ; CS (пользовательский селектор кода)
    ; EIP (адрес пользовательского кода)
    
    push 0x23                   ; SS - пользовательский селектор стека
    push eax                    ; ESP - пользовательский стек
    
    ; Подготавливаем флаги (включаем прерывания)
    pushf                       ; Получаем текущие флаги
    pop ecx
    or ecx, 0x200               ; Устанавливаем флаг IF (прерывания)
    push ecx                    ; EFLAGS
    
    push 0x1B                   ; CS - пользовательский селектор кода
    push ebx                    ; EIP - адрес пользовательского кода
    
    ; Переходим в пользовательский режим
    iret

; Переключение обратно в режим ядра
; void switch_to_kernel_mode()
global switch_to_kernel_mode
switch_to_kernel_mode:
    cli                         ; Отключаем прерывания
    
    ; Восстанавливаем селекторы ядра
    mov ax, 0x10                ; Селектор данных ядра (Ring 0)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    sti                         ; Включаем прерывания
    ret

; Создание пользовательской задачи
; void create_user_task(uint32_t entry_point, uint32_t stack_top, task_t* task)
global create_user_task
create_user_task:
    push ebp
    mov ebp, esp
    
    ; Получаем параметры
    mov eax, [ebp + 8]          ; entry_point
    mov ebx, [ebp + 12]         ; stack_top  
    mov ecx, [ebp + 16]         ; task
    
    ; Получаем указатель на registers_t в задаче
    mov edx, ecx
    add edx, 132                ; Смещение до поля regs
    
    ; Инициализируем регистры для пользовательского режима
    xor eax, eax
    mov [edx + 0], eax          ; eax = 0
    mov [edx + 4], eax          ; ebx = 0
    mov [edx + 8], eax          ; ecx = 0
    mov [edx + 12], eax         ; edx = 0
    mov [edx + 16], eax         ; esi = 0
    mov [edx + 20], eax         ; edi = 0
    
    ; Устанавливаем пользовательский стек
    mov [edx + 24], ebx         ; esp = stack_top
    mov [edx + 28], ebx         ; ebp = stack_top
    
    ; Устанавливаем точку входа
    mov eax, [ebp + 8]          ; entry_point
    mov [edx + 32], eax         ; eip = entry_point
    
    ; Устанавливаем флаги (включаем прерывания + пользовательский режим)
    mov eax, 0x3202             ; IF=1, IOPL=3 для пользовательского режима
    mov [edx + 36], eax         ; eflags
    
    ; Устанавливаем пользовательские сегментные регистры
    mov ax, 0x1B                ; Пользовательский селектор кода (Ring 3)
    mov [edx + 40], ax          ; cs
    mov ax, 0x23                ; Пользовательский селектор данных (Ring 3)
    mov [edx + 42], ax          ; ds
    mov [edx + 44], ax          ; es
    mov [edx + 46], ax          ; fs
    mov [edx + 48], ax          ; gs
    mov [edx + 50], ax          ; ss
    
    pop ebp
    ret

; Вход в пользовательский режим с сохранением контекста ядра
; void enter_user_mode_with_context(task_t* user_task, task_t* kernel_task)
global enter_user_mode_with_context  
enter_user_mode_with_context:
    push ebp
    mov ebp, esp
    
    ; Сохраняем контекст ядра
    mov eax, [ebp + 12]         ; kernel_task
    test eax, eax
    jz .switch_to_user
    
    ; call save_context  ; Временно отключено
    push eax
    extern save_context
    call save_context
    add esp, 4
    
.switch_to_user:
    ; Переключаемся на пользовательскую задачу
    mov eax, [ebp + 8]          ; user_task
    push eax
    extern restore_context
    call restore_context
    add esp, 4
    
    pop ebp
    ret

; Возврат из пользовательского режима в режим ядра
; Эта функция вызывается при системном вызове или прерывании
; void return_to_kernel_mode(task_t* kernel_task)
global return_to_kernel_mode
return_to_kernel_mode:
    push ebp
    mov ebp, esp
    
    ; Восстанавливаем контекст ядра
    mov eax, [ebp + 8]          ; kernel_task
    test eax, eax
    jz .done
    
    push eax
    call restore_context
    add esp, 4
    
.done:
    pop ebp
    ret

; Получение текущего уровня привилегий (CPL)
; int get_current_privilege_level()
global get_current_privilege_level
get_current_privilege_level:
    mov eax, cs                 ; Получаем селектор кода
    and eax, 3                  ; Извлекаем CPL (биты 0-1)
    ret

; Проверка, выполняется ли код в пользовательском режиме
; int is_user_mode()
global is_user_mode
is_user_mode:
    call get_current_privilege_level
    cmp eax, 3                  ; CPL = 3 означает пользовательский режим
    je .user_mode
    xor eax, eax                ; Возвращаем 0 (режим ядра)
    ret
.user_mode:
    mov eax, 1                  ; Возвращаем 1 (пользовательский режим)
    ret

; Настройка GDT для поддержки пользовательского режима
; void setup_user_mode_gdt()
global setup_user_mode_gdt
setup_user_mode_gdt:
    push ebp
    mov ebp, esp
    
    ; Simplified implementation - just return
    ; GDT setup is handled elsewhere
    
    pop ebp
    ret