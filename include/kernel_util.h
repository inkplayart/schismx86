#ifndef KERNELUTIL
#define KERNELUTIL

#include "schismMultiBoot.h"
#include "kernel_util_defs.h"
#include "schismKernelIO.h"

//system call required files
#include "../sys/fcntl.h" //file IO

extern uint32_t* heapBottom;

//void kernel_initRamData(multiBootHeader* mbh);

//void initRamData(multiBootHeader* mbh);

void initRamData(multiBootHeader mbh,ramData* masterRam);

ramData kernelMasterRam; //this will store the master information about RAM

//void systemCall(int sysCallNo,void* ptr); //This is the function that userspace programs call

void systemCall(int sysCallNo,void* ptr); //this is the function that the sysCallHander calls

extern void isr_keyboard();
extern void isr_generic();
extern void sysCallHandler(); //This is an assembly function
#endif
