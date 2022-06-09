#!/bin/bash

#set it so that a failed command exits the script
set -e

export PREFIX="../opt/cross"
export PATH="$PREFIX/bin:$PATH"

echo "Cleaning directory"
if test -f *.o; then
	rm *.o
fi
if test -f "schism.iso"; then
	rm schism.iso
fi
echo "Done."
echo "Assembling boot.s"
i686-elf-as  boot.s -o boot.o
echo "Done"
#echo "Assembling memory detector"
#i686-elf-as detectRAM.s -o do_e820.o
#echo "Done"

echo "Compiling kernel.c"
i686-elf-gcc -Iinclude -Isys -Ischismlibc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling IOPort Library"
i686-elf-gcc -Iinclude -c include/schismIOPort.c -o schismIOPort.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling PCI Library"
i686-elf-gcc -Iinclude -c include/schismPCI.c -o schismPCI.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling Kernel IO Library"
i686-elf-gcc -Iinclude -c include/schismKernelIO.c -o schismKernelIO.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling PS/2 Library"
i686-elf-gcc -Iinclude -c include/schismPS2.c -o schismPS2.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling AHCI Library"
i686-elf-gcc -Iinclude -c include/schismAHCI.c -o schismAHCI.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
echo "Done"
echo "Compiling ATA Library"
i686-elf-gcc -Iinclude -c include/schismATA.c -o schismATA.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
echo "Done"
echo "Compiling Multiboot Library"
i686-elf-gcc -Iinclude -c include/schismMultiBoot.c -o schismMultiBoot.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling Kernel Utils"
i686-elf-gcc -Iinclude -Isys -Ischismlibc -c include/kernel_util.c -o kernel_util.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling Standard Library"
i686-elf-gcc -Iinclude -c include/stdlib.c -o stdlib.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling ELF Loader Library"
i686-elf-gcc -Iinclude -c include/extractElf.c -o extractElf.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling XSFS Library"
i686-elf-gcc -Iinclude -c include/xsfs.c -o xsfs.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling GDT Library"
i686-elf-gcc -Iinclude -Isys -Ischismlibc -c include/schismGDT.c -o schismGDT.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
#two steps: First the C, then the asm
echo "...........Assembling GDT ASM"
i686-elf-as  setGDTInCPU.s -o setGDTInCPU.o
echo "...........Assembling CS Reconfigurator"
i686-elf-as reload_CS_CPU.s -o reload_CS_CPU.o
echo "Done"
echo "Compiling Interrupt Library"
echo "...........Compiling PIC Lib"
i686-elf-gcc -Iinclude -c include/schism_PIC.c -o schism_PIC.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "............Compiling IDT Lib"
i686-elf-gcc -Iinclude -Isys -Ischismlibc -c include/schism_IDT.c -o schism_IDT.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "..............Assembling IDT ASM"
i686-elf-as setIDTInCPU.s -o setIDTInCPU.o
echo "..............Assembling keyboard ISR"
i686-elf-as include/keyboardInterrupt.s -o keyboardInterrupt.o
echo "..............Assembling generic ISR"
i686-elf-as include/genericInterrupt.s -o genericInterrupt.o
echo "Done"
echo ".............Assembling system call"
i686-elf-as include/sysCallInterrupt.s -o sysCallInterrupt.o
echo "Done"
echo "Compiling ISR library"
i686-elf-gcc -Iinclude -Isys -Ischismlibc  -c include/ISR_Test.c -o ISR_Test.o -std=gnu99 -ffreestanding -mgeneral-regs-only -O2 -Wall -Wextra 
echo "Done"
echo "Compiling mem explore library"
i686-elf-gcc -Iinclude -c memExplore/memExplore.c -o memExplore.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling stdio"
i686-elf-gcc -Iinclude -Isys -Ischismlibc -c schismlibc/_stdio.c -o stdio.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Compiling fcntl"
i686-elf-gcc -Iinclude -Isys -Ischismlibc -c sys/fcntl.c -o fcntl.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
echo "Done"
echo "Linking kernel"
i686-elf-gcc -T linker_updated.ld -o schism.bin -ffreestanding -O2 -nostdlib *.o -lgcc
echo "Done"
echo "Moving bin files"
cp schism.bin iso/boot/schism.bin
echo "Done"
echo "Creating bootable iso"
sudo grub-mkrescue -o schism.iso iso
echo "Done"
echo "Copying to host"
cp schism.iso /mnt/c/Users/tienb
echo "+++++++++++++"
echo "=============================="
echo "Done! Schism is ready to go"
echo -n "Total SLOC Count is: "
( find ./ -name '*.c' -print0 -o -name '*.h' -print0 -o -name '*.s' -print0 -o -name '*.sh' -print0 | xargs -0 cat ) | wc -l
echo "=============================="
echo "+++++++++++++"