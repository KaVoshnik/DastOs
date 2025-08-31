#!/bin/bash

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
    nasm -f bin bootloader.asm -o bootloader.bin
}

build_kernel() {
    nasm -f elf32 start.asm -o start.o
    nasm -f elf32 idt.asm -o idt.o
    gcc -ffreestanding -c kernel.c -o kernel.o
    ld -T link.ld start.o idt.o kernel.o -o kernel.bin
}

create_image() {
    dd if=/dev/zero of=os.img bs=512 count=2880
    dd if=bootloader.bin of=os.img bs=512 count=1 conv=notrunc
    dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc
}

run_qemu() {
    qemu-system-x86_64 -fda os.img
}

full_build_run() {
    build_bootloader
    build_kernel
    create_image
    run_qemu
}

clean_project() {
    rm -f *.o *.bin os.img
}

while true; do
    show_menu
    read -p "Выберите опцию: " choice
    case $choice in
        1) build_bootloader ;;
        2) build_kernel ;;
        3) create_image ;;
        4) run_qemu ;;
        5) full_build_run ;;
        6) clean_project ;;
        0) exit 0 ;;
        *) echo "Неверный выбор" ;;
    esac
done