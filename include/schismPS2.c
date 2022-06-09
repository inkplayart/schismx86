//PS/2 functionality

#include "schismPS2.h"

void _PS2_writeCommand(uint8_t cmd)
{
	_IOPORT_writeByte((uint16_t) PS2_CMD_STAT, cmd);
}

void _PS2_writeByteToDevice(uint8_t cmd)
{
	_IOPORT_writeByte((uint16_t) PS2_DATA, cmd);
}

uint8_t _PS2_readByteFromDevice()
{
	//wait for the byte to have data to send
	while((_PS2_readStatus()&1) == 0){}
	return _PS2_readData();
}

uint8_t _PS2_readStatus() //reads the status byte
{
	return _IOPORT_readByte((uint16_t)(PS2_CMD_STAT));
}

bool _PS2_dataReady()
{
	return _PS2_readStatus()&1; //returns 1 if the LSB of status reg is set, 
	//indicating that data is ready to read
}

uint8_t _PS2_readData() //reads the data byte
{
	return _IOPORT_readByte((uint16_t)(PS2_DATA));
}

bool _PS2_waitForAck()
{
	while(_PS2_readByteFromDevice()!=PS2_ACK){}
	return true;
}

bool _PS2_selfTest()
{
	//now test the PS2 port
	uint8_t outData;
	_PS2_writeCommand(PS2_CONTROLLER_TEST);
	
	//wait until the read is ready
	outData = _PS2_readStatus();
	while((outData & 1) == 0){
	kernel_printf("Checking for data\n");
	outData = _PS2_readStatus();
	}
	
	outData = _PS2_readData();
	if(outData == PS2_CONTROLLER_OK)
		return true;
	return false;
}

uint32_t _PS2_CheckDevice()
{
	//I have no idea if I need to do this, but for fun:
	
	//disable 2nd port
	_PS2_writeCommand(PS2_DISABLE_PORT_2);
	
	//enable 1st port
	_PS2_writeCommand(PS2_ENABLE_PORT_1);
	
	//disable scanning so that keyboard input won't screw this up
	_PS2_writeByteToDevice(PS2_DISABLE_SCANNING);
	
	//wait for ACK
	kernel_printf("Waiting for ACK\n");
	 _PS2_waitForAck();
	
	//write identify
	_PS2_writeByteToDevice(PS2_IDENTIFY);
	
	kernel_printf("Waiting for ACK\n");
	 _PS2_waitForAck();
	 
	kernel_printf("Waiting for no ACK\n");
	while(!_PS2_dataReady()){}
	
	uint32_t outData = 0;
	uint8_t lowByte = _PS2_readByteFromDevice();
	kernel_printf("Received %d \n",(int)lowByte);
	outData |= lowByte;
	if(lowByte == PS2_DEVICE_MF2_HIGHBYTE)
	{
		//read all data
		while(!_PS2_dataReady()){}
		outData |= _PS2_readByteFromDevice() << 8;
	}
	
	//re-enable scanning so that the keyboard can talk to us
	_PS2_writeByteToDevice(PS2_ENABLE_SCANNING);
	kernel_printf("Waiting for ACK\n");
	_PS2_waitForAck();
	
	return outData;
}
