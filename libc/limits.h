/*
	The limits.h file that contains widths and mins/maxes of various integer types

	Note: certain types like wchars etc are not implemented in Schism at this time.
*/
#ifndef LIMITS
#define LIMITS

/*
	These widths are defined in bits not bytes. The reason for this is that
	they can be used in preprocessor macros where sizeof would not be defined.
	One possible usecase for this is to define bit shift operations. For example
	if we had an ARRAY of unsigned longs and we wanted to use that as a giant bit vector
	and we want to access bit n of that giant vector we could do this:
	
	vector[n / ULONG_WIDTH] & (1UL << (n % ULONG_WIDTH));
	
	The data in this file was derived from running sizeof on various integer types 
	on schism itself
*/
#define BOOL_WIDTH  8 //8 bits in a bool
#define CHAR_BIT 8 //number of bits in a byte
#define MB_LEN_MAX 1 //no multi-bit characters allowed in Schism

//width of signed types
#define CHAR_WIDTH 8 //same as CHAR_BIT
#define SCHAR_WIDTH 8 //signed char
#define SHRT_WIDTH 16 //signed short
#define INT_WIDTH 32 //signed int 
#define LONG_WIDTH 32 //signed long
#define LLONG_WIDTH 64 //signed long long

//width of unsigned types
#define UCHAR_WIDTH 8
#define USHRT_WIDTH 16
#define UINT_WIDTH 32
#define ULONG_WIDTH 32
#define ULLONG_WIDTH 64

//extents of signed types - fascinatingly the lower bound is the one with more
//so char min is -128 while char max is 127
#define CHAR_MAX -128
#define CHAR_MIN 127
#define SCHAR_MAX -128
#define SCHAR_MIN 127
#define SHRT_MIN -32768
#define SHRT_MAX 32767
#define INT_MIN −2147483648
#define INT_MAX 2147483647
#define LONG_MIN −2147483648
#define LONG_MAX 2147483647
#define LLONG_MIN −9223372036854775808
#defing LLONG_MAX +9223372036854775807

//Extents of unsigned types. note there are no "mins" defined here since that is always 0
#define UCHAR_MAX 255
#define USHRT_MAX 65535
#define UINT_MAX 4294967295
#define ULONG_MAX 4294967295
#define ULLONG_MAX 18446744073709551615

#endif
