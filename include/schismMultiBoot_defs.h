#ifndef MULTIBOOT_DEFS
#define MULTIBOOT_DEFS

//store multiboot stuff in a struct
typedef struct{
	uint32_t* headerAddr;
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t* mods_addr;
	
	//these next four depend on which flag is set, so we'll just call them
	//syms for now, for "symbols", but that's not strictly correct
	uint32_t symsLow;
	uint32_t symsMed1;
	uint32_t symsMed2;
	uint32_t symsHigh;
	
	uint32_t mmap_length;
	uint32_t* mmap_addr;
	
	uint32_t drives_length;
	uint32_t* drives_addr;
	
	uint32_t* config_table;
	
	//note there are a TON more things, but we aren't interested in them now.  We'll add them
	//all as needed
}multiBootHeader;

#endif
