#!/bin/bash
# test_dastos.sh - Script to build, test DastOs in QEMU, and clean up

# Build the OS
make all

# Run in QEMU with auto-check (simple boot check via exit code)
qemu-system-i386 -fda dastos.img -display none -monitor none -serial file:output.log &
QEMU_PID=$!

# Wait a bit for boot, then check if it booted (e.g., look for welcome message)
sleep 5
if grep -q "Welcome to DastOs!" output.log; then
    echo "Test passed: OS booted successfully."
else
    echo "Test failed: OS did not boot as expected."
fi

# Kill QEMU
kill $QEMU_PID

# Clean up
make clean
rm -f output.log
echo "Cleanup complete."