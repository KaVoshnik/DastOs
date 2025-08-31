#!/bin/bash
# clean.sh

echo "Cleaning up..."
make clean
rm -f *.bin *.o
echo "Done."