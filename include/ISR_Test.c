#include "ISR_Test.h"
extern int test;

//Handles the keyboard event if an input device is attached and something is interested
//in what it has to say. Will set the device to EOF and FERR if the buffer is full
void keyboard_event()
{
//	curInputDev.avail = 1;
	test = 1;
	/*
	if(curInputDev->avail != NO_DEV && curInputDev->dev != 0)
	{
		 //we have a device attached, and a keyboard event has happened, so let's get that character
		if(curInputDev->dev->filePos + 1 < curInputDev->dev->bufferSize)
		{
			 curInputDev->dev->buf[curInputDev->dev->filePos] = _PS2_readData();
			 curInputDev->dev->filePos++;
		}
		else
		{
			//the buffer is full! Something is really wrong, since we are supposed to be
			//emptying stdin more or less as fast as it comes in
			curInputDev->dev->eof = EOF;
			curInputDev->dev->ferr = FERR;
		}
		
	}
	*/
}
		