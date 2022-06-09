#ifndef XSFS
#define XSFS

/*
	eXtremely Simple File System
	
	This is a very basic CRUD file system based around SFS by Brendan Trotter
	and expanded upon by Benjamin David Lunt.
*/

#include <stdint.h>
#include "schismATA.h"
#include "schismAHCI.h"
#include "schismKernelIO.h" //for strcmp
#include "stdlib.h" //for memcpy

#define FILE_NAME_LEN 50 //whatever's left over in the fs_record block is for file names
#define FILE_RECORD_SIZE 64 //64 bytes per record

#define USED 1
#define DELETED 0
#define FS_META_END 2
#define FS_NOTHING 3 //used to initialize and indicate that the variable exists but is not initialized

#define FS_SUCCESS 0
#define NO_SPACE_DATA 1
#define NO_SPACE_META 2
#define NO_FILE 3

typedef struct xsfsRecord{
	uint8_t type; //either USED or DELETED, with one special block, FS_META_END, indicating end of meta area
	uint8_t osDat; //an OS-specific data byte used to store up to 256 options
	uint32_t startBlock; //LBA of the first block of the file
	uint32_t endBlock; //LBA of the last block of the file that contains file data
	uint32_t size; //size in bytes
	char name[FILE_NAME_LEN]; //null terminated
}fs_record;

typedef struct xsfshdd{
	uint32_t bytesPerSector; //required to know how to write
	uint32_t totSectors; //required to know sizes
	ahcihba* hostbus; //required to read/write sectors
}hdd;

/*
	Completely formats the hdd by writing the end block to position 0 of the last sector
*/
void clearFS(hdd* drive);

/*
	Lists all files in the meta area by name and gives information about them
*/
//This should really be part of the OS, but we need this for debugging
void listFilesByName(hdd* drive);


/*
	If possible creates a new file by searching for free space 
	
	Return values:
	FS_SUCCESS - file successfully created and allocated
	NO_SPACE_DATA - not enough contiguous space left in the data sector
	NO_SPACE_META - no space left in the meta data sector
*/
uint8_t createFile(uint32_t sizeInBytes,char fileName[],uint8_t osDat,hdd* drive); 

/*
	Returns a file's data as an array of bytes, or NULL if that
	file does not exist or malloc is unable to allocate space
*/
uint8_t* readFile(char fileName[],uint8_t osDat, hdd* drive);

/*
	Updates a file as follows: 
	
	If the new buffer is of equal or lesser size
	than the file's size on disk ((endBlock-startBlock)*512), the file is overwritten with
	the new buffer.
	
	If the new buffer is of greater size than the file's size on disk, a new file is allocated,
	if possible, and the current file is deleted.
	
	Return values are:
	
	SUCCESSS - file successfully updated
	NO_SPACE_DATA - the data sector does not have enough free, contiguous space
	NO_SPACE_META - no space left in the meta data sector
	NO_FILE - no such file exists to update
*/
uint8_t updateFile(uint32_t newBufferSize,uint8_t* buf, char fileName[],uint8_t osDat, hdd* drive);

/*
	Deletes a file by changing its type to DELETED in the meta data. Return values are:
	
	SUCCESS - file successfully deleted
	NO_FILE - no such file exists to delete
*/
uint8_t deleteFile(char fileName[],uint8_t osDat, hdd* drive);

/*
	Returns the LBA+offset of a file record, searched by name. Return values are:
	
	SUCCESS - file located
	NO_FILE - file does not exist
	
	You may wonder why we are returning LBA and offset rather than the record itself. Well, this function is used for
	both updating/finding the file AND for updating/finding the record, so we need to be able to access the record on
	disk, not just some RAM representation of it.
*/
uint8_t findFileRecord(char fileName[],uint8_t osDat, uint32_t* recordLBA, uint32_t* recordOffset,hdd* drive);



#endif
