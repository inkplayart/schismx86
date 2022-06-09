#ifndef SCHISM_PIC
#define SCHISM_PIC

//the Programmable Interrupt Controller is on an IO port
#include "schismIOPort.h"

//always include debug stuff
#include "schismKernelIO.h"

//definitions
#define PIC1_CMDR 0x0020 //command register IO port
#define PIC1_DATA 0x0021 //data regist IO port
#define PIC2_CMDR 0x00A0 //secondary PIC command register IO port
#define PIC2_DATA 0x00A1 //secondary PIC data register IO port

#define PIC1_STANDARD_OFFSET 0x8
#define PIC2_STANDARD_OFFSET 0x70

#define PIC1_SCHISM_OFFSET 0x40

//Do not define anything for the secondary PIC at this time

//command codes
#define PIC_EOI		0x20		/* End-of-interrupt command code */

//initialization code words 
#define ICW1_ICW4	0x01		/* ICW4 is needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */


//Function definitions
void PIC_sendEOI(unsigned char irq);
void PIC_remap(int offset1, int offset2);
void PIC_standard_setup();
void IRQ_disable(unsigned char IRQline);
void IRQ_enable(unsigned char IRQline);

#endif
