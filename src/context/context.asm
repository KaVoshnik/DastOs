; Переключение контекста задач
bits 32

section .text

; Функция сохранения контекста текущей задачи
; void save_context(task_t* task)
global save_context
save_context:
    push ebp
    mov ebp, esp
    
    ; Получаем указатель на задачу из аргумента
    mov eax, [ebp + 8]          ; task_t* task
    
    ; Сохраняем все регистры общего назначения в структуру task->regs
    ; Смещения в структуре registers_t:
    ; eax=0, ebx=4, ecx=8, edx=12, esi=16, edi=20, esp=24, ebp=28, eip=32, eflags=36
    mov edx, eax
    add edx, 132                ; Смещение до поля regs в структуре task_t
    
    ; Сохраняем регистры общего назначения
    mov [edx + 0], eax          ; Сохраняем исходное значение eax
    mov [edx + 4], ebx
    mov [edx + 8], ecx
    mov eax, [ebp + 8]          ; Восстанавливаем eax с указателем на task
    mov [edx + 12], eax         ; Сохраняем правильное значение eax вместо edx
    mov [edx + 16], esi
    mov [edx + 20], edi
    
    ; Сохраняем ESP (указывает на стек до вызова функции)
    mov eax, ebp
    add eax, 8                  ; ESP до вызова функции
    mov [edx + 24], eax
    
    ; Сохраняем EBP
    mov eax, [ebp]              ; Значение EBP вызывающей функции
    mov [edx + 28], eax
    
    ; Сохраняем EIP (адрес возврата)
    mov eax, [ebp + 4]          ; Адрес возврата
    mov [edx + 32], eax
    
    ; Сохраняем флаги
    pushf
    pop eax
    mov [edx + 36], eax
    
    ; Сохраняем сегментные регистры
    ; Смещения: cs=40, ds=42, es=44, fs=46, gs=48, ss=50
    mov ax, cs
    mov [edx + 40], ax
    mov ax, ds
    mov [edx + 42], ax
    mov ax, es
    mov [edx + 44], ax
    mov ax, fs
    mov [edx + 46], ax
    mov ax, gs
    mov [edx + 48], ax
    mov ax, ss
    mov [edx + 50], ax
    
    pop ebp
    ret

; Функция восстановления контекста задачи
; void restore_context(task_t* task)
global restore_context
restore_context:
    push ebp
    mov ebp, esp
    
    ; Получаем указатель на задачу
    mov eax, [ebp + 8]          ; task_t* task
    mov edx, eax
    add edx, 132                ; Смещение до поля regs
    
    ; Восстанавливаем сегментные регистры
    mov ax, [edx + 42]          ; ds
    mov ds, ax
    mov ax, [edx + 44]          ; es
    mov es, ax
    mov ax, [edx + 46]          ; fs
    mov fs, ax
    mov ax, [edx + 48]          ; gs
    mov gs, ax
    mov ax, [edx + 50]          ; ss
    mov ss, ax
    
    ; Восстанавливаем флаги
    mov eax, [edx + 36]
    push eax
    popf
    
    ; Восстанавливаем стек
    mov esp, [edx + 24]
    
    ; Восстанавливаем регистры общего назначения
    mov eax, [edx + 0]
    mov ebx, [edx + 4]
    mov ecx, [edx + 8]
    mov esi, [edx + 16]
    mov edi, [edx + 20]
    mov ebp, [edx + 28]
    
    ; Восстанавливаем edx последним и переходим к сохраненному EIP
    mov edx, [edx + 12]         ; edx был сохранен в месте eax
    jmp [ebp + 8]               ; Переход к сохраненному EIP

; Атомное переключение контекста между задачами
; void switch_context(task_t* current, task_t* next)
global switch_context
switch_context:
    push ebp
    mov ebp, esp
    
    ; Отключаем прерывания для атомарности
    cli
    
    ; Сохраняем контекст текущей задачи
    mov eax, [ebp + 8]          ; current task
    test eax, eax
    jz .restore_next            ; Если current == NULL, только восстанавливаем
    
    ; Сохраняем все регистры текущей задачи
    mov edx, eax
    add edx, 132                ; Смещение до regs
    
    ; Сохраняем регистры в момент переключения
    mov [edx + 0], eax
    mov [edx + 4], ebx
    mov [edx + 8], ecx
    push eax                    ; Сохраняем eax временно
    mov eax, [ebp + 8]          ; Восстанавливаем правильный eax
    mov [edx + 12], eax         ; Сохраняем edx в позицию eax (исправляем)
    pop eax
    mov [edx + 16], esi
    mov [edx + 20], edi
    
    ; Сохраняем стек и базовый указатель
    mov eax, esp
    mov [edx + 24], eax         ; ESP
    mov eax, ebp
    mov [edx + 28], eax         ; EBP
    
    ; Сохраняем адрес возврата (EIP)
    mov eax, .restore_next
    mov [edx + 32], eax         ; EIP для возврата в эту функцию
    
    ; Сохраняем флаги
    pushf
    pop eax
    mov [edx + 36], eax
    
    ; Сохраняем сегментные регистры
    mov ax, cs
    mov [edx + 40], ax
    mov ax, ds
    mov [edx + 42], ax
    mov ax, es
    mov [edx + 44], ax
    mov ax, fs
    mov [edx + 46], ax
    mov ax, gs
    mov [edx + 48], ax
    mov ax, ss
    mov [edx + 50], ax

.restore_next:
    ; Восстанавливаем контекст следующей задачи
    mov eax, [ebp + 12]         ; next task
    test eax, eax
    jz .done                    ; Если next == NULL, завершаем
    
    mov edx, eax
    add edx, 132                ; Смещение до regs
    
    ; Восстанавливаем сегментные регистры
    mov ax, [edx + 42]          ; ds
    mov ds, ax
    mov ax, [edx + 44]          ; es  
    mov es, ax
    mov ax, [edx + 46]          ; fs
    mov fs, ax
    mov ax, [edx + 48]          ; gs
    mov gs, ax
    
    ; Восстанавливаем флаги
    mov eax, [edx + 36]
    push eax
    popf
    
    ; Восстанавливаем стек
    mov esp, [edx + 24]
    mov ebp, [edx + 28]
    
    ; Восстанавливаем регистры общего назначения
    mov eax, [edx + 0]
    mov ebx, [edx + 4]
    mov ecx, [edx + 8]
    mov esi, [edx + 16]
    mov edi, [edx + 20]
    
    ; Включаем прерывания
    sti
    
    ; Переходим к коду следующей задачи
    push dword [edx + 32]       ; Помещаем EIP в стек
    mov edx, [edx + 12]         ; Восстанавливаем edx
    ret                         ; "Возвращаемся" к EIP следующей задачи

.done:
    ; Включаем прерывания и выходим
    sti
    pop ebp
    ret

; Функция для создания начального контекста задачи
; void init_task_context(task_t* task, void* entry_point, void* stack_top)
global init_task_context
init_task_context:
    push ebp
    mov ebp, esp
    
    ; Получаем параметры
    mov eax, [ebp + 8]          ; task
    mov ebx, [ebp + 12]         ; entry_point
    mov ecx, [ebp + 16]         ; stack_top
    
    ; Получаем указатель на registers_t
    mov edx, eax
    add edx, 132                ; Смещение до regs
    
    ; Обнуляем все регистры
    xor eax, eax
    mov [edx + 0], eax          ; eax
    mov [edx + 4], eax          ; ebx
    mov [edx + 8], eax          ; ecx
    mov [edx + 12], eax         ; edx
    mov [edx + 16], eax         ; esi
    mov [edx + 20], eax         ; edi
    
    ; Устанавливаем стек и базовый указатель
    mov [edx + 24], ecx         ; esp = stack_top
    mov [edx + 28], ecx         ; ebp = stack_top
    
    ; Устанавливаем точку входа
    mov [edx + 32], ebx         ; eip = entry_point
    
    ; Устанавливаем флаги (включаем прерывания)
    mov eax, 0x202              ; IF=1, остальные флаги по умолчанию
    mov [edx + 36], eax
    
    ; Устанавливаем сегментные регистры ядра
    mov ax, 0x08                ; Сегмент кода ядра
    mov [edx + 40], ax          ; cs
    mov ax, 0x10                ; Сегмент данных ядра
    mov [edx + 42], ax          ; ds
    mov [edx + 44], ax          ; es
    mov [edx + 46], ax          ; fs
    mov [edx + 48], ax          ; gs
    mov [edx + 50], ax          ; ss
    
    pop ebp
    ret

; Простая функция для переключения на задачу (используется планировщиком)
; void task_switch(task_t* next_task)
global task_switch
task_switch:
    push ebp
    mov ebp, esp
    
    ; Получаем указатель на следующую задачу
    mov eax, [ebp + 8]          ; next_task
    test eax, eax
    jz .done                    ; Если задача NULL, ничего не делаем
    
    ; Переключаемся на задачу (без сохранения текущего контекста,
    ; так как планировщик управляет этим)
    call restore_context
    
.done:
    pop ebp
    ret