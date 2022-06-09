#include "extractElf.h"

//Deep copies the header from the file
void readHeader(uint8_t* elf_file,Elf32_Ehdr* header)
{
	//read the identity
	int hdrIndx = 0;
	kernel_memcpy(elf_file, header->e_ident, EI_NIDENT);
	hdrIndx += EI_NIDENT;
	
	//The next few are going to be lazy casting, since the data never exceeds the size of a byte
	header->e_type = (uint16_t)elf_file[hdrIndx];
	hdrIndx += sizeof(uint16_t);

	header->e_machine = (uint16_t)elf_file[hdrIndx];
	hdrIndx += sizeof(uint16_t);
	
	header->e_version = (uint32_t)elf_file[hdrIndx];
	hdrIndx += sizeof(uint32_t);
	
	//Really, really can't lazy cast the rest. We need it to be cast to the right size
	header->e_entry = *((uint32_t*)(elf_file+hdrIndx));
	hdrIndx += sizeof(uint32_t);
	
	header->e_phoff = *((uint32_t*)(elf_file+hdrIndx));
	hdrIndx += sizeof(uint32_t);
	
	header->e_shoff = *((uint32_t*)(elf_file+hdrIndx));
	hdrIndx += sizeof(uint32_t);
	
	header->e_flags = *((uint32_t*)(elf_file+hdrIndx));
	hdrIndx += sizeof(uint32_t);
	
	header->e_ehsize = *((uint16_t*)(elf_file+hdrIndx));
	hdrIndx += sizeof(uint16_t);
	
	header->e_phentsize = *((uint16_t*)(elf_file+hdrIndx));
	hdrIndx += sizeof(uint16_t);
	
	header->e_phnum = *((uint16_t*)(elf_file+hdrIndx));
	hdrIndx += sizeof(uint16_t);
	
	header->e_shentsize = *((uint16_t*)(elf_file+hdrIndx));
	hdrIndx += sizeof(uint16_t);
	
	header->e_shnum = *((uint16_t*)(elf_file+hdrIndx));
	hdrIndx += sizeof(uint16_t);
	
	header->e_shstrndx = *((uint16_t*)(elf_file+hdrIndx));
	hdrIndx += sizeof(uint16_t);
}

bool isValidELF(Elf32_Ehdr hdr)
{
	return hdr.e_ident[EI_MAG0] == ELFMAG0 && hdr.e_ident[EI_MAG1] == ELFMAG1 && hdr.e_ident[EI_MAG2] == ELFMAG2 && hdr.e_ident[EI_MAG3] == ELFMAG3;
}

Elf32_Shdr* getSectionHeaders(uint8_t* elf_file,Elf32_Ehdr hdr)
{
	return (Elf32_Shdr*)(elf_file + hdr.e_shoff); //This is the starting address of the array of section headers 
}

/*
	 //Gets the size for malloc
	 WARNING: This needs to be updated once we have more sophisticated binaries
	 so that things like relocation etc can happen
*/
uint32_t getFlatBinarySize(Elf32_Shdr* headerTable,Elf32_Ehdr hdr)
{
	//init
	uint32_t size = 0;
	
	//now, walk the section header table, finding all program sections, and add their sizes
	for(int i = 0; i < hdr.e_shnum; i++)
	{
		if(headerTable[i].sh_type == SHT_PROGBITS || headerTable[i].sh_type == SHT_NOBITS)
			size += headerTable[i].sh_size;
	}
	
	return size;
}

/*
	returns the actual loadable binary
	
	TODO: NOTE, this is a SUPER, SUPER simple flat binary extractor.
	It ignores relocation, it puts all of the sections right up against each other etc. It is NOT
	finished! But it will work for very simple programs and therefore for testing purposes
	
	WARNING: This function uses kernel_malloc. Eventually it needs to be re-written to use the proper
	malloc
*/
uint8_t* createFlatBinary(uint8_t* elf_file,Elf32_Shdr* headerTable,Elf32_Ehdr hdr)
{
	uint8_t* program = (uint8_t*)kernel_malloc(getFlatBinarySize(headerTable,hdr));
	//now copy. We are using locals because it's just easier on my brain
	uint32_t offset = 0; //where in the file the section starts
	uint32_t secSize = 0; //how many bytes to copy
	uint32_t progOffset = 0; //where in the program array this goes
	for(int i = 0; i < hdr.e_shnum; i++)
	{
		if(headerTable[i].sh_type == SHT_PROGBITS || headerTable[i].sh_type == SHT_NOBITS)
		{
			offset = headerTable[i].sh_offset;
			secSize = headerTable[i].sh_size;
			kernel_memcpy((elf_file+offset),(program+progOffset),secSize);
			progOffset += secSize;
		}
	}
	return program;
}

/*
	returns the actual loadable binary, loaded at a known address. This will be useful for now,
	but it requires us to know the exact right address
	
	TODO: NOTE, this is a SUPER, SUPER simple flat binary extractor.
	It ignores relocation, it puts all of the sections right up against each other etc. It is NOT
	finished! But it will work for very simple programs and therefore for testing purposes
	
	WARNING: This function uses kernel_malloc. Eventually it needs to be re-written to use the proper
	malloc
*/
void createFlatBinaryAtLocation(uint8_t* elf_file, Elf32_Shdr* headerTable, Elf32_Ehdr hdr, uint8_t* memLoc)
{
	//now copy. We are using locals because it's just easier on my brain
	uint32_t offset = 0; //where in the file the section starts
	uint32_t secSize = 0; //how many bytes to copy
	uint32_t progOffset = 0; //where in the program array this goes
	for(int i = 0; i < hdr.e_shnum; i++)
	{
		if(headerTable[i].sh_type == SHT_PROGBITS || headerTable[i].sh_type == SHT_NOBITS)
		{
			offset = headerTable[i].sh_offset;
			secSize = headerTable[i].sh_size;
			kernel_memcpy((elf_file+offset),(memLoc+progOffset),secSize);
			progOffset += secSize;
		}
	}
}