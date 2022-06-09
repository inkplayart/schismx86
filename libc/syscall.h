#ifndef syscall
#define syscall

#define SYS_CALL_OFFSET 0x80 //the system call offset in the IDT

//system call numbers
#define OPEN_CALL 0 //I'm sure these can be changed later, or ignored.
#define PUTC_CALL 1 //why not
#define GETCHAR_CALL 2

void systemCall( int sysCallNo, void * ptr );

#endif
