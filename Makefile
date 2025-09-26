# Компиляторы и инструменты
AS = nasm
CC = gcc
LD = ld

# Директории
SRC_DIR = src
BUILD_DIR = build

# Флаги компиляции
ASFLAGS = -f elf32
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -c -Iinclude
LDFLAGS = -m elf_i386 -T linker.ld

# Исходные файлы
BOOT_ASM = $(SRC_DIR)/boot/boot.asm
INTERRUPTS_ASM = $(SRC_DIR)/interrupts/interrupts.asm
CONTEXT_ASM = $(SRC_DIR)/context/context.asm
SYSCALLS_ASM = $(SRC_DIR)/syscalls/syscalls.asm
USER_MODE_ASM = $(SRC_DIR)/usermode/usermode.asm

KERNEL_C = $(SRC_DIR)/kernel/kernel.c
KEYBOARD_C = $(SRC_DIR)/keyboard/keyboard.c
TERMINAL_C = $(SRC_DIR)/terminal/terminal.c
UTILS_C = $(SRC_DIR)/utils/utils.c
INTERRUPTS_C = $(SRC_DIR)/interrupts/interrupts.c
MEMORY_C = $(SRC_DIR)/memory/memory.c
FILESYSTEM_C = $(SRC_DIR)/filesystem/filesystem.c
SCHEDULER_C = $(SRC_DIR)/scheduler/scheduler.c
SHELL_C = $(SRC_DIR)/shell/shell.c

# Объектные файлы
BOOT_OBJ = $(BUILD_DIR)/boot.o
INTERRUPTS_ASM_OBJ = $(BUILD_DIR)/interrupts_asm.o
CONTEXT_OBJ = $(BUILD_DIR)/context.o
SYSCALLS_OBJ = $(BUILD_DIR)/syscalls.o
USER_MODE_OBJ = $(BUILD_DIR)/usermode.o

KERNEL_OBJ = $(BUILD_DIR)/kernel.o
KEYBOARD_OBJ = $(BUILD_DIR)/keyboard.o
TERMINAL_OBJ = $(BUILD_DIR)/terminal.o
UTILS_OBJ = $(BUILD_DIR)/utils.o
INTERRUPTS_C_OBJ = $(BUILD_DIR)/interrupts_c.o
MEMORY_OBJ = $(BUILD_DIR)/memory.o
FILESYSTEM_OBJ = $(BUILD_DIR)/filesystem.o
SCHEDULER_OBJ = $(BUILD_DIR)/scheduler.o
SHELL_OBJ = $(BUILD_DIR)/shell.o

OBJECTS = $(BOOT_OBJ) $(INTERRUPTS_ASM_OBJ) $(CONTEXT_OBJ) $(SYSCALLS_OBJ) $(USER_MODE_OBJ) $(KERNEL_OBJ) $(KEYBOARD_OBJ) $(TERMINAL_OBJ) $(UTILS_OBJ) $(INTERRUPTS_C_OBJ) $(MEMORY_OBJ) $(FILESYSTEM_OBJ) $(SCHEDULER_OBJ) $(SHELL_OBJ)

# Выходные файлы
KERNEL_BIN = $(BUILD_DIR)/myos.bin

# Цель по умолчанию
all: $(KERNEL_BIN)

# Создание директории для сборки
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Сборка ASM файлов
$(BOOT_OBJ): $(BOOT_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(BOOT_ASM) -o $(BOOT_OBJ)

$(INTERRUPTS_ASM_OBJ): $(INTERRUPTS_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(INTERRUPTS_ASM) -o $(INTERRUPTS_ASM_OBJ)

$(CONTEXT_OBJ): $(CONTEXT_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(CONTEXT_ASM) -o $(CONTEXT_OBJ)

$(SYSCALLS_OBJ): $(SYSCALLS_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(SYSCALLS_ASM) -o $(SYSCALLS_OBJ)

$(USER_MODE_OBJ): $(USER_MODE_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(USER_MODE_ASM) -o $(USER_MODE_OBJ)

# Сборка C файлов
$(KERNEL_OBJ): $(KERNEL_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(KERNEL_C) -o $(KERNEL_OBJ)

$(KEYBOARD_OBJ): $(KEYBOARD_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(KEYBOARD_C) -o $(KEYBOARD_OBJ)

$(TERMINAL_OBJ): $(TERMINAL_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TERMINAL_C) -o $(TERMINAL_OBJ)

$(UTILS_OBJ): $(UTILS_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(UTILS_C) -o $(UTILS_OBJ)

$(INTERRUPTS_C_OBJ): $(INTERRUPTS_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INTERRUPTS_C) -o $(INTERRUPTS_C_OBJ)

$(MEMORY_OBJ): $(MEMORY_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(MEMORY_C) -o $(MEMORY_OBJ)

$(FILESYSTEM_OBJ): $(FILESYSTEM_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(FILESYSTEM_C) -o $(FILESYSTEM_OBJ)

$(SCHEDULER_OBJ): $(SCHEDULER_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SCHEDULER_C) -o $(SCHEDULER_OBJ)

$(SHELL_OBJ): $(SHELL_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SHELL_C) -o $(SHELL_OBJ)

# Связывание ядра
$(KERNEL_BIN): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(KERNEL_BIN)

# Сборка только ядра
kernel: $(KERNEL_BIN)

# Запуск в QEMU
run: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN)

# Очистка
clean:
	rm -rf $(BUILD_DIR)

# Полная очистка и пересборка
rebuild: clean all

.PHONY: all kernel clean rebuild run