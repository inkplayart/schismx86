#ifndef VFS
#define VFS

#include "_stdio.h"
#include "libcdefs.h"
#include "stdlib.h"

#define NAMESIZE 12 //8.3 filenames
#define FILE_ALLOCATED 1
#define FILE_NOT_ALLOCATED 0


//Now, there is actually only ever going to be one of these and it lives in the kernel
typedef struct vfile{
int allocated[FOPEN_MAX]; //an array indicating whether a file is allocated (nonzero) or not (zero).
char names[NAMESIZE*FOPEN_MAX]; //the names in 8.3 format
FILE* files[FOPEN_MAX]; //the actual files themselves
}virtualFileSystem;

//opens a file if possible, allocates its memory and its initial buffer,
//sets it up in the VFS and assigns it a unique ID. Returns NULL if this is not possible
void open(void* inpStruct);
FILE* open_helper(char* filename,int mode,virtualFileSystem* vf);

//closes a file, cleans up its memory in the VFS. This does NOT clean up the file itself,
//that is fclose's job, which is a process-level thing
void close(FILE* file);

extern virtualFileSystem* kernel_vfs;


#endif
