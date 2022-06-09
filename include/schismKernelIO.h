//kernel IO functionality.  Eventually this will be superseded by a proper stdio.h
#ifndef KERNEL_IO_DEF
#define KERNEL_IO_DEF

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

//keyboard support
#include "schismPS2.h"

//print character info
#define ZERO_CHAR 48
#define ESCAPE_BYTE 1
#define BACKSPACE_BYTE 0x0E
#define TAB_BYTE 0x0F
#define ENTER_BYTE 0x1C
#define LEFT_CONTROL_BYTE 0x1D
#define LEFT_SHIFT_BYTE 0x2A
#define LEFT_SHIFT_RELEASED 0xAA
#define RIGHT_SHIFT_BYTE 0x36
#define RIGHT_SHIFT_RELEASED 0xB6
#define LEFT_ALT_BYTE 0x38


#define INT_OUTPUT_SIZE 11
#define TAB_WIDTH 5

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

//this should go into schism util or something later
size_t strlen(const char* str);

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_writeint(int data);
void terminal_writeuint32(uint32_t data);
void terminal_handle_newline();
void kernel_printf(const char* data,...);
char kernel_getch();
void getline(char* line);
int strcmp(char* str1, char* str2); //returns -1 if str1 < str2, 0 if equal, and 1 if str1 > str2

//Some very useful helper functions
/*
	Prints size bytes at location ptr in binary
*/
void printBytesBinary(unsigned int size,void* ptr);
void printDWordAsInt(unsigned int numWords, unsigned int* ptr);
void printBytesBinaryLines(unsigned int size, void* ptr);
#endif
