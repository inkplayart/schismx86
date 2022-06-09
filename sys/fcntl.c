#include "fcntl.h"

/*
	file open system call. Since it is a system call, it assumes that absolutely
	everything is sanitized properly
*/
void open(void *inpStruct)
{
	FOPEN_STRUCT* fop = (FOPEN_STRUCT*)inpStruct;
	fop->file = open_helper(fop->fName,fop->mode,kernel_vfs);
}

//helper for the system call
FILE* open_helper(char* filename, int mode,virtualFileSystem* vf)
{
	/*
		Search for an empty space. If none are available, return null.
	*/	
	int fd = -1;
	for(int fileID = 0; fileID < FOPEN_MAX && fd == -1; fileID++)
	{
		if((vf->allocated)[fileID] == FILE_NOT_ALLOCATED)
			fd = fileID;
	}
	//now check
	if(fd == -1)
		return 0; //NULL
		
	//OK, but now check to make sure that a file with this same name doesn't already exist
	for(int fileID = 0; fileID < FOPEN_MAX; fileID++)
	{
		char fn[NAMESIZE+1];
		kernel_memclr(fn,NAMESIZE+1);
		kernel_memcpy((vf->names+NAMESIZE*fileID),fn,NAMESIZE);
		fn[NAMESIZE+1] = 0;
		if(strcmp(fn,filename)==0)
			return 0; //no dice, file already exists
	}
	
	
	
	//otherwise we are good to go
	FILE* file = (FILE*)kernel_malloc(sizeof(FILE));
	if(file == (FILE*)0) //there is no space in memory to allocate the file
		return 0; //NULL
		
	//otherwise we are rockin'. Allocate the buffer
	file->buf = (char*)kernel_malloc(BUFSIZ);
	if((file->buf) == 0) //there is no space for the buffer
		return 0; //NULL
	
	//OK, ffs, now we know we have memory and it's all allocated. Let's rock this
	file->filePos = 0; //start the file
	file->eof = 0; //not eof
	file->mode = mode; //set as per what is sent in
	file->FD = fd; 
	file->ferr = NO_ERR;
	file->bufferSize = 0; //nothing in the buffer
	
	//now set the VFS appropriately
	vf->allocated[fd] = FILE_ALLOCATED;
	vf->files[fd] = file;
	
	//figure out the file name size
	int fNameSize = 0;
	
	//checking for <= NAMESIZE is actually silly because this is the system call, it's all sanitized
	while(fNameSize <= NAMESIZE && filename[fNameSize]!=0)
		fNameSize++;
	
	//the weirdness of the pointer addition is needed to get the right offset for the filename
	kernel_memcpy((uint8_t*)filename,(uint8_t*)(vf->names+NAMESIZE*fd),fNameSize);
	
	//we're all set up. Return it!
	return file;
	
	
}