//PCI Bus stuff
#ifndef SCHISM_PCI_INCLUDE
#define SCHISM_PCI_INCLUDE

#include "schismKernelIO.h"
#include "schismIOPort.h"
#include "kernel_util_defs.h"
#include "schismPCI_defs.h"
#include "stdlib.h"

//this function writes to the PCI port's PCI_ADDR
void _PCI_writeAddr(uint32_t address);

//write a DWord to the PCI bus's DATA address
void _PCI_writeDataRaw(uint32_t data);

//write a DWORD data to address address
void _PCI_writeDataToAddress(uint32_t address,uint32_t data);

//this function reads data from the PCI port
uint32_t _PCI_readData();

//gets a PCI bus, device, function string ready to query
uint32_t _PCI_makeBusDevFunc(uint8_t bus, uint8_t device, uint8_t funct,uint8_t regOffset);

//Enumerates PCI stuff for the first few addresses
void _PCI_enumerate(pciRecord* pciBus);
#endif
