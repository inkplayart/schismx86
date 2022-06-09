#ifndef SCHISM_IDT
#define SCHISM_IDT

#include "schismKernelIO.h"
#include "stdlib.h"
#include "schism_PIC.h"

//definitions
#define INTERRUPT_GATE 0xE
#define RING_0 0
#define VALID_DESCRIPTOR 1
#define IDT_ENTRY_REAL_SIZE 8 //IDT entries need to be properly aligned and are 8 bytes long
#define NUM_VECTORS 256 //there are 256 possible interrupts and the IDT must be this size?
#define KERNEL_CODE_SEGMENT 0b1000

#define IDT_NUM_INTERRUPTS 256

//define the IDT entry structure

typedef struct {
	uint32_t offset; //the location of the ISR
	uint16_t selector; //the segment selector
	uint8_t gate; //the gate type.
	uint8_t DPL; //the ring level, should always be zero
	uint8_t present; //present bit, is 1 for valid descriptors
}IDT_entry;

//the IDT Entry is useful to set things in human-readable form. This
//packed version is what is actually needed for the IDT to make sense
typedef struct{
	uint64_t IDTPacked;
}IDT_entry_packed;

IDT_entry_packed* IDT; //this is the IDT itself.  It must be initialized prior to use

//this function lives in setGDTInCPU.s
extern void setIDT(uint8_t* IDT_Loc, uint32_t IDT_Size); 


void createIDT(uint32_t numInterrupts);
void packIDTEntry(IDT_entry entr,uint32_t interruptNum);
#endif
