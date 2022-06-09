#!/bin/bash

#set it so that a failed command exits the script
set -e

export PREFIX="../../opt/cross"
export PATH="$PREFIX/bin:$PATH"

echo "Cleaning directory"
if test -f *.o; then
	rm *.o
fi

echo "Compiling stdio"
gcc -c stdio.c -o stdio.o -std=gnu99 -ffreestanding -fpic -O2 -Wall -Wextra
echo "Done"
echo "Compiling syscall"
gcc -c syscall.c -o syscall.o -std=gnu99 -ffreestanding -fpic -O2 -Wall -Wextra
echo "Done"
echo "Compiling crt0"
gcc -c crt0.c -o crt0.o -std=gnu99 -ffreestanding -fpic -O2 -Wall -Wextra
echo "Done"
echo "Archiving into libc.a"
ar -cvq libc.a *.o
echo "Done"

echo "+++++++++++++"
echo "=============================="
echo "Done! libc is ready to go"
echo "=============================="
echo "+++++++++++++"
