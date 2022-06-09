#include "memExplore.h"

void createNewVar(char* name,uint32_t ptr){
	//first one
	if(varHead->name == null)
	{
		varHead->name = name;
		varHead->ptr = ptr;
		varHead->next = null;
		//keep the next pointer to null
	}
	else
	{
		memVar* tmp = varHead;
		
		while(tmp->next != null)
			tmp = tmp->next;
		
		tmp->next = (memVar*)kernel_malloc(sizeof(memVar));
		tmp = tmp->next; //now it's not null...prolly
		tmp->name = name;
		tmp->ptr = ptr;
		tmp->next = null;
	}
}

memVar* findByName(char* name)
{
	memVar* tmp = varHead;
	
	while(strcmp(tmp->name,name) != 0 && tmp != null)
	{
		tmp = tmp->next;
	}
	return tmp; //this will return null if name is not found, so we have to handle that
}

uint8_t readByteString(char* name,uint32_t offset)
{
	memVar* tmp = findByName(name);
	
	if(tmp != null)
	{
		uint8_t* pt = (uint8_t*)tmp->ptr;
		return *(pt + offset);
	}
	else
	{
		kernel_printf("Name isn't found...you should probably ignore this data\n");
		return 0;
	}
}

uint16_t readDWordString(char* name,uint32_t offset)
{
	memVar* tmp = findByName(name);
	if(tmp != null)
	{
		uint16_t* pt = (uint16_t*)tmp->ptr;
		return *(pt + offset);
	}
	else
	{
		kernel_printf("Name isn't found...you should probably ignore this data\n");
		return 0;
	}
}


uint32_t readUIntString(char* name,uint32_t offset)
{
	memVar* tmp = findByName(name);
	if(tmp != null)
	{
		uint32_t* pt = (uint32_t*)tmp->ptr;
		return *(pt + offset);
	}
	else
	{
		kernel_printf("Name isn't found...you should probably ignore this data\n");
		return 0;
	}
}

//now, we can't use this when creating a new variable, since we need access to index and count later on
////but for everything else we can
char* readNameString(char* cmd)
{
	kernel_printf("Reading name string: \n");
	//let.  Read the string.
	int indx = 4; //where the string must start.  ABSOLUTELY ZERO error checking
	int count = 0;
	while(cmd[indx + count] != ' ' && cmd[indx+count] != 0)
		count++; //count the length of the string
	char* newName = kernel_malloc(count + 1); //make a new string, one bigger so we can null-terminate
	
	count = indx; //4 is where this starts
	
	//read the new name
	while(cmd[count] != ' ' && cmd[count] != 0)
	{
		newName[count-4] = cmd[count];
		count++;
	}		
		
	newName[count-4] = 0; //zero terminate this string or else WEIRD things happen
	
	return newName;		
}

//reads the offset string for r*s
uint32_t readOffsetString(char* cmd)
{
	//ok, we need to read the command and the name out of the way:
	int indx = 4; //where the string must start.  ABSOLUTELY ZERO error checking
	int count = 0;
	while(cmd[indx + count] != ' ' && cmd[indx+count] != 0)
		count++; //count the length of the string
	count += indx;
	//now we found the name, read again
	if(cmd[count] != 0)
	{
		//create the offset
		uint32_t offset = 0;
		
		//determine how big it is
		count++; //now we are at the beginning of the number
		
		//find the end
		while(cmd[count] != 0)
		{
			offset = offset*10 + (cmd[count] - ZERO_CHAR);
			count++;
		}
		//now we're done
		return offset;
	}
	else
		return 0; //no offset is requested
}

void memExploreLoop()
{
	kernel_printf("Welcome to mem explore! Type esc to escape, or help for a list of commands\n");
	kernel_printf("Varhead: %u\n",varHead);
	varHead = kernel_malloc(sizeof(memVar));
	varHead->next = null;
	varHead->name = null;
	
	//yeah, I know there are better ways to do this.
	char cmd[100];
	for(int i = 0; i < 100; i++)
		cmd[i] = 0;
	
	//the main loop
	while(cmd[0]!='e' && cmd[1] != 's' && cmd[2] != 'c')
	{	kernel_printf(">> ");
		for(int i = 0; i < 100; i++)
			cmd[i] = 0;
		getline(cmd);
		if(cmd[0] == 'l' && cmd[1] == 'e' && cmd[2] == 't')
		{
			//let.  Read the string.
			int indx = 4; //where the string must start.  ABSOLUTELY ZERO error checking
			int count = 0;
			while(cmd[indx + count] != ' ')
				count++; //count the length of the string
			char* newName = kernel_malloc(count + 1); //make a new string, one bigger so we can null-terminate
			
			count = indx; //4 is where this starts
			
			//read the new name
			while(cmd[count] != ' ')
			{
				newName[count-4] = cmd[count];
				count++;
			}		
				
			newName[count-4] = 0; //zero terminate this string or else WEIRD things happen
			
			//create the pointer
			uint32_t pt = 0;
			
			//determine how big it is
			count++; //now we are at the beginning of the number
			
			//find the end
			while(cmd[count] != 0)
			{
				pt = pt*10 + (cmd[count] - ZERO_CHAR);
				count++;
			}
			
			//ok, we are ready.  Let's create the new variable
			createNewVar(newName,pt);
		}
		else if(cmd[0] == 'w' && cmd[1] == 'h' && cmd[2] == 'o')
		{
			whos();
		}
		else if(cmd[0] == 'r' && cmd[1] == 'b' && cmd[2] == 's')
		{
			char* name = readNameString(cmd);
			uint32_t offset = readOffsetString(cmd);
			uint8_t outByte = readByteString(name,offset);
			uint32_t outInt = outByte;
			memVar* tmp = findByName(name);
			kernel_printf("Value at %u is %u\n",tmp->ptr + offset,outInt);
		//	free(name); //free up memory
		}
		else if(cmd[0] == 'r' && cmd[1] == 'd' && cmd[2] == 's')
		{
			char* name = readNameString(cmd);
			uint32_t offset = readOffsetString(cmd);
			uint16_t outBytes = readDWordString(name,offset);
			uint32_t outInt = outBytes;
			memVar* tmp = findByName(name);
			kernel_printf("Value at %u is %u\n",tmp->ptr + offset*sizeof(uint16_t),outInt);
		//	free(name);
		}
		else if(cmd[0] == 'r' && cmd[1] == 'u' && cmd[2] == 's')
		{
			char* name = readNameString(cmd);
			uint32_t offset = readOffsetString(cmd);
			uint32_t outInt = readUIntString(name,offset);
			memVar* tmp = findByName(name);
			kernel_printf("Value at %u is %u\n",tmp->ptr + offset*sizeof(uint32_t),outInt);
		//	free(name);
		}
		else
			kernel_printf("Invalid command\n");
	}
	
	kernel_printf("Memexplore ended\n");
}

void whos() //lists all variables currently defined
{
	memVar* tmp = varHead;
	while(tmp != null)
	{
		terminal_writestring(tmp->name); //yeah, kernelIO should be able to do this...
		kernel_printf(", %u\n",tmp->ptr);
		tmp = tmp->next;
	}
}
