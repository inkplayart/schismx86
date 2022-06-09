#include "syscall.h"

void systemCall( int sysCallNo, void * ptr )
{
	//we need to put the input arguments into registers so they are accessible via
	//the system call. the "a" puts sysCallNo into EAX and "c" puts ptr into ECX.
	//the "memory" thing means that "This instruction may overwrite (clobber) stuff
	//in registers, so do not cache any memory stuff in the registers until we are done"
	//it's an instruction to the compiler itself to not use the registers until we are done
    asm volatile( "int $0xc0" :: "a"(sysCallNo), "c"(ptr) : "memory" );
}