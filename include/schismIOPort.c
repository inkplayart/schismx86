//preposterously low level IO port stuff
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "schismIOPort.h"

//writes dword data to port port
void _IOPORT_writeDWord(uint16_t port, uint32_t data)
{
	//send the data out using the "out" instruction, which writes to IO registers.
	
	/*
		OK, so what's going on here is: out is the mnemonic for the "output to port" instruction
		%0 and %1 are the input arguments to the command, which are specified in the colons
		"a" means "store this in AX register"
		"dN" means "allow this to be a stream of bytes"
	*/
	__asm__ volatile ("out %1, %0":/*no outputs*/:"dN"((port)), "a" (data));
}


//writes byte data to port port
void _IOPORT_writeByte(uint16_t port, uint8_t data)
{
	//send the data out using the "out" instruction, which writes to IO registers.
	__asm__ volatile ("outb %1, %0":/*no outputs*/:"dN"((port)), "a" (data));
}

//reads a dword from port
uint32_t _IOPORT_readDWord(uint16_t port)
{
	uint32_t inData;
	
	//Note that %1 is the address we are sending to.  "Nd" means "send this as bytes,no worries".
	__asm__ volatile("inl %1, %0":"=a"(inData):"Nd"((port)));
	
	return inData;
}

//reads a byte from port
uint8_t _IOPORT_readByte(uint16_t port)
{
	uint8_t inData;
	
	//Note that %1 is the address we are sending to.  "Nd" means "send this as bytes,no worries".
	__asm__ volatile("inb %1, %0":"=a"(inData):"Nd"((port))); 
	
	return inData;
}
