# Компиляторы и инструменты
AS = nasm
CC = gcc
LD = ld
CC64 = gcc
LD64 = ld

# Директории
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = isodir

# Флаги компиляции
ASFLAGS = -f elf32
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -I$(SRC_DIR)/include
LDFLAGS = -m elf_i386 -T $(SRC_DIR)/linker.ld
LDFLAGS64 = -T $(SRC_DIR)/linker64.ld

# Исходные файлы
BOOT_ASM = $(SRC_DIR)/boot/boot.asm
BOOT64_ASM = $(SRC_DIR)/boot/boot64.asm
INTERRUPTS_ASM = $(SRC_DIR)/kernel/interrupts.asm
KERNEL_C = $(SRC_DIR)/kernel/kernel.c
KEYBOARD_C = $(SRC_DIR)/kernel/keyboard.c
CONTEXT_ASM = $(SRC_DIR)/kernel/context.asm
SYSCALLS_ASM = $(SRC_DIR)/kernel/syscalls.asm
USER_MODE_ASM = $(SRC_DIR)/kernel/usermode.asm

# Объектные файлы
BOOT_OBJ = $(BUILD_DIR)/boot.o
INTERRUPTS_OBJ = $(BUILD_DIR)/interrupts.o
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
KEYBOARD_OBJ = $(BUILD_DIR)/keyboard.o
CONTEXT_OBJ = $(BUILD_DIR)/context.o
SYSCALLS_OBJ = $(BUILD_DIR)/syscalls.o
USER_MODE_OBJ = $(BUILD_DIR)/usermode.o
OBJECTS = $(BOOT_OBJ) $(INTERRUPTS_OBJ) $(KERNEL_OBJ) $(KEYBOARD_OBJ) $(CONTEXT_OBJ) $(SYSCALLS_OBJ) $(USER_MODE_OBJ)
OBJECTS64 = $(BUILD_DIR)/boot64.o $(BUILD_DIR)/kernel64.o

# Выходные файлы
KERNEL_BIN = $(BUILD_DIR)/myos.bin
KERNEL64_BIN = $(BUILD_DIR)/myos64.bin
ISO_FILE = myos.iso

# Цель по умолчанию
all: $(ISO_FILE)

# Создание директории для сборки
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Сборка объектных файлов
$(BOOT_OBJ): $(BOOT_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(BOOT_ASM) -o $(BOOT_OBJ)

$(BUILD_DIR)/boot64.o: $(BOOT64_ASM) | $(BUILD_DIR)
	$(AS) -f elf64 $(BOOT64_ASM) -o $(BUILD_DIR)/boot64.o

$(INTERRUPTS_OBJ): $(INTERRUPTS_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(INTERRUPTS_ASM) -o $(INTERRUPTS_OBJ)

$(CONTEXT_OBJ): $(CONTEXT_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(CONTEXT_ASM) -o $(CONTEXT_OBJ)

$(SYSCALLS_OBJ): $(SYSCALLS_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(SYSCALLS_ASM) -o $(SYSCALLS_OBJ)

$(USER_MODE_OBJ): $(USER_MODE_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(USER_MODE_ASM) -o $(USER_MODE_OBJ)

$(KERNEL_OBJ): $(KERNEL_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(KERNEL_C) -o $(KERNEL_OBJ)

$(BUILD_DIR)/kernel64.o: $(SRC_DIR)/kernel/kernel64.c | $(BUILD_DIR)
	$(CC64) -m64 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -I$(SRC_DIR)/include $(SRC_DIR)/kernel/kernel64.c -o $(BUILD_DIR)/kernel64.o

$(KEYBOARD_OBJ): $(KEYBOARD_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(KEYBOARD_C) -o $(KEYBOARD_OBJ)

# Связывание ядра
$(KERNEL_BIN): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(KERNEL_BIN)

$(KERNEL64_BIN): $(OBJECTS64)
	$(LD64) $(LDFLAGS64) $(OBJECTS64) -o $(KERNEL64_BIN)

# Создание конфигурации GRUB
grub-config:
	@mkdir -p $(ISO_DIR)/boot/grub
	@echo 'menuentry "MyOS with ELF Loader" {' > $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot /boot/myos.bin' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg

# Создание ISO образа
$(ISO_FILE): $(KERNEL_BIN) grub-config
	@mkdir -p $(ISO_DIR)/boot
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/myos.bin
	grub-mkrescue -o $(ISO_FILE) $(ISO_DIR)

# Сборка только ядра
kernel: $(KERNEL_BIN)

# Запуск в QEMU (из bin файла)
run-bin: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN)

kernel64: $(KERNEL64_BIN)

run64: $(KERNEL64_BIN)
	qemu-system-x86_64 -kernel $(KERNEL64_BIN)

# Запуск в QEMU (из ISO образа)
run-iso: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE)

# Запуск в QEMU с VNC (для удаленного доступа)
run-vnc: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -vnc :1 -daemonize

# Очистка
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(ISO_FILE)
	rm -f $(ISO_DIR)/boot/myos.bin

# Полная очистка и пересборка
rebuild: clean all

# Показать структуру проекта
tree:
	@echo "Структура проекта MyOS с ELF-загрузчиком:"
	@echo "├── src/"
	@echo "│   ├── boot/"
	@echo "│   │   └── boot.asm          # Загрузчик с Multiboot заголовком"
	@echo "│   ├── kernel/"
	@echo "│   │   ├── kernel.c          # Основной код ядра с ELF-загрузчиком"
	@echo "│   │   ├── interrupts.asm    # Обработчики прерываний"
	@echo "│   │   ├── context.asm       # Переключение контекста задач"
	@echo "│   │   ├── syscalls.asm      # Системные вызовы"
	@echo "│   │   └── usermode.asm      # Поддержка пользовательского режима"
	@echo "│   ├── include/"
	@echo "│   │   ├── elf.h             # Определения ELF структур"
	@echo "│   │   └── types.h           # Базовые типы данных"
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
	@echo "MyOS с ELF-загрузчиком - Build System"
	@echo "====================================="
	@echo "Исходники:"
	@echo "  Boot:       $(BOOT_ASM)"
	@echo "  Interrupts: $(INTERRUPTS_ASM)"
	@echo "  Kernel:     $(KERNEL_C)"
	@echo "  Context:    $(CONTEXT_ASM)"
	@echo "  Syscalls:   $(SYSCALLS_ASM)"
	@echo "  User Mode:  $(USER_MODE_ASM)"
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
	@echo "  make run-vnc   - Запустить с VNC доступом"
	@echo "  make clean     - Очистить сборку"
	@echo "  make rebuild   - Пересобрать с нуля"
	@echo "  make tree      - Показать структуру"
	@echo "  make info      - Эта информация"

# Помощь
help: info

.PHONY: all kernel clean rebuild run-bin run-iso run-vnc grub-config tree info help