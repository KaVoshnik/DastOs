# Makefile for DastOs

all: dastos.img

boot.bin: boot.asm
    nasm -f bin boot.asm -o boot.bin

kernel.bin: kernel.o
    ld -m elf_i386 -Ttext 0x7E00 --oformat binary kernel.o -o kernel.bin

kernel.o: kernel.c
    gcc -ffreestanding -m32 -c kernel.c -o kernel.o

dastos.img: boot.bin kernel.bin
    dd if=/dev/zero of=dastos.img bs=512 count=2880  # Floppy size
    dd if=boot.bin of=dastos.img bs=512 conv=notrunc
    dd if=kernel.bin of=dastos.img bs=512 seek=1 conv=notrunc

clean:
    rm -f *.bin *.o dastos.img

.PHONY: all clean