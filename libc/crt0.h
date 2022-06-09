#ifndef CRT0
#define CRT0

#include "stdio.h"

//these live in the process itself, they are not actually part of stdio
//but stdio can access them
FILE* stdout;
FILE* stdin;

void _start(void* args);
void exit(int retVal);
int main();

#endif
