#ifndef ISR_TEST
#define ISR_TEST
#include "schismKernelIO.h"
#include "schism_PIC.h"
#include "_stdio.h"

#define NO_DEV 0
#define DEV_ATTACHED 1

struct interrupt_frame {
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
} __attribute__((packed));

typedef struct kbrd{
	FILE* dev;
	int avail; //the user level IO library modifies this via a system call
}keyboard;
 
void keyboard_event();
#endif
