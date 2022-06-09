#ifndef STD_DEFS
#define STD_DEFS

#define FREE_MEMORY 1
#define ALLOCATED_MEMORY 0

//this is a data structure that contains the start location and the size of the heap
typedef struct{
	uint32_t heapSize;
	void* heapStart; //byte addressable for now
}heapData;

//this is the master heap record. It lives in the kernel
extern heapData masterHeap;

#endif
