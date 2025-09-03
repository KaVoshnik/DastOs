#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}=== Building MyOS v0.6 ===${NC}"

# Проверяем наличие необходимых инструментов
if ! command -v nasm &> /dev/null; then
    echo -e "${RED}ERROR: NASM not found!${NC}"
    exit 1
fi

if ! command -v gcc &> /dev/null; then
    echo -e "${RED}ERROR: GCC not found!${NC}"
    exit 1
fi

if ! command -v ld &> /dev/null; then
    echo -e "${RED}ERROR: LD not found!${NC}"
    exit 1
fi

echo -e "${YELLOW}Step 1/5: Compiling boot loader...${NC}"
nasm -f elf32 src/boot.asm -o src/boot.o
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Boot loader compiled${NC}"
else
    echo -e "${RED}✗ Boot loader compilation failed${NC}"
    exit 1
fi

echo -e "${YELLOW}Step 2/5: Compiling interrupt handlers...${NC}"
nasm -f elf32 src/interrupts.asm -o src/interrupts.o
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Interrupt handlers compiled${NC}"
else
    echo -e "${RED}✗ Interrupt handlers compilation failed${NC}"
    exit 1
fi

echo -e "${YELLOW}Step 3/5: Compiling kernel...${NC}"
gcc -m32 -c src/kernel.c -o src/kernel.o -ffreestanding -fno-stack-protector -nostdlib -O2 -Wall -Wextra
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Kernel compiled${NC}"
else
    echo -e "${RED}✗ Kernel compilation failed${NC}"
    exit 1
fi

echo -e "${YELLOW}Step 4/5: Linking kernel...${NC}"
ld -m elf_i386 -T src/linker.ld src/boot.o src/interrupts.o src/kernel.o -o myos.bin
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Kernel linked successfully${NC}"
else
    echo -e "${RED}✗ Kernel linking failed${NC}"
    exit 1
fi

echo -e "${YELLOW}Step 5/5: Creating bootable ISO (optional)...${NC}"
if command -v grub-mkrescue &> /dev/null; then
    mkdir -p iso/boot/grub
    cp myos.bin iso/boot/
    echo 'menuentry "MyOS v0.6" { multiboot /boot/myos.bin }' > iso/boot/grub/grub.cfg
    grub-mkrescue -o myos.iso iso/ 2>/dev/null
    rm -rf iso
    if [ -f myos.iso ]; then
        echo -e "${GREEN}✓ ISO created successfully${NC}"
    else
        echo -e "${YELLOW}⚠ ISO creation skipped (grub-mkrescue issues)${NC}"
    fi
else
    echo -e "${YELLOW}⚠ ISO creation skipped (grub-mkrescue not available)${NC}"
fi

# Очистка объектных файлов
rm -f src/*.o

echo -e "${GREEN}=== Build Complete! ===${NC}"
echo -e "${GREEN}Binary: myos.bin${NC}"
if [ -f myos.iso ]; then
    echo -e "${GREEN}ISO: myos.iso${NC}"
fi

echo -e "${YELLOW}To run in QEMU:${NC}"
echo -e "${GREEN}qemu-system-i386 -kernel myos.bin${NC}"
if [ -f myos.iso ]; then
    echo -e "${GREEN}qemu-system-i386 -cdrom myos.iso${NC}"
fi