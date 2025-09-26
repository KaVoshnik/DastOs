; Системные вызовы
bits 32

section .text

; Обработчик системных вызовов находится в interrupts.asm

; Функции для вызова системных вызовов из пользовательского кода

; int syscall0(int syscall_num)
global syscall0
syscall0:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]      ; syscall_num
    int 0x80
    
    pop ebp
    ret

; int syscall1(int syscall_num, int arg0)
global syscall1
syscall1:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]      ; syscall_num
    mov ebx, [ebp + 12]     ; arg0
    int 0x80
    
    pop ebp
    ret

; int syscall2(int syscall_num, int arg0, int arg1)
global syscall2
syscall2:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]      ; syscall_num
    mov ebx, [ebp + 12]     ; arg0
    mov ecx, [ebp + 16]     ; arg1
    int 0x80
    
    pop ebp
    ret

; int syscall3(int syscall_num, int arg0, int arg1, int arg2)
global syscall3
syscall3:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]      ; syscall_num
    mov ebx, [ebp + 12]     ; arg0
    mov ecx, [ebp + 16]     ; arg1
    mov edx, [ebp + 20]     ; arg2
    int 0x80
    
    pop ebp
    ret

; int syscall4(int syscall_num, int arg0, int arg1, int arg2, int arg3)
global syscall4
syscall4:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]      ; syscall_num
    mov ebx, [ebp + 12]     ; arg0
    mov ecx, [ebp + 16]     ; arg1
    mov edx, [ebp + 20]     ; arg2
    mov esi, [ebp + 24]     ; arg3
    int 0x80
    
    pop ebp
    ret

; int syscall5(int syscall_num, int arg0, int arg1, int arg2, int arg3, int arg4)
global syscall5
syscall5:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]      ; syscall_num
    mov ebx, [ebp + 12]     ; arg0
    mov ecx, [ebp + 16]     ; arg1
    mov edx, [ebp + 20]     ; arg2
    mov esi, [ebp + 24]     ; arg3
    mov edi, [ebp + 28]     ; arg4
    int 0x80
    
    pop ebp
    ret