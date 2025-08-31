BITS 32
global start

start:
    ; Просто выводим символ 'X' в верхний угол экрана
    mov byte [0xB8000], 'X'
    mov byte [0xB8001], 0x0F
    
    ; Бесконечный цикл
    jmp $

times 512 - ($-$$) db 0  ; Заполняем до 512 байт