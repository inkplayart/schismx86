#include "schismPCI.h"


//this function writes the address to the PCI port
void _PCI_writeAddr(uint32_t address)
{
	_IOPORT_writeDWord((uint16_t)PCI_ADDR,address);
}

//write a DWord to the PCI bus's DATA address
void _PCI_writeDataRaw(uint32_t data)
{
	_IOPORT_writeDWord((uint16_t)PCI_DATA,data);
}

/*
	write a DWORD data to address address.  To do this it's weird: you write the address,
	then your write the data, and the PCI bus figures itself out.  I am 45% sure that what happens here
	is that you prime the PCI bus by lighting up the proper address lines, then when you
	write the data to the data address it signals a transfer to occur...I think.
*/
void _PCI_writeDataToAddress(uint32_t address,uint32_t data)
{
	//first write the address
	_PCI_writeAddr(address);
	//then write the data
	_PCI_writeDataRaw(data); //I am 32% sure that this is all I'm supposed to do...
}

//this function reads data from the PCI port
uint32_t _PCI_readData()
{
	return _IOPORT_readDWord((uint16_t)(PCI_DATA));
}

//gets a PCI bus, device, function, offset string ready to query
//essentially, this creates an address to write to or read from
uint32_t _PCI_makeBusDevFunc(uint8_t bus, uint8_t device, uint8_t funct,uint8_t regOffset)
{
	uint32_t addr = 0;
	addr |= bus<<PCI_BUS_NUM_SHIFT;
	addr |= device << PCI_DEVICE_NUM_SHIFT;
	addr |= funct << PCI_FUNCTION_NUM_SHIFT;
	addr |= 1 << PCI_ENABLE_SHIFT;
	addr |= regOffset;
	return addr;
}

//finds the last entry in the pci bus record
pciRecord* _PCI_findLastEntry(pciRecord* pciBus)
{
	pciRecord* pciDevice = pciBus;
	//if this is an uninitialized bus, just return it
	if(pciDevice->nextRecord == (pciRecord*)UNINITIALIZED_RECORD)
		return pciBus;
	while(pciDevice->nextRecord != 0)
	{
		pciDevice = pciDevice->nextRecord;
	}
	return pciDevice;
}

/*
	Enumerates PCI stuff.  This is SUPER brute force: I go through every
	bus, every device on each bus, and every function
	
	pciBus is the master kernel record for the location of the PCI data
*/
void _PCI_enumerate(pciRecord* pciBus)
{
	kernel_printf("Enumerating PCI Devices \n");
	
	pciRecord* pciDevice = pciBus;
	uint8_t bus = 0;
	uint8_t dev = 0;
	uint8_t function = 0;
	for(bus=0; bus < 255; bus++)
	{
		for(dev=0; dev < 32; dev++)
		{
			for(function = 0; function < 8; function++)
			{
				//Set up the read by reading the vendor and device ID
				uint32_t pciAddr = _PCI_makeBusDevFunc(bus,dev,function,PCI_REGISTER_IDS);
				_PCI_writeAddr((uint32_t)pciAddr);
				uint32_t outData = _PCI_readData();
				uint32_t vendorID = outData & PCI_LOW_WORD; //only want lower bytes
				//uint32_t deviceID = (outData >> PCI_HIGH_WORD);
				if(vendorID != (PCI_NO_DEVICE))
				{
					//we found one! So now we want its data.  Make a new one and link it in
					pciDevice = _PCI_findLastEntry(pciBus);
					pciRecord* curDevice;
					if(pciDevice->nextRecord != (pciRecord*)UNINITIALIZED_RECORD)
					{
						pciDevice->nextRecord = (pciRecord*)kernel_malloc(sizeof(pciRecord));
						curDevice = pciDevice->nextRecord;
					}
					else
					{
						//this is the first device we've found and the bus is uninitialized,
						//so initialize the bus
						curDevice = pciDevice;
						pciDevice->nextRecord = 0;
					}
					//get vendor and device ID first
				//	kernel_printf("Valid device. Bus: %d Device: %d Function: %d\n",bus,dev,function);
					curDevice->bus = bus;
					curDevice->device = dev;
					curDevice->function = function;
					
					//now get the class and subclass
					uint32_t pciAddr = _PCI_makeBusDevFunc(bus,dev,function,PCI_REGISTER_CLASS);
					_PCI_writeAddr((uint32_t)pciAddr);
					outData = _PCI_readData();
					uint32_t classData = outData >> PCI_HIGH_WORD; //high byte is class code, low byte is subclass
					//load it into the record
					curDevice->deviceClass = classData>>8;
					curDevice->subclass = classData&0xFF;
					
					uint32_t extraData = outData & PCI_LOW_WORD; //high byte is Prog IF, low byte is revision ID
					curDevice->progIF = extraData>>8;
					curDevice->revNo = extraData&0xFF;
					curDevice->nextRecord = 0;
				//	kernel_printf("\t Class Code: %d Subclass: %d Prog IF: %d \n",classData>>8,classData&0xFF,extraData>>8);	
				}
			}
		}
	}
	kernel_printf("Done\n");
}

void _PCI_output(pciRecord* pciBus)
{
	pciRecord* pciDevice = pciBus;
	if(pciDevice->nextRecord == (pciRecord*)UNINITIALIZED_RECORD)
		kernel_printf("PCI bus not initialized!\n");
	else
	{
		while(pciDevice->nextRecord != 0)
		{
			kernel_printf("BDF: %u, %u, %u, Class: %u, Subclass: %u, IF: %u \n",pciDevice->bus,pciDevice->device,pciDevice->function,pciDevice->deviceClass,pciDevice->subclass,pciDevice->progIF);
			pciDevice = pciDevice->nextRecord;
		}
		//now we do the last one, since the while loop ends one early
		kernel_printf("BDF: %u, %u, %u, Class: %u, Subclass: %u, IF: %u \n",pciDevice->bus,pciDevice->device,pciDevice->function,pciDevice->deviceClass,pciDevice->subclass,pciDevice->progIF);
	}
}
