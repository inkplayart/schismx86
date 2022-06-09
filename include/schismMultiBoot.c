#include "schismMultiBoot.h"


void _MB_setFlagsAndAddr(uint32_t* addr, multiBootHeader* mbh)
{
	mbh->flags = *addr; //what is stored at the address is the flags variable
	mbh->headerAddr = addr;
}

//this will be expanded as we go
void _MB_FillHeader(multiBootHeader* mbh)
{
	//there isn't much more to do here but straight passthrough
	mbh->mem_lower = mbh->headerAddr[1];
	mbh->mem_upper=mbh->headerAddr[2];
	mbh->boot_device = mbh->headerAddr[3];
	mbh->cmdline = mbh->headerAddr[4];
	mbh->mods_count = mbh->headerAddr[5];
	mbh->mods_addr = (uint32_t*)mbh->headerAddr[6];
	mbh->symsLow = mbh->headerAddr[7];
	mbh->symsMed1 = mbh->headerAddr[8];
	mbh->symsMed2 = mbh->headerAddr[9];
	mbh->symsHigh = mbh->headerAddr[10];
	mbh->mmap_length = mbh->headerAddr[11];
	mbh->mmap_addr = (uint32_t*)mbh->headerAddr[12];
}

void _MB_printHeader(multiBootHeader mbh)
{
	kernel_printf("Header location: %u \n",mbh.headerAddr);
	
	kernel_printf("mem lower: %u, mem upper: %u\n",mbh.mem_lower,mbh.mem_upper);
	kernel_printf("Boot device: %u \n",mbh.boot_device);
	kernel_printf("cmdline: %u \n",mbh.cmdline);
	kernel_printf("mods_count: %u mods_addr: %u \n",mbh.mods_count,mbh.mods_addr);
	
	//this one will be a for loop, it's easier
	for(int i = 0; i < 4; i++)
		kernel_printf("Syms %d: %u ",i,mbh.headerAddr[7+i]);
	kernel_printf("\n");
	
	kernel_printf("mmap_length: %u \n",mbh.mmap_length);
	kernel_printf("mmap_addr: %u \n",mbh.mmap_addr);
}

void _MB_printMMap(multiBootHeader mbh)
{
	kernel_printf("Memory map length: %u \n",mbh.mmap_length);

	kernel_printf("Memory map address: %u \n",mbh.mmap_addr);
	
	uint32_t byteCount = 0; //gets us out of the while loop
	int i = 0; //keeps track of where in the structure we are
	while(byteCount < mbh.mmap_length)
	{
		uint32_t memregionSize = mbh.mmap_addr[i];
		
		i+=1; //get ready to read the next uint32_t
		
		kernel_printf("Region: ");
		
		//now, there are memRegionSize bytes to read, which we'll read as uint32_t's,
		//so 4 bytes each
		/*for(uint32_t j = 0; j < memregionSize/4-1; j++)
		{
			kernel_printf("%u, ",mbh.mmap_addr[i+j]);
		}*/
		
		uint32_t baseAddr = mbh.mmap_addr[i] + mbh.mmap_addr[i+1];
		kernel_printf("baddr: %u, ",baseAddr);
		i+=2;
		uint32_t size = mbh.mmap_addr[i] + mbh.mmap_addr[i+1];
		kernel_printf("Size: %u, ",size);
		i+=2;
		
		//the last part is the type of memory, so let's print that
		int type = mbh.mmap_addr[i];
		i++;
		kernel_printf("Type: ");
		if(type == 1)
			kernel_printf("Available RAM, ");
		else if(type == 3)
			kernel_printf("ACPI, ");
		else if(type == 4)
			kernel_printf("Preserve, ");
		else if(type == 5)
			kernel_printf("Defective RAM, ");
		else
			kernel_printf("Reserved, ");
		kernel_printf("\n");
		//i+=memregionSize/4;//advance the uint32_t counter to correctly address the array
		
		//increment byteCount: add the toal bytes of the memory region, plus
		//a sizeof(uint32_t) to account for the size variable of the region
		byteCount += memregionSize + sizeof(uint32_t);
	}
	
}
