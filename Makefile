# Компиляторы и инструменты
AS = nasm
CC = gcc
LD = ld

# Директории
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = isodir

# Флаги компиляции
ASFLAGS = -f elf32
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -I$(SRC_DIR)/include
LDFLAGS = -m elf_i386 -T $(SRC_DIR)/linker.ld

# Исходные файлы
BOOT_ASM = $(SRC_DIR)/boot/boot.asm
INTERRUPTS_ASM = $(SRC_DIR)/kernel/interrupts.asm
KERNEL_C = $(SRC_DIR)/kernel/kernel.c

# Объектные файлы
BOOT_OBJ = $(BUILD_DIR)/boot.o
INTERRUPTS_OBJ = $(BUILD_DIR)/interrupts.o
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
OBJECTS = $(BOOT_OBJ) $(INTERRUPTS_OBJ) $(KERNEL_OBJ)

# Выходные файлы
KERNEL_BIN = $(BUILD_DIR)/myos.bin
ISO_FILE = myos.iso

# Цель по умолчанию
all: $(ISO_FILE)

# Создание директории для сборки
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Сборка объектных файлов
$(BOOT_OBJ): $(BOOT_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(BOOT_ASM) -o $(BOOT_OBJ)

$(INTERRUPTS_OBJ): $(INTERRUPTS_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(INTERRUPTS_ASM) -o $(INTERRUPTS_OBJ)

$(KERNEL_OBJ): $(KERNEL_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(KERNEL_C) -o $(KERNEL_OBJ)

# Связывание ядра
$(KERNEL_BIN): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(KERNEL_BIN)

# Создание ISO образа
$(ISO_FILE): $(KERNEL_BIN)
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/myos.bin
	grub-mkrescue -o $(ISO_FILE) $(ISO_DIR)

# Сборка только ядра
kernel: $(KERNEL_BIN)

# Запуск в QEMU (из bin файла)
run-bin: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN)

# Запуск в QEMU (из ISO образа)
run-iso: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE)

# Запуск в QEMU с VNC (для удаленного доступа)
run-vnc: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -vnc :1

# Очистка
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(ISO_FILE)
	rm -f $(ISO_DIR)/boot/myos.bin

# Полная очистка и пересборка
rebuild: clean all

# Показать структуру проекта
tree:
	@echo "Структура проекта MyOS:"
	@echo "├── src/"
	@echo "│   ├── boot/"
	@echo "│   │   └── boot.asm          # Загрузчик с Multiboot заголовком"
	@echo "│   ├── kernel/"
	@echo "│   │   ├── kernel.c          # Основной код ядра"
	@echo "│   │   └── interrupts.asm    # Обработчики прерываний"
	@echo "│   ├── include/              # Заголовочные файлы (пусто)"
	@echo "│   └── linker.ld             # Скрипт компоновщика"
	@echo "├── build/                    # Директория сборки"
	@echo "│   ├── *.o                   # Объектные файлы"
	@echo "│   └── myos.bin              # Скомпилированное ядро"
	@echo "├── isodir/                   # Структура ISO образа"
	@echo "│   └── boot/"
	@echo "│       ├── grub/grub.cfg     # Конфигурация GRUB"
	@echo "│       └── myos.bin          # Ядро для ISO"
	@echo "├── myos.iso                  # Загрузочный ISO образ"
	@echo "├── Makefile                  # Система сборки"
	@echo "└── replit.md                 # Документация проекта"

# Информация о сборке
info:
	@echo "MyOS Build System"
	@echo "=================="
	@echo "Исходники:"
	@echo "  Boot:       $(BOOT_ASM)"
	@echo "  Interrupts: $(INTERRUPTS_ASM)"
	@echo "  Kernel:     $(KERNEL_C)"
	@echo ""
	@echo "Сборка:"
	@echo "  Objects:    $(BUILD_DIR)/*.o"
	@echo "  Kernel:     $(KERNEL_BIN)"
	@echo "  ISO:        $(ISO_FILE)"
	@echo ""
	@echo "Доступные команды:"
	@echo "  make all       - Собрать ISO образ"
	@echo "  make kernel    - Собрать только ядро"
	@echo "  make run-iso   - Запустить ISO в QEMU"
	@echo "  make run-bin   - Запустить ядро в QEMU"
	@echo "  make clean     - Очистить сборку"
	@echo "  make rebuild   - Пересобрать с нуля"
	@echo "  make tree      - Показать структуру"
	@echo "  make info      - Эта информация"

# Помощь
help: info

.PHONY: all kernel clean rebuild run-bin run-iso run-vnc tree info help