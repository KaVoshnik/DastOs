# Makefile

CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -ffreestanding -Wall -Wextra -I. -c
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld

OBJ = bootloader.o kernel.o

all: dastos.bin

bootloader.o: bootloader.asm
	$(AS) $(ASFLAGS) $< -o $@

kernel.o: kernel.c
	$(CC) $(CFLAGS) $< -o $@

dastos.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o dastos.elf $^
	$(OBJCOPY) -O binary dastos.elf dastos.bin

run: dastos.bin
	qemu-system-i386 -kernel dastos.bin

clean:
	rm -f *.o dastos.elf dastos.bin

.PHONY: all run clean