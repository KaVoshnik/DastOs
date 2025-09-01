#!/bin/bash

# Имя выходного файла
OUTPUT="myos"
ISO_NAME="myos.iso"
KERNEL="myos.bin"

# Цвета для красивого вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Сборка моей ОС ===${NC}"

# Проверка наличия исходных файлов
if [ ! -f "boot.asm" ]; then
    echo -e "${RED}Ошибка: Файл boot.asm не найден!${NC}"
    exit 1
fi

if [ ! -f "kernel.c" ]; then
    echo -e "${RED}Ошибка: Файл kernel.c не найден!${NC}"
    exit 1
fi

if [ ! -f "linker.ld" ]; then
    echo -e "${RED}Ошибка: Файл linker.ld не найден!${NC}"
    exit 1
fi

echo -e "${YELLOW}Шаг 1/5: Компиляция boot.asm...${NC}"
if ! nasm -f elf32 boot.asm -o boot.o; then
    echo -e "${RED}Ошибка при компиляции boot.asm${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Успешно скомпилирован boot.asm${NC}"

echo -e "${YELLOW}Шаг 2/5: Компиляция kernel.c...${NC}"
if ! gcc -m32 -c kernel.c -o kernel.o -ffreestanding -fno-stack-protector -nostdlib; then
    echo -e "${RED}Ошибка при компиляции kernel.c${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Успешно скомпилирован kernel.c${NC}"

echo -e "${YELLOW}Шаг 3/5: Линковка объектных файлов...${NC}"
if ! ld -m elf_i386 -T linker.ld boot.o kernel.o -o $KERNEL; then
    echo -e "${RED}Ошибка при линковке${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Успешно собран $KERNEL${NC}"

echo -e "${YELLOW}Шаг 4/5: Создание ISO-образа...${NC}"
# Создаем временную структуру для ISO
mkdir -p iso/boot/grub

# Копируем ядро
cp $KERNEL iso/boot/

# Создаем конфиг GRUB
cat > iso/boot/grub/grub.cfg << EOF
menuentry "MyOS" {
    multiboot /boot/$KERNEL
    boot
}
EOF

# Создаем ISO-образ
if ! grub-mkrescue -o $ISO_NAME iso 2>/dev/null; then
    # Если grub-mkrescue не найден, пробуем xorriso
    if command -v xorriso &> /dev/null; then
        xorriso -as mkisofs -o $ISO_NAME -b boot/grub/i386-pc/eltorito.img -no-emul-boot -boot-load-size 4 -boot-info-table iso/ 2>/dev/null
    else
        echo -e "${RED}Не найдены инструменты для создания ISO (grub-mkrescue или xorriso)${NC}"
        rm -rf iso
        exit 1
    fi
fi

echo -e "${GREEN}✓ Успешно создан $ISO_NAME${NC}"

echo -e "${YELLOW}Шаг 5/5: Запуск в QEMU...${NC}"
echo -e "${GREEN}Запуск QEMU. Для выхода нажмите Ctrl+A, затем X${NC}"

# Запуск ISO-образа в QEMU
if command -v qemu-system-i386 &> /dev/null; then
    qemu-system-i386 -cdrom $ISO_NAME
elif command -v qemu-system-x86_64 &> /dev/null; then
    qemu-system-x86_64 -cdrom $ISO_NAME
else
    echo -e "${RED}QEMU не найден. Установите qemu-system-i386 или qemu-system-x86_64${NC}"
    rm -rf iso
    exit 1
fi

# Очистка временных файлов
rm -rf iso boot.o kernel.o $KERNEL

echo -e "${GREEN}=== Готово! ===${NC}"