#ifndef SCHISM_IO_PORT
#define SCHISM_IO_PORT
//preposterously low level IO port stuff
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//writes dword data to port port
void _IOPORT_writeDWord(uint16_t port, uint32_t data);
//writes byte data to port port
void _IOPORT_writeByte(uint16_t port, uint8_t data);

//reads a dword from port
uint32_t _IOPORT_readDWord(uint16_t port);
//reads byte from port
uint8_t _IOPORT_readByte(uint16_t data);
#endif
