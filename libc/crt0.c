#include "crt0.h"

/*
	In a real, multi-threaded system, this is supposed to live in
	thread local variables. However in our case we don't need that.
	Everything is single tasking and everything has only a single crt0, so
	we can define errno globally here. Then, anything that includes errno.h
	will be able to access it anyway.
*/
int errno;

void _start(void* args)
{
	//The ISO-C rules are that startup sets errno to zero
	errno = 0;
	FILE** files = (FILE**)args;
	//instantiate
	stdin = files[0];
	stdout = files[1];

	//call exit(main). For now we are leaving this without arguments
	exit(main());
}

void exit(int retVal)
{
	//um...this actually does nothing right now, so we'll just return
	return;
}
