#!/bin/bash

#set it so that a failed command exits the script
set -e

export PREFIX="../../opt/cross"
export PATH="$PREFIX/bin:$PATH"

echo "Cleaning directory"
if test -f *.o; then
	rm *.o
fi

echo "Compiling code"
gcc -I../libc helloWorld.c ../libc/libc.a -o helloWorld.o -fpie -pie -std=gnu99 -nostartfiles -ffreestanding --no-standard-libraries -static-pie -Tlink.ld
echo "Done"
