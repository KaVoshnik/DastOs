; x86_64 IDT handlers for IRQ0 (timer) and IRQ1 (keyboard)
bits 64

section .text

global isr_stub_default
isr_stub_default:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    ; send EOI for PIC1 by default
    mov al, 0x20
    out 0x20, al
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    leave
    iretq

extern irq0_handler_c
global irq0_handler
irq0_handler:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    call irq0_handler_c
    ; EOI to PIC1
    mov al, 0x20
    out 0x20, al
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    leave
    iretq

extern irq1_handler_c
global irq1_handler
irq1_handler:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    call irq1_handler_c
    ; EOI to PIC1
    mov al, 0x20
    out 0x20, al
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    leave
    iretq

; ---------------- 64-bit software interrupt INT 0x80 (syscalls) ----------------
extern handle_syscall64
extern proc_exit_flag
extern syscall_exit_to_kernel
global syscall_handler
syscall_handler:
    push rbp
    mov rbp, rsp
    ; Save caller-saved we might clobber
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    ; Convention: rax=num, rdi=a0, rsi=a1, rdx=a2, r8=a3, r9=a4
    mov rdi, rax               ; place syscall number in rdi (1st arg)
    call handle_syscall64

    ; Check if syscall requested exit to kernel shell
    cmp byte [rel proc_exit_flag], 0
    je .no_exit

    ; clear flag
    mov byte [rel proc_exit_flag], 0

    ; Restore registers then jump to kernel exit handler
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    leave
    ; Jump (not returning to user)
    jmp syscall_exit_to_kernel

.no_exit:
    ; Return value in rax
    ; Restore registers
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    leave
    iretq


