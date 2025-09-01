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

build_bootloader() {
    echo "Сборка загрузчика..."
    nasm -f bin bootloader.asm -o boot.bin
    if [ $? -eq 0 ]; then
        echo "✅ Загрузчик собран успешно"
        ls -lh boot.bin
    else
        echo "❌ Ошибка сборки загрузчика"
    fi
}

build_kernel() {
    echo "Сборка ядра..."
    # Компиляция ассемблерной заглушки
    nasm -f elf32 start.asm -o start.o
    
    # Компиляция ядра на C
    gcc -m32 -ffreestanding -nostdlib -c kernel.c -o kernel.o
    
    # Линковка
    ld -m elf_i386 -T link.ld start.o kernel.o -o kernel.bin -nostdlib
    
    if [ $? -eq 0 ]; then
        echo "✅ Ядро собрано успешно"
        ls -lh kernel.bin
    else
        echo "❌ Ошибка сборки ядра"
    fi
}

create_image() {
    echo "Создание образа диска..."
    dd if=/dev/zero of=os.img bs=512 count=2880 2>/dev/null
    dd if=boot.bin of=os.img bs=512 count=1 conv=notrunc 2>/dev/null
    dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✅ Образ диска создан"
        ls -lh os.img
    else
        echo "❌ Ошибка создания образа"
    fi
}

run_qemu() {
    if [ ! -f os.img ]; then
        echo "❌ Образ os.img не найден. Сначала создайте его."
        return
    fi
    echo "Запуск QEMU..."
    echo "Нажмите Ctrl+A, затем X для выхода"
    qemu-system-x86_64 -drive format=raw,file=os.img -m 512M
}

full_build() {
    echo "Полная сборка..."
    build_bootloader
    build_kernel
    create_image
    echo "✅ Полная сборка завершена"
}

clean_project() {
    echo "Очистка проекта..."
    rm -f *.o *.img *.bin
    echo "✅ Проект очищен"
}

# Основной цикл
while true; do
    show_menu
    read -p "Выберите действие (0-6): " choice
    echo
    
    case $choice in
        1)
            build_bootloader
            ;;
        2)
            build_kernel
            ;;
        3)
            create_image
            ;;
        4)
            run_qemu
            ;;
        5)
            full_build
            run_qemu
            ;;
        6)
            clean_project
            ;;
        0)
            echo "До свидания!"
            exit 0
            ;;
        *)
            echo "Неверный выбор. Попробуйте снова."
            ;;
    esac
    
    echo
    read -p "Нажмите Enter для продолжения..."
    clear
done