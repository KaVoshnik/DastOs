#!/bin/bash
# run.sh

echo "Building DastOS..."
make

if [ $? -eq 0 ]; then
    echo "Running DastOS in QEMU..."
    qemu-system-i386 -kernel dastos.bin
else
    echo "Build failed!"
fi