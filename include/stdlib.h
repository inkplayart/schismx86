#ifndef SCHISM_STDLIB
#define SCHISM_STDLIB

//standard includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

//need the multiboot stuff so that we can search the memory map for real RAM
#include "schismMultiBoot.h"
#include "schismKernelIO.h"
#include "kernel_util_defs.h"
#include "stdlib_defs.h"

extern uint32_t* heapBottom;


//this will search for where the heapBottom is, and then figure out
//how big the heap is allowed to be
void initKernelMalloc(heapData* heap,ramData masterRam,uint32_t heapMaxSize);

void* kernel_malloc(uint32_t size);
void* kernel_malloc_align(uint32_t size, uint32_t alignment); //an extension to malloc that aligns on boundaries
void free(void* ptr);

//copies numBytes from src to dest
void kernel_memcpy(uint8_t* src, uint8_t* dest, uint32_t numBytes);
//sets bytes to zero
void kernel_memclr(uint8_t* loc, uint32_t numBytes);

#endif
