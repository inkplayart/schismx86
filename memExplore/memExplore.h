#ifndef MEMEXPLORE
#define MEMEXPLORE

#include "../include/schismKernelIO.h"
#include "../include/stdlib.h"
//#include "../include/stdlib.h"
#include <stddef.h>
#include <stdint.h>

#define null 0

//the memory exploration library.  VERY useful when discovering hardware

//this struct stores (name,ptr) pairs so we can refer to variables.  Stored in a
//linked list, because why not?
typedef struct memVar{
	char* name;
	uint32_t ptr;
	struct memVar* next;
}memVar;

memVar* varHead;

void createNewVar(char* name,uint32_t ptr);
memVar* findByName(char* name);

uint8_t readByteString(char* name,uint32_t offset);
uint16_t readDWordString(char* name,uint32_t offset);
uint32_t readUIntString(char* name,uint32_t offset);

void memExploreLoop(); //main loop
void whos(); //lists all variables currently defined


#endif
