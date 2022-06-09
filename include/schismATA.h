#ifndef ATA
#define ATA

#include "schismAHCI.h"
#include "schismKernelIO.h"

#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_READ_DMA 0xC8

//a function to send a command! Here's hoping!
uint8_t* _ATA_sendID(ahcihba* HBA); //probably needs HBA
bool _ATA_writeSector(ahcihba* HBA,uint32_t sector,uint8_t* buf,uint32_t numBytes);
uint8_t* _ATA_readSector(ahcihba* HBA,uint32_t sector);
void _ATA_initHDD(ahcihba* HBA); //initializes the sizes of the HDD
#endif
