#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}=== Building MyOS ===${NC}"

echo -e "${YELLOW}Step 1/5: Compiling boot loader...${NC}"
nasm -f elf32 src/boot_fixed.asm -o src/boot.o
echo -e "${GREEN}✓ Boot loader compiled${NC}"

echo -e "${YELLOW}Step 2/5: Compiling interrupt handlers...${NC}"
nasm -f elf32 src/interrupts_full.asm -o src/interrupts.o
echo -e "${GREEN}✓ Interrupt handlers compiled${NC}"

echo -e "${YELLOW}Step 3/5: Compiling kernel...${NC}"
gcc -m32 -c src/kernel_full.c -o src/kernel.o -ffreestanding -fno-stack-protector -nostdlib
echo -e "${GREEN}✓ Kernel compiled${NC}"

echo -e "${YELLOW}Step 4/5: Linking kernel...${NC}"
ld -m elf_i386 -T src/linker_fixed.ld src/boot.o src/interrupts.o src/kernel.o -o myos.bin
echo -e "${GREEN}✓ Kernel linked successfully${NC}"

echo -e "${YELLOW}Step 5/5: Creating bootable ISO...${NC}"
mkdir -p iso/boot/grub
cp myos.bin iso/boot/
echo 'menuentry "MyOS" { multiboot /boot/myos.bin }' > iso/boot/grub/grub.cfg
grub-mkrescue -o myos.iso iso/ 2>/dev/null
rm -rf iso src/*.o
echo -e "${GREEN}✓ ISO created successfully${NC}"

echo -e "${GREEN}=== Build Complete! ===${NC}"
echo -e "${GREEN}To run: qemu-system-i386 -cdrom myos.iso${NC}"
echo -e "${GREEN}   or:  qemu-system-i386 -kernel myos.bin${NC}"