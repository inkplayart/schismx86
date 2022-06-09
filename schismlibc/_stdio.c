#include "_stdio.h"

FILE* fopen(const char* fname, const char* mode)
{
	//create the fopen struct
	FOPEN_STRUCT fop;
	
	//populate it
	fop.fName = fname;
	if(mode[0] == 'r') //yeah yeah, I know, use strcmp
	{
		if(mode[1] != 'w')
			fop.mode = READ;
		else
			fop.mode = READ_WRITE;
	}
	else if(mode[0] == 'w')
		fop.mode = WRITE;
		
	//now we need to do a system call
	systemCall(OPEN_CALL,&fop);
	
	return fop.file;
}

//write a single character to a file stream
int fputc ( int character, FILE * stream )
{
	//check to make sure that the buffer is not full
	if(stream->filePos == (long unsigned int)stream->bufferSize-1)
	{
		//fail. Set EOF and Ferr as per the spec
		stream->eof = EOF;
		stream->ferr = FERR;
		return EOF;//TODO: there is no way to fix this at the moment!
	}
	
	//otherwise we are rockin
	stream->buf[stream->filePos] = character;
	stream->filePos += 1; //increment by 1
	return 1; //anything but EOF
}

//write a single character to stdout
int putchar(int character) //yes, it's putchar,not putc, which is a different function
{
	//try to fputc first
	if(fputc(character, kstdout)==EOF)
		return EOF; //It didn't work
		
	//otherwise, kstdout has a new character, so system call it
	systemCall(PUTC_CALL,kstdout); //this sys call just needs the output stream
	return 1; //anything but EOF
}

//get a single character from the string
int fgetc(FILE* stream)
{
	if(stream->filePos == 0) //no data - a file pos of 0 means that we are INSERTING at 0, so no data exists
		return 0; //fail
	int retVal = stream->buf[stream->filePos - 1];
	stream->filePos++; //remove the character from the stream? Well, actually just skip it, it's still there and can be seeked...
	return retVal;
}

//get a single character from stdin
int getchar()
{
	//OK, to do this properly we need to call kernel_getch as long as there is space
	if(kstdin->filePos == (long unsigned int)kstdin->bufferSize-1)
		return 0; //fail
	
	//Now we have to do a system call, whose sole purpose is to kernel_getch. This means we need to send in the file
	systemCall(GETCHAR_CALL,kstdin);
	
	//Now it's in stdin, we need to get rid of it
	int retVal = kstdin->buf[kstdin->filePos];
	if(kstdin->filePos > 0)
		kstdin->filePos--; //For stdin, we actually do reduce the filepos, unless it's zero. If it's zero there is only one thing in there
	return retVal;
}