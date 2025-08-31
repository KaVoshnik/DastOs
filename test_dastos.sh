#!/bin/bash
# test_dastos.sh - Build, test, and clean DastOs

# Сборка
make all

# Запуск в QEMU с выводом в лог
qemu-system-i386 -fda dastos.img -display none -monitor none -serial file:output.log &
QEMU_PID=$!

# Ждём 5 секунд для загрузки
sleep 5

# Проверяем наличие приветственного сообщения
if grep -q "Welcome to DastOs!" output.log; then
    echo "Test passed: OS booted successfully."
else
    echo "Test failed: OS did not boot."
fi

# Завершаем QEMU
kill $QEMU_PID
wait $QEMU_PID 2>/dev/null

# Очистка
make clean
rm -f output.log
echo "Cleanup complete."