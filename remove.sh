#!/bin/bash

echo "Очистка проекта..."

# Удаляем временные файлы
rm -f *.o
rm -f *.img
rm -f *.bin  # Оставим только boot.bin и kernel.bin

# Оставляем только нужные файлы
echo "Оставлены:"
ls -l bootloader.asm kernel.asm link.ld boot.bin kernel.bin os.img