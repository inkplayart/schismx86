#ifndef EXTRACT_ELF
#define EXTRACT_ELF

#include <stdint.h> //This should exist in Schism
#include <stdbool.h>

#include <stdlib.h> //REMOVE THIS BEFORE TESTING ON SCHISM, IT WILL NOT YET COMPILE

#define EI_NIDENT 16 //Size in bytes of the identify array

//file type defines
#define ET_NONE 0 //no file type
#define ET_REL 1 //relocatable file
#define ET_EXEC 2 //executable file
#define ET_DYN 3 //shared object, dynamically linkable
#define ET_CORE 4 //core file
#define ET_LOPROC 0xff00 //Processor specific
#define ET_HIPROC 0xffff //Processor specific

//machine identities - we'll only define the most relevant ones
#define EM_NON 0 //no machine
#define EM_386 //i386

//identity indices
#define EI_MAG0 0 //magic number index 0
#define EI_MAG1 1 
#define EI_MAG2 2
#define EI_MAG3 3 //index 3 for magic
#define EI_CLASS 4 //file class
#define EI_DATA 5 //data encoding
#define EI_VERSION 6 //file version
#define EI_PAD 7 //start of padding bytes

//the actual magic bytes so we can ensure this is a valid ELF file on load
#define ELFMAG0 0x7f
#define ELFMAG1 0x45 //'E'
#define ELFMAG2 0x4C //'L'
#define ELFMAG3 0x46 //'F'

//the ELF file classes
#define ELFCLASSNONE 0 //invalid class
#define ELFCLASS32 1 //32 bit objects
#define ELFCLASS64 2 //64 bit objects

//data encoding
#define ELFDATANONE 0 //invalid encoding
#define ELFDATA2LSB 1 //Means LSB occupies the lower address
#define ELFDATA2MSB 2 //Means MSB occcupies the lower address

//section header types
#define SHT_NULL 0 //Yes, SHT really is what the section header table's defines start at in the ELF documentation
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2 //symbol table
#define SHT_STRTAB 3 //string table
#define SHT_RELA 4 //reloc table
#define SHT_HASH 5 //hash?
#define SHT_DYNAMIC 6 //dynamic relocation table
#define SHT_NOTE 7 //notes/comments
#define SHT_NOBITS 8 //data, typically BSS
#define SHT_REL 9 //Uh...
#define SHT_SHLIB 10
#define SHT_DYNSYM 11 //dynamic symbol table
#define SHT_LOPROC 0x70000000 //processor specific 
#define SHT_HIPROC 0x7fffffff //processor specific
#define SHT_LOUSER 0x80000000 //generic address for userspace in unix
#define SHT_HIUSER 0xffffffff //high address for userspace

//The ELF header
typedef struct elf_head_s{
	unsigned char e_ident[EI_NIDENT];
	uint16_t e_type; //type of file, such as EXEC etc
	uint16_t e_machine; //
	uint32_t e_version;
	uint32_t e_entry; //entry address
	uint32_t e_phoff; //program header offset
	uint32_t e_shoff; //section header offset
	uint32_t e_flags; //for x86 this is 0 since this processor defines no flags
	uint16_t e_ehsize; //size of this specific header...kind of meta
	uint16_t e_phentsize; //Pheasants. Also the size of one entry of the program header table. All entries are the same size
	uint16_t e_phnum; //Number of entries in the program header table
	uint16_t e_shentsize; //Size of one entry of the section header table. All entries are the same size
	uint16_t e_shnum; //number of entries in the program header table
	uint16_t e_shstrndx; //The section header table index of the entry associated with the section name string table
}Elf32_Ehdr;

//The section headers
typedef struct shead{
	uint32_t sh_name; //name of the section. SUPER irrelevant for the loader
	uint32_t sh_type; //type. VERY relevant. We are looking for certain parts that indicate the executable/data portions
	uint32_t sh_flags; //perhaps not relevant
	uint32_t sh_addr; //not relevant for our loader, loads always happen at the same (virtual)address
	uint32_t sh_offset; //the offset *in this ELF file*. VERY relevant
	uint32_t sh_size; //the size, very relevant for allocating space to the data
	uint32_t sh_link; //not relevant for now
	uint32_t sh_info; //not relevant for now
	uint32_t sh_addralign; //MAYBE not relevant for now, I don't know
	uint32_t sh_entsize; //not relevant for loading the program, but relevant for knowing how big various other tables are
}Elf32_Shdr;

void readHeader(uint8_t* elf_file, Elf32_Ehdr* hdr);
bool isValidELF(Elf32_Ehdr hdr); //determines if the magic constant is there
Elf32_Shdr* getSectionHeaders(uint8_t* elf_file,Elf32_Ehdr hdr); //returns the address of the section headers
uint32_t getFlatBinarySize(Elf32_Shdr* headerTable,Elf32_Ehdr hdr); //Gets the size for malloc
uint8_t* createFlatBinary(uint8_t* elf_file,Elf32_Shdr* headerTable,Elf32_Ehdr hdr); //returns the actual loadable binary
void createFlatBinaryAtLocation(uint8_t* elf_file, Elf32_Shdr* headerTable, Elf32_Ehdr hdr, uint8_t* memLoc); //creates the actual binary at the required location
#endif
