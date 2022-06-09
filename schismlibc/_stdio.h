#ifndef STDIO
#define STDIO

#include <stdint.h>
#include "kernel_util.h" //for system calls
#include "libcdefs.h"

//This file contains only the function definitions. For structs and consts see libcdefs.h

/*

	BIG NOTE: This is the kernel-only version. For processes this file needs to be modified
	and this note removed. The process version needs to have the "extern" removed from the
	stdio streams defined here.

	OK, so we are going to be implementing the functions and data types I need to get various
	things working. We will be doing this in as POSIX-compliant a way that I can make,
	but we aren't going to make the OS fully POSIX-compliant. What this means
	is that all functions that are written and data types will be POSIX-compliant,
	but we will not write all of the functions/data structures
	requred for POSIX-compliance.

*/

//opens a file
FILE* fopen(const char* fname, const char* mode);

//puts a single character into the buffer, if possible
int fputc ( int character, FILE * stream );
int putchar(int character); //put a single character into stdout and print, if possible
int fgetc(FILE* stream); //get a single character from the stream

extern FILE* kstdout;
extern FILE* kstdin;

#endif
