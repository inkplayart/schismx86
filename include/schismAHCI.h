#ifndef SCHISMAHCI
#define SCHISMAHCI

#include "schismPCI.h"
#include "schismKernelIO.h" //needed for debug and other info messages

//PCI offsets for various registers
#define ABAR_offset 0x24 //The AHCI Base Address (where in memory AHCI registers live) is on register 0x24h on the PCI bus
#define ABAR_shift 13; //we shift the ABAR register from the PCI bus right by 13 to get ABAR
#define AHCI_PCI_CMD 0x04

//AHCI offsets for various registers
#define AHCI_GHC 0x04 //the Global Host Control register, must be set properly during
//minimal initialization
#define AHCI_PI 0x0C //the Ports Implemented register
#define AHCI_BOHC 0x28 //the Bios Handoff Register

//port specific registers
#define AHCI_PORT_REGS_OFFSET 0x100
#define AHCI_PORT_REGS_SIZE 0x80
#define AHCI_PORT_IDENT 0x24
#define AHCI_PORT_CMD 0x18 //the port command register offset
#define AHCI_PORT_SERR 0x30 //the error register
#define AHCI_PORT_SACT 0x34 //used to find free command slots
#define AHCI_PORT_CI 0x38 //the port command issue register
#define AHCI_PORT_FB 0x08
#define AHCI_PORT_IS 0x10 //the port interrupt status. Has lots of error info
#define AHCI_PORT_SSTS 0x28 //the port SATA status register, more error info
#define AHCI_PORT_TFD 0x20 //the port Task File Data register, again more error info

//port CMD bits
#define AHCI_PORT_CMD_ST 1
#define AHCI_PORT_CMD_FRE 1<<4
#define AHCI_PORT_CMD_FR 1<<14
#define AHCI_PORT_CMD_CR 1<<15

//PCI class and subclass
#define AHCI_PCI_CLASS 0x01
#define AHCI_PCI_SUBCLASS 0x06

//hardware device signatures
#define AHCI_DEVICE_HDD 0x101
#define AHCI_DEVICE_ODD 0xEB140101
#define AHCI_DEVICE_UNKNOWN 0

//command register bits
#define AHCI_BUS_MASTER_ENABLE 4
#define AHCI_MEMORY_SPACE_ENABLE 2
#define AHCI_IO_SPACE_ENABLE 1

//Now to define stuff related to the received FIS
#define AHCI_RECEIVEDFIS_SIZE 0xFF
#define AHCI_RECEIVEDFIS_ALIGNMENT 256 //the recieved FIS must be aligned on a 256 byte boundary
#define AHCI_RECEIVEDFIS_DSFIS_OFFSET 0x00 //DMA Setup FIS
#define AHCI_RECIEVEDFIS_PSFIS_OFFSET 0x20 //PIO Setup FIS
#define AHCI_RECEIVEDFIS_RFIS_OFFSET 0x40 //this is where data gets put when we read
#define AHCI_RECEIVEDFIS_SDBFIS_OFFSET 0x58 //not a clue what this does
#define AHCI_RECEIVEDFIS_UNKNOWNFIS_OFFSET 0x60 //If the device does something weird FIS
#define AHCI_CMDLIST_ALIGNMENT 1024 //the command list must be 1kB aligned


//FIS definitions
#define FIS_TYPE_REG_H2D 0x27 //Host to Device

//CMD Table definitions
#define CMD_TABLE_PRDT_START 0x80 //the PRDT starts at offset 0x80
#define CMD_TABLE_MEM_ALIGNMENT 128
#define MAX_FIS_SIZE 64 //maximum number of bytes in the FIS
#define CMD_TABLE_SCRATCH_SIZE 0x40 //this is the size of a bunch of reserved bits in the cmd table

//store HBA stuff in a struct.  This will eventually encompass all of the relevant addresses and info for the HBA
typedef struct{
	//these three variables store the BDF address of the HBA on the PCI bus
	unsigned int PCIBus;
	unsigned int PCIDevice;
	unsigned int PCIFunction;
	uint8_t* baseAddr; //the base address of the HBA's control registers
	uint32_t NCS; //number of command slots per port
	struct ahciDevice* deviceList;
}ahcihba;

//stores information about a specific device attached to the HBA
typedef struct ahciDevice{
	unsigned int port;
	unsigned int type;
	struct ahciDevice* next;
	uint8_t* cmdList; //these two pointers allow us to create the memory we need to send and receive data
	uint8_t* received_FIS; 
	uint8_t* readSectorScratch; //This is where a sector read goes. It must be initalized, typically
	//in configure
	uint8_t* writeSectorScratch; //write scratch area, again malloc it to the sector size
	uint32_t numSectors;
	uint32_t sectorSize;
	uint32_t driveSize;
}ahciDevice;

//This is the FIS used to send commands to the device (H2D is "Host to Device")
typedef struct tagFIS_REG_H2D
{
	// DWORD 0
	uint8_t  fis_type;	// FIS_TYPE_REG_H2D
 
	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:3;		// Reserved
	uint8_t  c:1;		// 1: Command, 0: Control
 
	uint8_t  command;	// Command register
	uint8_t  featurel;	// Feature register, 7:0
 
	// DWORD 1
	uint8_t  lba0;		// LBA low register, 7:0
	uint8_t  lba1;		// LBA mid register, 15:8
	uint8_t  lba2;		// LBA high register, 23:16
	uint8_t  device;		// Device register
 
	// DWORD 2
	uint8_t  lba3;		// LBA register, 31:24
	uint8_t  lba4;		// LBA register, 39:32
	uint8_t  lba5;		// LBA register, 47:40
	uint8_t  featureh;	// Feature register, 15:8
 
	// DWORD 3
	uint8_t  countl;		// Count register, 7:0
	uint8_t  counth;		// Count register, 15:8
	uint8_t  icc;		// Isochronous command completion
	uint8_t  control;	// Control register
 
	// DWORD 4
	uint8_t  rsv1[4];	// Reserved
} FIS_REG_H2D;

//Physical region Descriptors
typedef struct tagPRDT{
	uint8_t* dataBaseAddr;
	uint32_t dataBaseAddrUpper; //set to zero for 32 bit OS
	uint32_t reserved;
	uint32_t descriptionInformation;
}PRDT;

//Command Header
typedef struct cmdHead{
	uint32_t CFL: 5; //Command header length, in DWords. Must be at least 2.
	uint32_t ATAPI: 1; //whether or not this is an ATAPI command (1 for yes, 0 for no. HDD is not ATAPI, ODD is)
	uint32_t RWBit: 1; //when set to 1, this is a device write. When set to 0 it is a device read
	uint32_t Prefetch: 1; //advanced use, set to 0 for now
	uint32_t RST: 1; //a reset bit for specific reset commands. Set to 0 for now
	uint32_t BIST: 1; //used for test mode. Set to 0 always for now
	uint32_t clearBusy: 1; //if set to 1, the HBA resets some bits
	uint32_t reservedBit11: 1;
	uint32_t PMP: 4; //Port multiplier. We will set this to zero for now since we do not have a port multiplier on board
	uint32_t PRDTL : 16; //PRDT Length, as in number of PRDTs
/*	uint32_t PRDTL : 16; //PRDT Length, as in number of PRDTs
	uint32_t PMP: 4; //Port multiplier. We will set this to zero for now since we do not have a port multiplier on board
	uint32_t reservedBit11: 1;
	uint32_t clearBusy: 1; //if set to 1, the HBA resets some bits
	uint32_t BIST: 1; //used for test mode. Set to 0 always for now
	uint32_t RST: 1; //a reset bit for specific reset commands. Set to 0 for now
	uint32_t Prefetch: 1; //advanced use, set to 0 for now
	uint32_t RWBit: 1; //when set to 1, this is a device write. When set to 0 it is a device read
	uint32_t ATAPI: 1; //whether or not this is an ATAPI command (1 for yes, 0 for no. HDD is not ATAPI, ODD is)
	uint32_t CFL: 4; //Command header length, in DWords. Must be at least 2.
*/	
	uint32_t PRDTBC; //byte counter. This is updated by the HBA while reads and writes are happening, and indicates how many bytes
	//are transfered to/from the device as time progresses.
	
	uint32_t CTBA; //Command table base address. This must be 128 byte aligned.
	
	uint32_t CTBAU; //when working with 64-bit addresses, this is the upper 32 bits. It is ignored entirely in Schism.
	uint32_t reservedDWord1;
	uint32_t reservedDWord2;
	uint32_t reservedDWord3;
	uint32_t reservedDWord4; //these are reserved for use by the HBA. Don't touch them.
	
}cmdHeader;

typedef struct __attribute__((__packed__)) commandTable{
	uint8_t FIS_Bytes[MAX_FIS_SIZE];
	uint8_t SCRATCH[CMD_TABLE_SCRATCH_SIZE];
	PRDT physicalRegion; //this actually should be an array, but for now we are leaving it as a single PRDT
}cmdTable;

ahciDevice* ahciDeviceList;

//HBA initialization, exploration, and querying functions
uint32_t _AHCI_readControlReg(ahcihba hostBus);
void _AHCI_getBaseAddress(ahcihba* hostBus);
uint32_t _AHCI_readPCIRegisterAtOffset(ahcihba hostBus,uint32_t offset);
void _AHCI_BIOS_Handoff(ahcihba hostBus);
void _AHCI_configure(ahcihba* hba);
uint32_t _AHCI_detectPorts(ahcihba hostBus);//gets the number of ports IMPLEMENTED, not the number of ports USED
uint32_t _AHCI_initDeviceList(ahcihba* hostBus);
bool _AHCI_getBDF(pciRecord* pciBus,ahcihba* hostBus);
void _AHCI_activatePorts(ahcihba hostBus); //activates ports and readies them for data transfer

ahciDevice* _AHCI_getHDD(ahcihba hostbust); //gets the HDD, or more accurately, the first HDD in the device list

//Command creation and running
void _AHCI_commandTable_FillFIS(uint8_t* cmdTable, void* FIS);
void _AHCI_commandTable_FillPRDT(uint8_t* cmdTable, PRDT* physicalReg, uint32_t PRDT_Length);
uint8_t* _AHCI_commandTable_Create(uint32_t PRDT_Length);

//utility
//get the port's base address
uint8_t* _AHCI_getPortBaseAddr(uint32_t port,ahcihba hostbus);
#endif
