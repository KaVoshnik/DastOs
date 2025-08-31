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
    nasm -f elf32 kernel.asm -o kernel.o
    if [ $? -eq 0 ]; then
        ld -m elf_i386 -T link.ld kernel.o -o kernel.bin
        if [ $? -eq 0 ]; then
            echo "✅ Ядро собрано успешно"
            ls -lh kernel.bin
        else
            echo "❌ Ошибка линковки ядра"
        fi
    else
        echo "❌ Ошибка сборки ядра"
    fi
}

create_image() {
    echo "Создание образа диска..."
    dd if=/dev/zero of=os.img count=3000 2>/dev/null
    dd if=boot.bin of=os.img count=1 conv=notrunc 2>/dev/null
    dd if=kernel.bin of=os.img seek=1 conv=notrunc 2>/dev/null
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
    qemu-system-x86_64 -fda os.img
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