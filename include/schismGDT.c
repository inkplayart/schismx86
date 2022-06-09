#include "schismGDT.h"

/**
 * \param target A pointer to the 8-byte GDT entry
 * \param source An arbitrary structure describing the GDT entry
 	
 	This code was taken entirely from osdev's wiki
 
 */
void encodeGDTEntry(uint8_t *target, gdtEntry source)
{
    // Check the limit to make sure that it can be encoded (in a 32-bit OS, this indicates it's too big)
    if ((source.limit > 65536) && ((source.limit & 0xFFF) != 0xFFF)) {
        kernel_printf("You can't do that!");
    }
    //set us up for page granularity (the if statement below is not useful for Schism)
    target[6] = 0xC0;
    source.limit = source.limit >> 12;
    /*
    if (source.limit > 65536) { //if this is the case then we necessarily use 4KiB page granularity
        // Adjust granularity if required
        source.limit = source.limit >> 12;
        
    } else {
        target[6] = 0x40;
    }*/
 
 	//the encodings - just go with these, trust
    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] |= (source.limit >> 16) & 0xF;
 
    // Encode the base 
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;
 
    // And... Type
    target[5] = source.type;
}

void allocateGDT(uint32_t nEntries)
{
	GDT = (uint8_t*)kernel_malloc(nEntries*GDT_ENTRY_SIZE);
}

void addGDTEntry(uint32_t entryNumber,gdtEntry source)
{
	//just encode at the right offset - find the pointer at the right place and go for it
	encodeGDTEntry(GDT + GDT_ENTRY_SIZE*entryNumber,source);
}

//this function is SUPER kernel specific, so I'm just hard coding the GDT
void createGDT(ramData usableRam)
{
	kernel_printf("Creating the GDT.  Note: Only kernelspace will exist right now\n Usable RAM is: %u",usableRam);
	//first, allocate the GDT
	allocateGDT(SCHISM_GDT_ENTRIES);
	
	//the first entry must be NULL for x86 purposes
	gdtEntry nullEntry = {.base = 0, .limit = 0, .access = 0, .flags = 0,.type = 0};
	addGDTEntry(0,nullEntry);
	
	//now we need kernel code.  We're mapping to all of memory here
	gdtEntry kernelCode = {.base=0,.limit=0xFFFFFFFF,.type=GDT_TYPE_CODE_XR,.access=0,.flags = 0};
	gdtEntry kernelData = {.base=0,.limit=0xFFFFFFFF,.type=GDT_TYPE_DATA_RW,.access=0,.flags = 0};
	//gdtEntry kernelCode = {.base=usableRam.kernelspaceStart,.limit=usableRam.kernelspaceSize,.type = GDT_TYPE_CODE_XR,.access=0,.flags=0};
	//now kernel data
//	gdtEntry kernelData = {.base=usableRam.kernelspaceStart,.limit=usableRam.kernelspaceSize,.type = GDT_TYPE_DATA_RW,.access=0,.flags=0};

	//encode them
	addGDTEntry(1,kernelCode);
	addGDTEntry(2,kernelData);
	
	//user code and data
//	gdtEntry userCode = {.base=usableRam.userspaceStart,.limit=usableRam.userspaceSize,.type=GDT_TYPE_CODE_XR,.access=0,.flags=0};
//	gdtEntry userData = {.base=usableRam.userspaceStart,.limit=usableRam.userspaceSize,.type=GDT_TYPE_DATA_RW,.access=0,.flags=0};
	//encode
//	addGDTEntry(3,userCode);
//	addGDTEntry(4,userData);
	
	//now TSS, which we'll leave as a data segment until we do multitasking/
//	gdtEntry TSS = {.base=usableRam.TSSStart,.limit=usableRam.TSSSize,.type=GDT_TYPE_DATA_RW,.access=0,.flags = 0};
//	addGDTEntry(5,TSS);
	
}
