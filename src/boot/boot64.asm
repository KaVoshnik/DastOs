; 64-bit bootstrap with Multiboot (32-bit entry), switch to Long Mode and call kernel64_main
bits 32

section .multiboot
align 4
    dd 0x1BADB002            ; multiboot magic
    dd 0x00000000            ; flags
    dd -(0x1BADB002 + 0x00000000) ; checksum

section .bss
align 16
stack_bottom:
    resb 16384               ; 16KB stack
stack_top:

; Page tables for early Long Mode (identity-map first 1GiB minimal)
section .bss
align 4096
pml4:
    resq 512
pdpt:
    resq 512
pdir0:
    resq 512

section .text
global _start64
extern kernel64_main

_start64:
    ; Setup stack
    mov esp, stack_top

    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5            ; PAE bit
    mov cr4, eax

    ; Build minimal 1GiB identity map using 2MiB pages (PS bit)
    ; Zero tables
    mov edi, pml4
    mov ecx, (4096/4)
    xor eax, eax
    rep stosd

    mov edi, pdpt
    mov ecx, (4096/4)
    xor eax, eax
    rep stosd

    mov edi, pdir0
    mov ecx, (4096/4)
    xor eax, eax
    rep stosd

    ; pml4[0] -> pdpt | present|write
    mov eax, pdpt
    or eax, 0x3
    mov [pml4], eax
    mov dword [pml4+4], 0

    ; pdpt[0] -> pdir0 | present|write
    mov eax, pdir0
    or eax, 0x3
    mov [pdpt], eax
    mov dword [pdpt+4], 0

    ; Fill pdir0 with 2MiB pages for first 1GiB
    xor ecx, ecx              ; index
    mov edx, 0                ; phys addr base low
.map_loop:
    mov eax, edx
    or eax, 0x83              ; present|write|PS (2MiB)
    mov [pdir0 + ecx*8], eax
    mov dword [pdir0 + ecx*8 + 4], 0
    add edx, 0x200000         ; next 2MiB
    inc ecx
    cmp ecx, 512
    jb .map_loop

    ; Load PML4 into CR3
    mov eax, pml4
    mov cr3, eax

    ; Enable Long Mode in EFER
    mov ecx, 0xC0000080       ; IA32_EFER
    rdmsr
    or eax, 1 << 8            ; LME
    wrmsr

    ; Enable paging (and protected mode already set by multiboot env)
    mov eax, cr0
    or eax, 1 << 31           ; PG
    mov cr0, eax

    ; Now enable 64-bit mode via long mode code segment jump
    ; Setup temporary GDT with 64-bit code segment
    lgdt [gdt_desc]
    ; Far jump to 64-bit CS
    jmp 0x08:long_mode_entry

; ---------------- 64-bit mode ----------------
bits 64
long_mode_entry:
    ; Setup data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; Set 64-bit stack (reuse same memory)
    mov rsp, stack_top

    ; Call C kernel
    extern kernel64_main
    call kernel64_main

.halt:
    hlt
    jmp .halt

; ---------------- GDT ----------------
align 8
gdt:
    dq 0                     ; null
    dq 0x00AF9A000000FFFF    ; 0x08: 64-bit code segment (long, DPL0)
    dq 0x00AF92000000FFFF    ; 0x10: data segment

gdt_desc:
    dw gdt_end - gdt - 1
    dq gdt
gdt_end:


