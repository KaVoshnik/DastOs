# MyOS - Простая операционная система

## Описание проекта

MyOS - это простая операционная система, написанная на языке C и ассемблере, с загрузчиком, ядром и скриптом для сборки и запуска в эмуляторе QEMU.

## Структура файлов

### 1. kernel.c
Ядро операционной системы на языке C, содержащее функции для работы с видеоадаптером.

```c
// Функции для работы с видеоадаптером
#define VIDEO_MEMORY 0xB8000

void clear_screen() {
    char *video_memory = (char*)VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x07;
    }
}

void print_string(const char *str) {
    char *video_memory = (char*)VIDEO_MEMORY;
    while (*str) {
        *video_memory++ = *str++;
        *video_memory++ = 0x07;
    }
}

// Главная функция ядра
void kernel_main() {
    clear_screen();
    print_string("MyOS v0.1 - C Kernel Loaded Successfully!");
    
    // Бесконечный цикл с запретом прерываний
    asm volatile ("cli");
    while (1) {
        asm volatile ("hlt");
    }
}
```

### 2. link.ld
Скрипт линковки для компоновки ядра.

```
ENTRY(start)
SECTIONS
{
    . = 0x10000;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
```

### 3. myos.sh
Скрипт для сборки и запуска операционной системы.

```bash
#!/bin/bash

# MyOS Builder & Runner
# Меню для сборки и запуска операционной системы

show_menu() {
    echo "=================="
    echo "  MyOS Builder"
    echo "=================="
    echo "1. Собрать загрузчик (bootloader)"
    echo "2. Собрать ядро (kernel)"
    echo "3. Создать образ диска (os.img)"
    echo "4. Запустить в QEMU"
    echo "5. Полная сборка и запуск"
    echo "6. Очистить проект"
    echo "0. Выход"
    echo "=================="
}

# ... (функции сборки и запуска)
```

### 4. start.asm
Ассемблерный код инициализации для ядра.

```asm
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
    
    ; Бесконечный цикл
    cli
    hlt
    jmp $
```

### 5. bootloader.asm
Загрузчик операционной системы.

```asm
BITS 16
ORG 0x7C00

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Сохраняем номер загрузочного диска
    mov [boot_drive], dl

    ; Сообщение о начале загрузки
    mov si, msg_loading
    call print_string

    ; Загрузка ядра
    call load_kernel

    ; Включение A20
    call enable_a20

    ; Загрузка GDT
    lgdt [gdt_descriptor]

    ; Переход в защищённый режим
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode

; ... (остальной код загрузчика)
```

## Сборка и запуск

1. Запустите скрипт myos.sh:
```bash
./myos.sh
```

2. Выберите опцию в меню:
   - 1: Сборка загрузчика
   - 2: Сборка ядра
   - 3: Создание образа диска
   - 4: Запуск в QEMU
   - 5: Полная сборка и запуск
   - 6: Очистка проекта
   - 0: Выход

## Особенности реализации

- **Загрузчик**: Реализован на ассемблере, загружает ядро с диска, включает A20, переходит в защищённый режим.
- **Ядро**: Написано на C, выводит сообщение на экран с помощью прямого доступа к видеопамяти.
- **Сборка**: Используется NASM для ассемблера и GCC для компиляции C кода.
- **Эмуляция**: Запуск через QEMU с созданием образа диска.

Проект представляет собой учебный пример простой операционной системы с базовой функциональностью.