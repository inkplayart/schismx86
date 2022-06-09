/*
	Contains only those definitions strictly required by ISO C. 
	errno is...bad in general and should be rarely used
*/
#ifndef ERRNOS
#define ERRNOS

#include "yvals.h"

volatile extern int errno;

#define EDOM _EDOM
#define ERANGE _ERANGE
#define EILSEQ _EILSEQ

#endif
