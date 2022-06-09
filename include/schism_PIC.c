#include "schism_PIC.h"

//this tells the PIC to chill and that the interrupt is over
// irq = the interrupt number that was just handled
void PIC_sendEOI(unsigned char irq)
{
	if(irq >= 8)
		_IOPORT_writeByte((uint16_t)PIC2_CMDR,(uint8_t)PIC_EOI);
 
	_IOPORT_writeByte((uint16_t)PIC1_CMDR,(uint8_t)PIC_EOI);
}

/* reinitialize the PIC controllers, giving them specified vector offsets (if needed)
   rather than 8h and 70h, as configured by default 
   
   When you enter protected mode (or even before hand, if you're not using GRUB) the first command you will need to give the two PICs is the initialise command (code 0x11). 
   This command makes the PIC wait for 3 extra "initialisation words" on the data port. These bytes give the PIC:

	Its vector offset. (ICW2)
	Tell it how it is wired to master/slaves. (ICW3)
	Gives additional information about the environment. (ICW4)
	
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
	
	Note: all of this code came directly from osdev wiki and was only modified
	to use my own IOPort library.  
*/
void PIC_remap(int offset1, int offset2)
{
	unsigned char a1, a2;
 
	a1 = _IOPORT_readByte((uint16_t)PIC1_DATA);                        // save masks
	a2 = _IOPORT_readByte((uint16_t)PIC2_DATA);
 
	_IOPORT_writeByte((uint16_t)PIC1_CMDR, (uint8_t)ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	_IOPORT_writeByte((uint16_t)PIC2_CMDR, (uint8_t)ICW1_INIT | ICW1_ICW4);
	_IOPORT_writeByte((uint16_t)PIC1_DATA, (uint8_t)offset1);                 // ICW2: Master PIC vector offset
	_IOPORT_writeByte((uint16_t)PIC2_DATA, (uint8_t)offset2);                 // ICW2: Slave PIC vector offset
	_IOPORT_writeByte((uint16_t)PIC1_DATA, (uint8_t)4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	_IOPORT_writeByte((uint16_t)PIC2_DATA, (uint8_t)2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	_IOPORT_writeByte((uint16_t)PIC1_DATA, (uint8_t)(ICW4_8086) | (ICW4_AUTO));				  // ICW4: Environment stuff
	_IOPORT_writeByte((uint16_t)PIC2_DATA, (uint8_t)(ICW4_8086) | (ICW4_AUTO));
 
	_IOPORT_writeByte((uint16_t)PIC1_DATA, (uint8_t)a1);   // restore saved masks.
	_IOPORT_writeByte((uint16_t)PIC2_DATA, (uint8_t)a2);
}

//sets up the PIC using the standard offsets
void PIC_standard_setup()
{
	//PIC_remap(PIC1_STANDARD_OFFSET,PIC2_STANDARD_OFFSET);
	PIC_remap(0x40,0x80);
	
	//disable everything
	for(unsigned char i = 0; i < 16; i++)
		IRQ_disable(i);
}

//sets an IRQ to be masked.  A masked interrupt cannot fire
void IRQ_disable(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = _IOPORT_readByte(port) | (1 << IRQline);
    _IOPORT_writeByte(port, value);        
}
 
 //this clears the mask to enable the interrupt
void IRQ_enable(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = _IOPORT_readByte(port) & ~(1 << IRQline);
    _IOPORT_writeByte(port, value);        
}
