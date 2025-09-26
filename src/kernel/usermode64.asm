; x86_64: switch to user mode with iretq
bits 64

section .text

; void switch_to_user_mode64(uint64_t user_rsp, uint64_t user_rip)
global switch_to_user_mode64
switch_to_user_mode64:
    ; Args: rdi=user_rsp, rsi=user_rip
    cli
    ; Load user data segment selectors (0x20 user data)
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Push user stack frame for iretq: SS, RSP, RFLAGS, CS, RIP
    push qword 0x20           ; SS (user data)
    push rdi                  ; RSP
    pushfq
    pop rax
    or rax, 0x200             ; IF=1
    push rax                  ; RFLAGS
    push qword 0x18           ; CS (user code)
    push rsi                  ; RIP
    iretq


