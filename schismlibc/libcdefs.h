#ifndef LIBCDEFS
#define LIBCDEFS
	
#define EOF -1 //EOF is defined in the standard as an "integer constant expression of type int and negative value"
#define NO_ERR 0
#define FERR 1
#define BUFSIZ 512 //the size of the buffer initially created by a call to fopen or setbuf. This is the minimum buffer size in bytes
#define FOPEN_MAX 1024 //The max number of files we can open at once. This is probably a VERY small number for real OSs, but for now it's OK

//file access modes
#define READ 0 //read only. Loading programs, loading files into other buffers etc
#define WRITE 1 //write only. stdin etc
#define READ_WRITE 2 //enables access to things like stdin, which gets written to by the keyboard but then read by a process
//there will be others

typedef uint32_t fpos_t; //this will eventually be a struct

typedef struct filestruct{
	//require stuff as per POSIX standard
	int FD; //file descriptor. In POSIX these are actually nonnegative integers but use type int
	
	//required stuff as per cstdlib spec
	char* buf; //the buffer. This is "the" file itself
	fpos_t  filePos; //where we are in the file at the moment
	int eof; //the end of file indicator
	int ferr; //the error indicator
	int mode; //read, read/write etc
	int bufferSize;
	
	//TODO: implement binary vs. text mode
}FILE;

typedef struct fos{
	FILE* file;
	int mode;
	char* fName;
}FOPEN_STRUCT;

#endif
