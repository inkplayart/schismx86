#include "schismATA.h"

//This is going to be one almighty function until we get the ID command working
uint8_t* _ATA_sendID(ahcihba* HBA){
	//OK, we need to get various addresses out, so first we need to find the HDD.
	//By this point we are VERY sure that we have a HDD
	
	ahciDevice* curDev = HBA->deviceList;
	while(curDev != 0 && curDev->type != AHCI_DEVICE_HDD)
	{
		curDev = curDev->next; //find the last one
	}
	//curDev now holds all of the references to the HDD
	uint32_t prt = curDev->port;
	
	//now we need to actually access the two structures stored there
	uint32_t* cmdListAddr = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA));
	//uint32_t* PXFB = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA)+AHCI_PORT_FB);
	//uint8_t* receivedFISAddr = (uint8_t*)(*PXFB);
	
	//create the command FIS. For Identify device this is an FIS_REG_H2D
	FIS_REG_H2D fis;
	kernel_memclr((uint8_t*)&fis, sizeof(FIS_REG_H2D));
	fis.fis_type = FIS_TYPE_REG_H2D;
	fis.command = ATA_CMD_IDENTIFY;	// 0xEC
	fis.device = 0;			// Master device
	fis.c = 1;				// Write command register

	//now set it up in the command table
	cmdTable cmdT;
	kernel_memclr((uint8_t*)&cmdT,sizeof(cmdTable));

	//copy the FIS over
	kernel_memcpy((uint8_t*)&fis,(uint8_t*)&cmdT,sizeof(FIS_REG_H2D));
	
	//now create the PRDT in the command table
	cmdT.physicalRegion.dataBaseAddr = curDev->readSectorScratch;//(uint8_t*)kernel_malloc_align(0x800,4);
	kernel_memclr((uint8_t*)cmdT.physicalRegion.dataBaseAddr,0x800);
	cmdT.physicalRegion.descriptionInformation = 0;
	cmdT.physicalRegion.descriptionInformation &= 1<<31; //enable interrupt on completion
	cmdT.physicalRegion.descriptionInformation += 0x800; //set the byte count to the size of the PRDT
	
	//Now set up the command header
	cmdHeader *cmdH = (cmdHeader*)(*cmdListAddr);
	kernel_memclr((uint8_t*)cmdH,sizeof(cmdHeader));
	cmdH->clearBusy = 1;
	cmdH->CFL = sizeof(FIS_REG_H2D)/4; //It's the length of the command header in DWords, not bytes
	cmdH->CTBA = (uint32_t)&cmdT;
	cmdH->RWBit = 1; //it's a write to the device
	cmdH->PRDTL = 1;
	
	//At this point we are ready to issue the command.
	
	uint32_t* portCmdEnable = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_CI);
	
	*portCmdEnable = 1; //this sets command 1 and tells the HBA to fetch it
	
	//wait for it to complete
	while(*portCmdEnable){}
	
	return (uint8_t*)cmdT.physicalRegion.dataBaseAddr;
}

//identifies the harddrive and initializes its sizes
void _ATA_initHDD(ahcihba* HBA){
	
	//OK, we need to get various addresses out, so first we need to find the HDD.
	//By this point we are VERY sure that we have a HDD
	
	ahciDevice* curDev = HBA->deviceList;
	while(curDev != 0 && curDev->type != AHCI_DEVICE_HDD)
	{
		curDev = curDev->next; //find the last one
	}
	//curDev now holds all of the references to the HDD
	uint32_t prt = curDev->port;
	
	//now we need to actually access the two structures stored there
	uint32_t* cmdListAddr = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA));
	//uint32_t* PXFB = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA)+AHCI_PORT_FB);
	//uint8_t* receivedFISAddr = (uint8_t*)(*PXFB);
	
	//create the command FIS. For Identify device this is an FIS_REG_H2D
	FIS_REG_H2D fis;
	kernel_memclr((uint8_t*)&fis, sizeof(FIS_REG_H2D));
	fis.fis_type = FIS_TYPE_REG_H2D;
	fis.command = ATA_CMD_IDENTIFY;	// 0xEC
	fis.device = 0;			// Master device
	fis.c = 1;				// Write command register

	//now set it up in the command table
	cmdTable cmdT;
	kernel_memclr((uint8_t*)&cmdT,sizeof(cmdTable));

	//copy the FIS over
	kernel_memcpy((uint8_t*)&fis,(uint8_t*)&cmdT,sizeof(FIS_REG_H2D));
	
	//now create the PRDT in the command table
	cmdT.physicalRegion.dataBaseAddr = curDev->readSectorScratch;//(uint8_t*)kernel_malloc_align(0x800,4);
	kernel_memclr((uint8_t*)cmdT.physicalRegion.dataBaseAddr,0x800);
	cmdT.physicalRegion.descriptionInformation = 0;
	cmdT.physicalRegion.descriptionInformation &= 1<<31; //enable interrupt on completion
	cmdT.physicalRegion.descriptionInformation += 0x800; //set the byte count to the size of the PRDT
	
	//Now set up the command header
	cmdHeader *cmdH = (cmdHeader*)(*cmdListAddr);
	kernel_memclr((uint8_t*)cmdH,sizeof(cmdHeader));
	cmdH->clearBusy = 1;
	cmdH->CFL = sizeof(FIS_REG_H2D)/4; //It's the length of the command header in DWords, not bytes
	cmdH->CTBA = (uint32_t)&cmdT;
	cmdH->RWBit = 1; //it's a write to the device
	cmdH->PRDTL = 1;
	
	//At this point we are ready to issue the command.
	
	uint32_t* portCmdEnable = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_CI);
	
	*portCmdEnable = 1; //this sets command 1 and tells the HBA to fetch it
	
	//wait for it to complete
	while(*portCmdEnable){}
	
	//Now we're good. We need to set up the HDD.
	uint8_t* identData = curDev->readSectorScratch;
	char dat[4] = {identData[120],identData[121],identData[122],identData[123]};
	char sz[4] = {identData[234],identData[235],identData[236],identData[237]};
	
	curDev->numSectors = *((uint32_t*)dat);
	if(*((uint32_t*)sz)==0)
		curDev->sectorSize = 512; //default value fo 512 bytes per sector
	curDev->driveSize = curDev->numSectors * curDev->sectorSize;
}


/*
	Writes buf to sector. This function will return false if numBytes is more than 
	the size of a sector
*/
bool _ATA_writeSector(ahcihba* HBA,uint32_t sector,uint8_t* buf,uint32_t numBytes)
{
	//TO DO: Figure out the actual size of a sector. I'm sure it's stored somewhere
	if(numBytes > 512)
		return false;
	//locate the HDD in the device list
	ahciDevice* curDev = HBA->deviceList;
	while(curDev != 0 && curDev->type != AHCI_DEVICE_HDD)
	{
		curDev = curDev->next; //find the last one
	}
	
	//curDev now holds all of the references to the HDD
	uint32_t prt = curDev->port;
	
	//now we need to actually access the two structures stored there
	uint32_t* cmdListAddr = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA));
	
	//create the command FIS. For Identify device this is an FIS_REG_H2D
	FIS_REG_H2D fis;
	kernel_memclr((uint8_t*)&fis, sizeof(FIS_REG_H2D));
	fis.fis_type = FIS_TYPE_REG_H2D;
	fis.command = ATA_CMD_WRITE_DMA;	// 0xCA
	fis.device = 0;			// Master device
	fis.c = 1;				// Write command register
	fis.lba0 = sector; //This is the sector we are writing to
	fis.countl = 1;

	//now set it up in the command table
	cmdTable cmdT;
	kernel_memclr((uint8_t*)&cmdT,sizeof(cmdTable));

	//copy the FIS over
	kernel_memcpy((uint8_t*)&fis,(uint8_t*)&cmdT,sizeof(FIS_REG_H2D));
	
	//now create the PRDT in the command table
	cmdT.physicalRegion.dataBaseAddr = curDev->writeSectorScratch;//(uint8_t*)kernel_malloc_align(0x800,4);
	kernel_memclr((uint8_t*)cmdT.physicalRegion.dataBaseAddr,0x800);
	
	//copy from buf to data base
	kernel_memcpy(buf,cmdT.physicalRegion.dataBaseAddr,numBytes);
	
	cmdT.physicalRegion.descriptionInformation = 0;
	cmdT.physicalRegion.descriptionInformation &= 1<<31; //enable interrupt on completion
	cmdT.physicalRegion.descriptionInformation += 0x800; //set the byte count to the size of the PRDT
	
	//Now set up the command header
	cmdHeader *cmdH = (cmdHeader*)(*cmdListAddr);
	kernel_memclr((uint8_t*)cmdH,sizeof(cmdHeader));
	cmdH->clearBusy = 1;
	cmdH->CFL = sizeof(FIS_REG_H2D)/4; //It's the length of the command header in DWords, not bytes
	cmdH->CTBA = (uint32_t)&cmdT;
	cmdH->RWBit = 1; //it's a write to the device
	cmdH->PRDTL = 1;
	
	//At this point we are ready to issue the command.
	
	uint32_t* portCmdEnable = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_CI);
	
	*portCmdEnable = 1; //this sets command 1 and tells the HBA to fetch it
	
	//wait for it to complete
	while(*portCmdEnable){}
	return true;
}

uint8_t* _ATA_readSector(ahcihba* HBA,uint32_t sector){
	//locate the harddrive in the device list
	ahciDevice* curDev = HBA->deviceList;
	while(curDev != 0 && curDev->type != AHCI_DEVICE_HDD)
	{
		curDev = curDev->next; //find the last one
	}
	//curDev now holds all of the references to the HDD
	uint32_t prt = curDev->port;
	
	//now we need to actually access the two structures stored there
	uint32_t* cmdListAddr = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA));

	//create the command FIS. For Identify device this is an FIS_REG_H2D
	FIS_REG_H2D fis;
	kernel_memclr((uint8_t*)&fis, sizeof(FIS_REG_H2D));
	fis.fis_type = FIS_TYPE_REG_H2D;
	fis.command = ATA_CMD_READ_DMA;	// 0xC8
	fis.device = 0;			// Master device
	fis.c = 1;				// Write command register
	fis.lba0 = sector; //the sector we are reading
	fis.countl = 1;

	//now set it up in the command table
	cmdTable cmdT;
	kernel_memclr((uint8_t*)&cmdT,sizeof(cmdTable));

	//copy the FIS over
	kernel_memcpy((uint8_t*)&fis,(uint8_t*)&cmdT,sizeof(FIS_REG_H2D));
	
	//now create the PRDT in the command table
	cmdT.physicalRegion.dataBaseAddr = curDev->readSectorScratch;//(uint8_t*)kernel_malloc_align(0x800,4);
	kernel_memclr((uint8_t*)cmdT.physicalRegion.dataBaseAddr,0x800);
	
	cmdT.physicalRegion.descriptionInformation = 0;
	cmdT.physicalRegion.descriptionInformation &= 1<<31; //enable interrupt on completion
	cmdT.physicalRegion.descriptionInformation += 0x800; //set the byte count to the size of the PRDT
	
	//Now set up the command header
	cmdHeader *cmdH = (cmdHeader*)(*cmdListAddr);
	kernel_memclr((uint8_t*)cmdH,sizeof(cmdHeader));
	cmdH->clearBusy = 1;
	cmdH->CFL = sizeof(FIS_REG_H2D)/4; //It's the length of the command header in DWords, not bytes
	cmdH->CTBA = (uint32_t)&cmdT;
	cmdH->RWBit = 0; //This one is a ready
	cmdH->PRDTL = 1;
	
	//At this point we are ready to issue the command.
	
	uint32_t* portCmdEnable = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_CI);
	
	*portCmdEnable = 1; //this sets command 1 and tells the HBA to fetch it
	
	//wait for it to complete
	while(*portCmdEnable){}
	
	return ((uint8_t*)cmdT.physicalRegion.dataBaseAddr); 
}