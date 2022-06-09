//kernel IO behaviour
#include "schismKernelIO.h"

 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
bool isShift;
 
void terminal_initialize(void) 
{
	isShift = false;
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_putchar(char c) 
{
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}
 
void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
	{
		if(data[i] != '\n')
			terminal_putchar(data[i]);
		else
			terminal_handle_newline();
	}
}
 
void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}

void terminal_writeint(int data)
{
	//we need an array of characters.  Its an integer, which in a 32 bit OS is
	//max 10 characters.  +1 for the zero character and we're good to go
	char outString[INT_OUTPUT_SIZE] = {0}; //the way we are doing this gives LSD first, so we need to reverse it later
	char outStringReversed[INT_OUTPUT_SIZE] = {0}; //this is the actual output string
	int inputSize = 0;
	if(data < 0)
	{
		terminal_putchar('-');
		data = -data;
	}
	if(data == 0)
	{
		terminal_putchar(ZERO_CHAR);
		return;
	}
	
	while(data!= 0)
	{
		outString[inputSize] = data%10 + ZERO_CHAR;
		inputSize++;
		data /= 10;
	}
	
	//now reverse the array
	int maxSize = inputSize-1; //inputSize has been ++'d one too many times
	for(int i = inputSize-1; i >= 0; i--)
	{
		outStringReversed[maxSize-i] = outString[i];
	}
	terminal_writestring(outStringReversed);
	return;
}

//almost identical to writeInt except what is being sent in and no check for negative
void terminal_writeuint32(uint32_t data)
{
	//we need an array of characters.  Its an integer, which in a 32 bit OS is
	//max 10 characters.  +1 for the zero character and we're good to go
	char outString[INT_OUTPUT_SIZE] = {0}; //the way we are doing this gives LSD first, so we need to reverse it later
	char outStringReversed[INT_OUTPUT_SIZE] = {0}; //this is the actual output string
	int inputSize = 0;
	if(data == 0)
	{
		terminal_putchar(ZERO_CHAR);
		return;
	}
	
	while(data!= 0)
	{
		outString[inputSize] = data%10 + ZERO_CHAR;
		inputSize++;
		data /= 10;
	}
	
	//now reverse the array
	int maxSize = inputSize-1; //inputSize has been ++'d one too many times
	for(int i = inputSize-1; i >= 0; i--)
	{
		outStringReversed[maxSize-i] = outString[i];
	}
	terminal_writestring(outStringReversed);
	return;
}

//a VERY basic printf for debug and basic talk with the komputermachin
void kernel_printf(const char* data,...)
{
	//create the variable argument list
	va_list vargin;
	
	//start it
	va_start(vargin,data);
	
	int i = 0;
	while(data[i])
	{
		if(data[i] != '%' && data[i] != '\n' && data[i] != '\t')
			terminal_putchar(data[i]);
		else
		{
			//handle the backslash
			if(data[i] == '\n')
			{
				terminal_handle_newline();
			}
			else if(data[i] == '\t')
			{
				for(int i = 0; i < TAB_WIDTH; i++)
					terminal_putchar(' ');
			}
			else //this is a format string
			{
				//this is a SUPER simple printf = it's only integers for now
				//first, increment i
				i++;
				//now check it = later we'll handle different things than int
				if(data[i] == 'd')
				{
					int nextOut = va_arg(vargin,int);
					terminal_writeint(nextOut);
				}
				else if(data[i] == 'u')
				{
					uint32_t nextOut = va_arg(vargin,uint32_t);
					terminal_writeuint32(nextOut);
				}
			}
		}
		i++;
	}
	
	//release the arguments
	va_end(vargin);
	return;

}

//In general it just increments the row counter, but if we are at the maximum height
//this will move the entire buffer up by 1 and blank out the last line
void terminal_handle_newline()
{
	terminal_column = 0;
	//reset the terminal after advancing the row
	if(++terminal_row == VGA_HEIGHT)
	{
		terminal_row--;
		//now copy everything
		for (size_t y = 1; y < VGA_HEIGHT; y++) {
			for (size_t x = 0; x < VGA_WIDTH; x++) {
				size_t index = y * VGA_WIDTH + x;
				size_t prevRowIndx = (y-1)*VGA_WIDTH + x;
				terminal_buffer[prevRowIndx] = terminal_buffer[index];
			}
		}
		//blank out the last line
		for(size_t i = 0; i < VGA_WIDTH; i++)
		{
			size_t index = (VGA_HEIGHT-1)*VGA_WIDTH + i;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_handle_backspace()
{
	if(terminal_column != 0)
		terminal_column--;
	else
	{
		if(terminal_row != 0)
		{
			terminal_column = VGA_WIDTH-1;
			terminal_row--;
		}
	}
	//blank it out
	terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
}

//OK, this is actually a GIANT switch statement that looks at the keys and returns them
//note: the scan code set is Scan Code Set 1, but I do not know if this will change
char kernel_getch()
{
	//this blocks until a byte is read
	uint8_t lowByte = _PS2_readByteFromDevice();
	//isShift = true;
	 //Handling shift is a LOT more difficult than it seems, we are going to ignore it 
	//for now
	if((lowByte == LEFT_SHIFT_BYTE || lowByte == RIGHT_SHIFT_BYTE) && !isShift)
	{
		isShift = true;
		return 0;
	}
	else if((lowByte == LEFT_SHIFT_BYTE || lowByte == RIGHT_SHIFT_BYTE) && isShift)
	{
		//this means that we are holding down shift
		while(lowByte == LEFT_SHIFT_BYTE || lowByte == RIGHT_SHIFT_BYTE)
			lowByte = _PS2_readByteFromDevice();
	}
	//now wait for another key
	
	//now check what it was.  If it is a release code, then we just return 0 - nothing happens
	if(lowByte == LEFT_SHIFT_RELEASED || lowByte == RIGHT_SHIFT_RELEASED)
	{
		isShift = false;
		return 0;
	}
	
	//OK, it's not a shift release, so it's something else. Handle it
	char retVal = 0;
	//yes, now a big, giant lookup table.  I'm using switch statements
	switch(lowByte)
	{
		case 0x00:
			retVal = 0;
			break;
		case 0x01:
			retVal = ESCAPE_BYTE;
			break;
		case 0x02:
			retVal = isShift ? '!' : '1';
			break;
		case 0x03:
			retVal = isShift ? '@' : '2';
			break;
		case 0x04:
			retVal = isShift ? '#' : '3';
			break;
		case 0x05:
			retVal = isShift ? '$' : '4';
			break;
		case 0x06:
			retVal = isShift ? '%' : '5';
			break;
		case 0x07:
			retVal = isShift ? '^' : '6';
			break;
		case 0x08:
			retVal = isShift ? '&' : '7';
			break;
		case 0x09:
			retVal = isShift ? '*' : '8';
			break;
		case 0x0A:
			retVal = isShift ? '(' : '9';
			break;
		case 0x0B:
			retVal = isShift ? ')' : '0';
			break;
		case 0x0C:
			retVal = isShift ? '_' : '-';
			break;
		case 0x0D:
			retVal = isShift ? '+' : '=';
			break;
		case 0x0E:
			retVal = BACKSPACE_BYTE;
			break;
		case 0x0F:
			retVal = '\t';
			break;
		case 0x10:
			retVal = isShift ? 'Q' : 'q';
			break;
		case 0x11:
			retVal = isShift ? 'W' : 'w';
			break;
		case 0x12:
			retVal = isShift ? 'E' : 'e';
			break;
		case 0x13:
			retVal = isShift? 'R' : 'r';
			break;
		case 0x14:
			retVal = isShift ? 'T' : 't';
			break;
		case 0x15:
			retVal = isShift ? 'Y' : 'y';
			break;
		case 0x16:
			retVal = isShift ? 'U' : 'u';
			break;
		case 0x17:
			retVal = isShift ? 'I' : 'i';
			break;
		case 0x18:
			retVal = isShift ? 'O' : 'o';
			break;
		case 0x19:
			retVal = isShift ? 'P' : 'p';
			break;
		case 0x1A:
			retVal = isShift ? '{' : '[';
			break;
		case 0x1B:
			retVal = isShift ? '}' : ']';
			break;
		case 0x1C:
			retVal = '\n';
			break;
		case 0x1D:
			retVal = LEFT_CONTROL_BYTE;
			break;
		case 0x1E:
			retVal = isShift ? 'A' : 'a' ;
			break;
		case 0x1F:
			retVal = isShift ? 'S' : 's';
			break;
		case 0x20:
			retVal = isShift ? 'D' : 'd';
			break;
		case 0x21:
			retVal = isShift ? 'F' : 'f';
			break;
		case 0x22:
			retVal = isShift ? 'G' : 'g';
			break;
		case 0x23:
			retVal = isShift ? 'H' : 'h';
			break;
		case 0x24:
			retVal = isShift ? 'J' : 'j';
			break;
		case 0x25:
			retVal = isShift ? 'K' : 'k';
			break;
		case 0x26:
			retVal = isShift ? 'L' : 'l';
			break;
		case 0x27:
			retVal = isShift ? ':' : ';';
			break;
		case 0x28:
			retVal = isShift ? '\"' : '\'';
			break;
		case 0x29:
			retVal = isShift ? '~' : '`';
			break;
		case 0x2A:
			retVal = LEFT_SHIFT_BYTE;
			break;
		case 0x2B:
			retVal = isShift ? '|' : '\\';
			break;
		case 0x2C:
			retVal = isShift ? 'Z' : 'z';
			break;
		case 0x2D:
			retVal = isShift ? 'X' : 'x';
			break;
		case 0x2E:
			retVal = isShift ? 'C' : 'c';
			break;
		case 0x2F:
			retVal = isShift ? 'V' : 'v';
			break;
		case 0x30:
			retVal = isShift ? 'B' : 'b';
			break;
		case 0x31:
			retVal = isShift ? 'N' : 'n';
			break;
		case 0x32:
			retVal = isShift ? 'M' : 'm';
			break;
		case 0x33:
			retVal = isShift ? '<' : ',';
			break;
		case 0x34:
			retVal = isShift ? '>' : '.';
			break;
		case 0x35:
			retVal = isShift ? '?' : '/';
			break;
		case 0x36:
			retVal = RIGHT_SHIFT_BYTE;
			break;
		case 0x37:
			retVal = 0x37; //this is a keypad byte, I'm not supporting this atm
			break;
		case 0x38:
			retVal = LEFT_ALT_BYTE;
			break;
		case 0x39:
			retVal = ' ';
			break;
		default:
			retVal = 0x00;
	}
	return retVal;
}

//we assume that line is big enough
void getline(char* line)
{
	char c = kernel_getch();
	int count = 0;
	while(c != '\n')
	{
		//not SUPER great to do this here, but it's easier to buffer
		if(c!= 0x00)
		{
			if(c!='\n' && c!= BACKSPACE_BYTE)
				terminal_putchar(c);
			else if (c == '\n')
				terminal_handle_newline();
			else
				terminal_handle_backspace();		
		}
		if(c == BACKSPACE_BYTE && count > 0)
			count--;
		else if (c != 0x00) //the default byte
		{
			line[count] = c;
			count++;
		}
		c = kernel_getch();
	}
	terminal_handle_newline();
	line[count] = 0;
}

int strcmp(char* str1, char* str2)
{
	int count = 0;
	while(str1[count] != 0 && str2[count] != 0)
	{
		if(str1[count] > str2[count])
			return 1;
		if(str1[count] < str2[count])
			return -1;
		count++;
	}
	if(str1[count] == 0 && str2[count] == 0)
		return 0;
	if(str1[count] == 0)
		return -1; //there is more in str2
	return 1; //there is more in str1
}

/*
	Prints size bytes at location ptr in binary
*/
void printBytesBinary(unsigned int size,void* ptr)
{
	unsigned char byte;
	unsigned char* b = (unsigned char*)ptr;
	for(int i = size-1; i>=0; i--)
	{
		for(int j = 7; j>=0; j--)
		{
			byte = (b[i]>>j)&1;
			if(byte == 0)
				kernel_printf("0");
			else
				kernel_printf("1");
		}
		kernel_printf(" ");
	}
}

//WARNING: THIS PRINTS EVERYTHING BACKWARDS. DO NOT USE IT UNTIL YOU FIX IT
void printBytesBinaryLines(unsigned int size, void* ptr)
{
	//prints bytes in binary in multiple lines of 4 bytes (one DWORD)
	unsigned char byte;
	unsigned char* b = (unsigned char*)ptr;
	int nBytesPrinted = 0;
	kernel_printf("\n");
	for(int i = size-1; i>=0; i--)
	{
		if(nBytesPrinted%4 == 0)
			kernel_printf("\n");
		for(int j = 7; j>=0; j--)
		{
			byte = (b[i]>>j)&1;
			if(byte == 0)
				kernel_printf("0");
			else
				kernel_printf("1");
		}
		kernel_printf(" ");
		nBytesPrinted++;
	}
	kernel_printf("\n");
}

/*
	Prints the numWords dwords starting at ptr
*/
void printDWordAsInt(unsigned int numWords, unsigned int* ptr)
{
	for(unsigned int i = 0; i < numWords; i++)
	{
		kernel_printf("%u\n",ptr[i]);
	}
}