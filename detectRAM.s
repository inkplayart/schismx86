//Use the INT 0x15, eax=0xE820 BIOS function to get a memory map

.set mmap_ent, 0x8000 //This is where we will store the memory map 
do_820:
	mov $0xBAD, %eax //move is SRC then DEST
	mov $mmap_ent, %ebx //move the pointer mmap_ent to ebx
	mov %eax, [%ebx] //move what's in eax to the memory location pointed at by ebx
	ret
