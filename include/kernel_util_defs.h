#ifndef UTIL_DEFS
#define UTIL_DEFS

#include "schismPCI_defs.h"
#include "stdlib_defs.h"
#include "schismMultiBoot_defs.h"

//kernel level defines that everyone needs to know
#define KERNEL_HEAP_MAX 4194000 //4MiB heap for now
#define PAGE_SIZE 4096 //4KiB pages

#define UNINITIALIZED_RECORD 0xFFFFFFFF //uninitialized data

#define SYS_CALL_OFFSET 0x80 //the system call offset in the IDT

//system call numbers
#define OPEN_CALL 0 //I'm sure these can be changed later, or ignored.
#define PUTC_CALL 1 //why not
#define GETCHAR_CALL 2

//io stuff
#define FOPEN_MAX 1024

//The entire purpose of this is to create a set of utilities that the kernel can share
//with other things.  For instance, it will include the size and location of usable RAM...
//It will be expanded as time goes on
typedef struct ramData{
	uintptr_t ramStart;
	uint32_t ramSize;
	uint32_t TSSSize;
	uintptr_t TSSStart;
	uint32_t kernelspaceSize;
	uintptr_t kernelspaceStart;
	uint32_t userspaceSize;	
	uintptr_t userspaceStart;
}ramData;

/*
	This next struct's purpose is to store pointers to other records.
	
	Each record except heapptr is a linked list of structs, each defined
	in its respective library 
*/
typedef struct {
	heapData* heapptr;
	multiBootHeader* mbootheader; //stores the multiboot header in case we need it
	pciRecord* pciptr;
	uintptr_t ahciptr;
	uintptr_t ps2ptr;	
}masterRecord;

#endif
