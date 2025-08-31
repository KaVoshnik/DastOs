# Makefile for DastOs

CC = i686-elf-gcc
LD = i686-elf-ld
NASM = nasm
QEMU = qemu-system-i386

all: dastos.img

boot.bin: boot.asm
    $(NASM) -f bin boot.asm -o boot.bin

kernel.o: kernel.c
    $(CC) -ffreestanding -m32 -c kernel.c -o kernel.o

kernel.bin: kernel.o
    $(LD) -Ttext 0x1000 --oformat binary kernel.o -o kernel.bin

dastos.img: boot.bin kernel.bin
    dd if=/dev/zero of=dastos.img bs=512 count=2880
    dd if=boot.bin of=dastos.img bs=512 conv=notrunc
    dd if=kernel.bin of=dastos.img bs=512 seek=1 conv=notrunc

clean:
    rm -f *.bin *.o dastos.img output.log

test: dastos.img
    ./test_dastos.sh

.PHONY: all clean test