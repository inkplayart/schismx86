#include "schism_IDT.h"

static inline void lidt(void* base, uint16_t size)
{   // This function works in 32 and 64bit mode
    struct {
        uint16_t length;
        void*    base;
    } __attribute__((packed)) IDTR = { size, base };
 
    asm ( "lidt %0" : : "m"(IDTR) );  // let the compiler choose an addressing mode
}

//create the IDT on the heap
void createIDT(uint32_t numInterrupts)
{
	kernel_printf("Creating IDT\n");
	//create it
	IDT = (IDT_entry_packed*)kernel_malloc(IDT_ENTRY_REAL_SIZE*(numInterrupts + PIC1_SCHISM_OFFSET));
	//tell the CPU it exists
//	setIDT((uint8_t*)IDT,IDT_ENTRY_REAL_SIZE*numInterrupts);
	lidt(IDT,IDT_ENTRY_REAL_SIZE*numInterrupts);
}

//pack an entry and put it onto the IDT
void packIDTEntry(IDT_entry entr,uint32_t interruptNum)
{
	
	//figure out where in the IDT we need to be
	//IDT_entry_packed* idtpacked = IDT + interruptNum * IDT_ENTRY_REAL_SIZE;
	IDT_entry_packed* idtpacked = &IDT[interruptNum + PIC1_SCHISM_OFFSET];
	idtpacked->IDTPacked &= 0; //set it to zero...but deep concern.  Where is it being placed?

	
	//put in the ISR offset first
	uint64_t highBytesOffset = entr.offset>>16;
	uint64_t lowBytesOffset = entr.offset&0b1111111111111111; //sketch but it works
	
	//pack them in
	idtpacked->IDTPacked |= highBytesOffset << 48;
	idtpacked->IDTPacked |= lowBytesOffset;
	
	idtpacked->IDTPacked |= (uint64_t)1<<47; //the present bit
	idtpacked->IDTPacked |= ((uint64_t)entr.DPL&0b11)<<45;
	idtpacked->IDTPacked |= ((uint64_t)entr.gate&0b1111)<<40;
	
	idtpacked->IDTPacked |= ((uint64_t)entr.selector)<<16;
}

