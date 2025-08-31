; boot.asm - Bootloader for DastOs
; Assemble with: nasm -f bin boot.asm -o boot.bin

BITS 16
ORG 0x7C00

; Boot sector code
start:
    cli                 ; Disable interrupts
    xor ax, ax          ; Zero out AX
    mov ds, ax          ; Set DS to 0
    mov es, ax          ; Set ES to 0
    mov ss, ax          ; Set SS to 0
    mov sp, 0x7C00      ; Set stack pointer below bootloader

    ; Load kernel from disk (assuming kernel is in sectors 2-10)
    mov ah, 0x02        ; BIOS read sector function
    mov al, 9           ; Number of sectors to read (adjust if kernel is larger)
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Start from sector 2 (sector 1 is boot)
    mov dh, 0           ; Head 0
    mov bx, 0x7E00      ; Load address (right after bootloader)
    int 0x13            ; BIOS interrupt
    jc disk_error       ; Jump if carry flag set (error)

    ; Jump to kernel
    jmp 0x0000:0x7E00   ; Far jump to kernel entry

disk_error:
    mov si, error_msg
    call print_string
    jmp $               ; Infinite loop

print_string:
    lodsb               ; Load byte from SI to AL
    or al, al           ; Check if zero
    jz .done            ; If zero, done
    mov ah, 0x0E        ; BIOS teletype function
    int 0x10            ; BIOS interrupt
    jmp print_string    ; Repeat
.done:
    ret

error_msg db 'Disk read error!', 0x0D, 0x0A, 0

times 510 - ($ - $$) db 0  ; Pad to 510 bytes
dw 0xAA55                  ; Boot signature