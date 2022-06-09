#include "kernel_util.h"
//this function finds the start address and size of usable RAM.  It will come in VERY handy
void initRamData(multiBootHeader mbh,ramData* masterRam)
{
	
	//set masterRam to have null as RAM Start, in case the memory finding fails (is that even possible?)
	masterRam->ramStart = 0;
	
	//OK, this is fairly "simple" - find the memory region that contains the
	//heap bottom, which is a global, and then return it
	
	uint32_t byteCount = 0; //gets us out of the while loop
	int i = 0; //keeps track of where in the structure we are
	while(byteCount < mbh.mmap_length)
	{
		//need to know how big this region is to increment byteCount
		uint32_t memregionSize = mbh.mmap_addr[i];
		i+=1; //get ready to read the next uint32_t
		
		uint32_t baseAddr = mbh.mmap_addr[i] + mbh.mmap_addr[i+1];
		i+=2;
		uint32_t size = mbh.mmap_addr[i] + mbh.mmap_addr[i+1];
		i+=2;
		
		//the last part is the type of memory
		uint32_t type = mbh.mmap_addr[i];
		i++;
		
		//increment byteCount: add the toal bytes of the memory region, plus
		//a sizeof(uint32_t) to account for the size variable of the region
		byteCount += memregionSize + sizeof(uint32_t);
		
		//figure out the final address of the region
		uint32_t endAddr = baseAddr + size;
		
		//now check to see if our pointer is in the right range and the type is available RAM
		if((heapBottom > (uint32_t*)baseAddr) && (heapBottom < (uint32_t*)endAddr) && type == 1)
		{
			//we've found our region - but this is sort of wrong.  Size is the size of all usable RAM...it
			//includes the kernel and other things...need to figure this out better
			masterRam->ramStart = (uintptr_t)baseAddr;
			masterRam->ramSize = size; //in Bytes
			
			uint32_t tempSize;
			
			//next we need to figure out kernelspace size.  This is (usableRam.ramSize)/4
			masterRam->kernelspaceSize = (masterRam->ramSize)/4; //this is now in Bytes
			tempSize = masterRam->kernelspaceSize; //need this in bytes later
			masterRam->kernelspaceSize = masterRam->kernelspaceSize/PAGE_SIZE; //now this is in pages
			masterRam->kernelspaceStart = (uintptr_t)baseAddr;
			
			//now figure out TSS size, which is 1KiB
			masterRam->TSSSize = 1; //it's 1 page, which is the smallest I can make it
			masterRam->TSSStart = (uintptr_t)((uint8_t*)masterRam->kernelspaceStart + tempSize);
			
			//finally, figure out userspace size, which is whatever's left over, again in pages NOT bytes
			masterRam->userspaceSize = (masterRam->ramSize/PAGE_SIZE) - masterRam->kernelspaceSize - masterRam->TSSSize;
			masterRam->userspaceStart = (uintptr_t)((uint8_t*)masterRam->TSSStart + PAGE_SIZE); //we need to add 1 whole page
			//Now set their base addresses, which the GDT will eventually use
			
			return;
		}
	}
	
	return; //theoretically we can never get here...
}

/*
void systemCall(int sysCallNo,void* ptr)
{
	//we need to put the input arguments into registers so they are accessible via
	//the system call
	__asm__ volatile("MOV %0, %%EAX" :: "" (sysCallNo));
	__asm__ volatile("MOV %0, %%ECX" :: "" (ptr));
	__asm__ volatile ("INT $0xC0"); //call interrupt 0x80
}
*/
void systemCall( int sysCallNo, void * ptr )
{
	//we need to put the input arguments into registers so they are accessible via
	//the system call. the "a" puts sysCallNo into EAX and "c" puts ptr into ECX.
	//the "memory" thing means that "This instruction may overwrite (clobber) stuff
	//in registers, so do not cache any memory stuff in the registers until we are done"
	//it's an instruction to the compiler itself to not use the registers until we are done
    asm volatile( "int $0xc0" :: "a"(sysCallNo), "c"(ptr) : "memory" );
}

//print whatever is at the file position of ptr, then decrement the pointer and end
void syscall_putchar(FILE* ptr)
{
	terminal_putchar((ptr->buf)[ptr->filePos-1]);
	ptr->filePos -= 1; //decrement
}

//get a character
void syscall_getchar(FILE* ptr)
{
	ptr->buf[ptr->filePos] = kernel_getch();
}

void sysCallC(int sysCallNo,void*ptr)
{
	//Now we can write in pure C. Thankfully no more assembly is required to handle the
	//system calls. What follows is a giant if statement that calls various system call functions
	
	if(sysCallNo == OPEN_CALL) //it's a call to open in fcntl.h
		open(ptr);
	else if(sysCallNo == PUTC_CALL) //it's a call to terminal_putchar eventually
		syscall_putchar((FILE*)ptr);
	else if(sysCallNo == GETCHAR_CALL) //we are trying to get the char
		syscall_getchar((FILE*)ptr);
}