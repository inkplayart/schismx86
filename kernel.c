#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
//cpuid testing
#include <cpuid.h>

#include "schismIOPort.h"
#include "schismKernelIO.h"
#include "schismPCI.h"
#include "schismPS2.h"
#include "schismAHCI.h"
#include "schismMultiBoot.h"
#include "stdlib.h"
#include "kernel_util.h"
#include "schismGDT.h"
#include "schism_PIC.h"
#include "schism_IDT.h"
#include "ISR_Test.h"
#include "schismATA.h"
#include "xsfs.h"

//various standard-library system interfaces
#include "fcntl.h"
#include "_stdio.h"

//program loader
#include "extractElf.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif


extern uintptr_t heapLoc;
uint32_t* heapBottom = &heapLoc; //we can sort of put this here

extern volatile uint32_t* st;
extern volatile uint32_t* start_start;
extern volatile uint32_t* multibootHeaderLocation; 

//declare the memory detector (eventually move this to a library
//extern do_e820();
//#define MEM_MAP_LOC 0x8000
 
/* Example: Get CPU's model number.  I have ZERO clue why, but
	the input arguments make zero sense, but such is the magic of 
	inline assembly.
 */
void get_cpuData(unsigned int inputCode, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
	__get_cpuid(inputCode, eax, ebx, ecx, edx);
}



void createCharArrayFromUInt(unsigned int a,char* outChar,int startIndex)
{
	char ls = a >> 24;
	char mb1 = (a>>16)&0xFF;
	char mb2 = (a>>8)&0xFF;
	char msb = a&0xFF;
	
	outChar[startIndex] = msb;
	outChar[startIndex+1] = mb2;
	outChar[startIndex+2] = mb1;
	outChar[startIndex+3] = ls;
}

/*
	Provides the vendor string in outChar.
	
	The return value is the highest function callable with cpuID for this chip
*/
int get_cpuModel(unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx,char* outChar)
{
	get_cpuData(0,eax,ebx, ecx, edx);
	
	createCharArrayFromUInt(*ebx,outChar,0);
	createCharArrayFromUInt(*edx,outChar,4);
	createCharArrayFromUInt(*ecx,outChar,8);
	
	return *eax;
}

static inline bool are_interrupts_enabled()
{
    unsigned long flags;
    asm volatile ( "pushf\n\t"
                   "pop %0"
                   : "=g"(flags) );
    return flags & (1 << 9);
}

int test = 0;

//the master heap record.  It belongs to the kernel
heapData masterHeap;

//the kernel's virtual file system
virtualFileSystem* kernel_vfs;

//kernel's three streams
FILE* kstdin;
FILE* kstdout;
FILE* kstderr;

//standard devices
keyboard curInputDev;

void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();
	
	kernel_printf("Schism Initialized.  Ready to Discover \n");
	
	//obtain the multiboot header
	multiBootHeader mbh;
	
	//the cast below is to avoid the problem of discarded qualifiers - multibootHeaderLocation is
	//listed as volatile because it is modified by the bootloader, but the function asks for non-volatile stuff
	_MB_setFlagsAndAddr((uint32_t*)multibootHeaderLocation, &mbh);
	
	_MB_FillHeader(&mbh);
	
	
	//check on the multiboot modules
	
	/*
		OK, so here's the deal: Multiboot modules are just sets of bytes, so we can
		in theory create programs on Linux, copy them byte-for-byte, and run them.
		
		This is good. The code to access it is a bit weird: mbh.mods_addr contains the
		address of the modules. It is, once again, a pointer-to-a-pointer. It points, I think,
		to the first module's starting byte once you dereference it. So mods_addr actualy points to the address the 
		holds the first modules starting byte. Anyway, see below. When that code runs the initrd is printed.
		
		Now the problem: Modules have to be loaded before you screw with RAM, or you need to figure out a place
		to put them. Either way you can't overwrite the data even using your malloc until
		you've loaded all the modules off. This is a chicken-and-egg: malloc is used to discover the HDD, which
		will eventually store the modules. 
		
		Ah, an option presents itself: We know where RAM ends (well, multiboot does and we can figure it out from
		initRamData). We also know how big the modules are AND where the heap starts and how big it is. Since the kernel heap is 
		temporary anyway, we can just make a pointer that is far enough away from the heap but not too close to ramEnd, 
		copy the bytes over, allocate the heap and keep the modules in their own structure, process the modules, then 
		we can overwrite them all we want.
	*/
	int modsCount = mbh.mods_count;
	uint32_t* modsAddr = mbh.mods_addr;
	
	kernel_printf("Mods count: %u at address %u\n",modsCount,*(uint32_t*)modsAddr);
	
	/*
		OK, so the problem now is that I need to move the program I have loaded as initrd. In general this won't really be needed
		since we are just loading these things for now and I need to put them somewhere, I"m just going to memcpy the
		binary. However, I need to know how big it is first, so hopefully I can do all of that without resorting to the heap
	*/
	Elf32_Ehdr executable;
	uint8_t* elfFile = (uint8_t*)(*modsAddr);
	//read the header
	readHeader(elfFile,&executable);
	
	//OK, so now we know where it is...why don't we just move it over right now?
	uint8_t* progLoc = (uint8_t*) 16777216; //TOTAL magic number right now, this will be generalized later
	
	//create the header table
	Elf32_Shdr* headerTab = getSectionHeaders(elfFile,executable);
	createFlatBinaryAtLocation(elfFile,headerTab,executable,progLoc);
	
	//should be loaded, let's check
	uint32_t prn = progLoc[4];
	kernel_printf("Entry address: %u",executable.e_entry);
	
	//Get ready for RAM data
	initRamData(mbh,&kernelMasterRam);
	
	//now initialize malloc.  It's heap time!  Let's make a 4MB kernel heap for now
	initKernelMalloc(&masterHeap,kernelMasterRam,KERNEL_HEAP_MAX);
	
	//now rock the GDT
	kernel_printf("Initializing and Loading GDT\n");
	createGDT(kernelMasterRam);
	
	//that "24" tells us that the current GDT is 24 bytes.  This should be nGDTEntries*8,
	//since each entry is 8 bytes long.  My current GDT has 3 entries.  This is 
	//kludgy...
	setGDT(GDT,24);
	reloadSegments();
	
	masterRecord kernelMaster;
	kernelMaster.heapptr = (&masterHeap);
	kernelMaster.mbootheader = (&mbh);
	kernelMaster.pciptr = (pciRecord*)kernel_malloc(sizeof(pciRecord));

	(kernelMaster.pciptr)->nextRecord = (pciRecord*)0xFFFFFFFF; //Special value to indicate it's new

	//check PCI bus and PS2
	_PCI_enumerate(kernelMaster.pciptr);
	_PS2_CheckDevice();
	

	kernel_printf("Setting up interrupts\n");
	createIDT(IDT_NUM_INTERRUPTS); //why not?
	
	//create for me a new IDT Entry
	IDT_entry keyboardIDT ={.offset = (uint32_t)&isr_keyboard,.selector = KERNEL_CODE_SEGMENT,.gate = INTERRUPT_GATE,.DPL = RING_0,.present=VALID_DESCRIPTOR};
	IDT_entry genericIDT ={.offset = (uint32_t)&isr_generic,.selector = KERNEL_CODE_SEGMENT,.gate = INTERRUPT_GATE,.DPL = RING_0,.present=VALID_DESCRIPTOR};
	IDT_entry sysCallIDT ={.offset = (uint32_t)&sysCallHandler,.selector = KERNEL_CODE_SEGMENT,.gate = INTERRUPT_GATE,.DPL = RING_0,.present=VALID_DESCRIPTOR};	

	//set up the IDT with generic functions
	for(int i = 0; i < IDT_NUM_INTERRUPTS - 0x41; i++)
		packIDTEntry(genericIDT,i);
	
	//keyboard interrupt
	packIDTEntry(keyboardIDT,1);
	
	//system call interrupt
	packIDTEntry(sysCallIDT,SYS_CALL_OFFSET); //Note that this will be placed at SYS_CALL_OFFSET+PIC1_SCHISM_OFFSET
	
	//set up PIC
	PIC_standard_setup();
	
	//enable the keyboard interrupt
	IRQ_enable(1);
	
	//disable the timer interrupt
	IRQ_disable(0);
	
	asm("sti");

	ahcihba hba;

	//get the correct BDF address
	_AHCI_getBDF(kernelMaster.pciptr,&hba);
	
	_AHCI_getBaseAddress(&hba);
	_AHCI_initDeviceList(&hba);
	_AHCI_configure(&hba);
	

	//kernel_printf("Creating the standard three streams.\n Currently they are: out %u, in %u, err %u\n",kstdout,kstdin,kstderr);
	//create the three standard streams
	kstdin = fopen("kstdin","rw");
	kstdout = fopen("kstdout","rw");
	kstderr = fopen("kstderr","rw");
	
		//attach the current input device
	curInputDev.dev = kstdin;
	curInputDev.avail = 0;
	
	kernel_printf("It's time to launch the executable \n");
	FILE* standardIOFiles[2];
	standardIOFiles[0] = kstdin;
	standardIOFiles[1] = kstdout;
	void (*exec)(void*) = executable.e_entry;
	
	(*exec)(standardIOFiles);
	
	kernel_printf("Writing a sector: ");
	char* str = "Sector 1";
	_ATA_writeSector(&hba,1,str,18);
	char* outString = _ATA_readSector(&hba,14);
	
	terminal_writestring(outString);
	
	//time to explore the disk a bit before using it for stuff
	_ATA_initHDD(&hba);
	ahciDevice* curDev = _AHCI_getHDD(hba);
	kernel_printf("Number sectors: %u, sector size %u, and drive size %u",curDev->numSectors,curDev->sectorSize,curDev->driveSize);
	
	//Now it's time to play with files
	hdd drive;
	drive.bytesPerSector = curDev->sectorSize;
	drive.totSectors = curDev->numSectors;
	drive.hostbus = &hba;
	
	//Now we actually need to create an initial record. 
	kernel_printf("HDD Time drive!\n");
	//clearFS(&drive);
	
	/*kernel_printf("Writing files!\n");
	
	createFile(120,"They are goat",0,&drive);
	createFile(513,"Werr be goat",0,&drive);
	
	kernel_printf("Listing files: \n");
	listFilesByName(&drive);
	
	uint8_t ret = deleteFile("Werr be goat",0,&drive);
	
	if(ret == FS_SUCCESS)
	{
		kernel_printf("Grand success\n");
	}
	
	ret = deleteFile("Umlaut",0,&drive);
	
	
	if(ret == NO_FILE)
	{
		kernel_printf("Did it\n");
	}
	
	
	
	listFilesByName(&drive);
	
	kernel_printf("Reading a file!\n");
	
	//OK, now try to write a file
	updateFile(15,"1234567AAABCDE","We are goat",0,&drive);
	
	//we need to read trhe sector, we aren't ready yet for anything else
	uint8_t* fileRead = readFile("We are goat",0,&drive);
	terminal_writestring(fileRead);
	
	char* newFile = (char*)kernel_malloc(1024);
	for(int i = 0; i < 1024;i++)
	{
		newFile[i] = 65 + i%26;
	}
	//createFile(1024,"BigFile",0,&drive);
	updateFile(10,"UMLAUTZIA","BigFile",0,&drive);
	fileRead = readFile("BigFile",0,&drive);
	terminal_writestring(fileRead);
	
	kernel_printf("\n");
	listFilesByName(&drive);
	kernel_printf("Done.  Schism Ended.");	
	*/
	
	kernel_printf("Sizeof char: %d\n",sizeof(char));
	kernel_printf("Sizeof bool: %d\n",sizeof(bool));
	kernel_printf("Sizeof short: %d\n",sizeof(short int));
	kernel_printf("Sizeof int: %d\n",sizeof(int));
	kernel_printf("Sizeof long int: %d\n",sizeof(long int));
	kernel_printf("Sizeof long long int: %d,\n",sizeof(long long int));
	
	char cds = 128;
	kernel_printf("Do I exist as a char? %d\n",cds);
	
}
