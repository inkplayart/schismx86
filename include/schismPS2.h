//PS/2 functionality
#ifndef SCHISM_PS2
#define SCHISM_PS2

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "schismIOPort.h"
#include "schismKernelIO.h"

//port addresses
#define PS2_DATA 0x60 //The Data Port (IO Port 0x60) is used for reading data that was received from 
//a PS/2 device or from the PS/2 controller itself and writing data to a PS/2 device or to the PS/2 controller itself.

#define PS2_CMD_STAT 0x64 //this is the COMMAND and STATUS address.  It commands the PS2
//controller itself, not the PS2 devices living there

//commands for controller
#define PS2_CONTROLLER_TEST 0xAA
#define PS2_DISABLE_PORT_2 0xA7
#define PS2_ENABLE_PORT_1 0xAE

//commands for devices
#define PS2_DISABLE_SCANNING 0xF5
#define PS2_ENABLE_SCANNING 0xF4
#define PS2_IDENTIFY 0xF2

//responses
#define PS2_CONTROLLER_OK 0x55
#define PS2_ACK 0xFA
#define PS2_DEVICE_MF2_HIGHBYTE 0xAB
#define PS2_DEVICE_MF2_LOWBYTE 0x83

//read/write functions
void _PS2_writeCommand(uint8_t cmd);
uint8_t _PS2_readStatus(); //reads the status byte
uint8_t _PS2_readByteFromDevice(); //reads a byte from the device
void _PS2_writeByteToDevice(uint8_t cmd); //writes a byte to do the device, used during setup
bool _PS2_dataReady(); //returns true when there is data ready to be sent
uint8_t _PS2_readData(); //reads the data byte
bool _PS2_waitForAck(); //waits for ACK byte

//control fuctions.  If port1 is true it will test port 1, otherwise it will test port 2
bool _PS2_selfTest();

//checks the device on port 1
uint32_t _PS2_CheckDevice();

#endif
