#!/bin/bash

#set it so that a failed command exits the script
set -e

export PREFIX="../../opt/cross"
export PATH="$PREFIX/bin:$PATH"

echo "Cleaning directory"
if test -f *.o; then
	rm *.o
fi

echo "Diagnostic stuff"
i686-elf-gcc -v -print-prog-name=ld
i686-elf-gcc -dumpspecs
echo "Done"

echo "Compiling code"
i686-elf-gcc -I../libc helloWorld.c ../libc/libc.a -o helloWorld.o  -std=gnu99 -nostdlib -ffreestanding --no-standard-libraries -Tlink.ld
#i686-elf-gcc -I../libc helloWorld.c ../libc/libc.a -o helloWorld.o -std=gnu99 -ffreestanding -nostdlib -fpic -static-pie
#gcc -m32 -fcf-protection=none -I../libc helloWorld.c ../libc/stdio.c ../libc/syscall.c ../libc/crt0.c -o helloWorld.o -fpie -std=gnu99 -nostartfiles -ffreestanding --no-standard-libraries -static-pie
echo "Done"
