#ifndef SCHISMGDT
#define SCHISMGDT
	
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "kernel_util.h"
#include "schismKernelIO.h"
#include "stdlib.h"

#define GDT_ENTRY_SIZE 8 //8 bytes per entry
	
#define SCHISM_GDT_ENTRIES 7 //1 for null, 2 code segments, 2 data segments (kernel and user), and 1 TSS

//type selectors
#define GDT_TYPE_DATA_RW 0x92
#define GDT_TYPE_CODE_XR 0x9A //execute and read
//Note that other access types are needed later, but for basic GDT we're sticking to these

typedef struct{
	uint32_t base;
	uint32_t limit;
	uint8_t access;
	uint8_t flags;	
	uint8_t type;
}gdtEntry;

uint8_t* GDT; //this is the GDT itself.  It has to be malloc'd because it has to live in memory
//only after we know what memory is

//correctly puts the right bits in the right place for a GDT entry
void encodeGDTEntry(uint8_t *target, gdtEntry source);

//allocate the correct number of bytes for nEntries entries in the GDT
//remember that each entry is 8 bytes, so we need 8*nEntries bytes allocated
void allocateGDT(uint32_t nEntries);

//adds an entry to the GDT.  entryNumber starts at 0.  Each entryNumber offsets us by 8 bytes
void addGDTEntry(uint32_t entryNumber,gdtEntry source);

//creates the whole GDT
void createGDT(ramData usableRam);

//this function lives in setGDTInCPU.s
extern void setGDT(uint8_t* GDT_Loc, uint32_t GDT_Size); 

extern void reloadSegments(); //this function lives in setGDTInCPU.s

#endif
