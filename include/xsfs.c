#include "xsfs.h"

/*
	If possible creates a new file by searching for free space 
	
	Return values:
	SUCCESS - file successfully created and allocated
	NO_SPACE_DATA - not enough contiguous space left in the data sector
	NO_SPACE_META - no space left in the meta data sector
*/
uint8_t createFile(uint32_t sizeInBytes,char fileName[],uint8_t osDat,hdd* drive)
{
	kernel_printf("Creating a file\n");
	//start the search at the last sector
	uint32_t searchSector = drive->totSectors-1; //-1 because sectors start at 0
	uint8_t* meta;
	fs_record record;
	record.type = FS_NOTHING; //starting with nothing
	int recordNum = 0; //once we find the record, we need to know where it is in case we need to 
	//move sectors
	uint32_t lastBlock = 0;
	while(record.type != FS_META_END)
	{
		//search each sector of the file records
		meta = _ATA_readSector(drive->hostbus,searchSector);
		recordNum = 0;
		for(int i = 0; i < drive->bytesPerSector/FILE_RECORD_SIZE; i++)
		{
			kernel_printf("Now in the for loop");
			record = ((fs_record*)meta)[i];
			kernel_printf("Record type is: %d\n",record.type);
			recordNum++;
			if(record.type == DELETED && record.size >= sizeInBytes)
			{
				//we found it! We are swanky to go - no need to worry about meta size
				((fs_record*)meta)[i].type = USED;
				((fs_record*)meta)[i].osDat = osDat;
				//start block remains unchanged, but end block may change
				uint32_t numFullBlocksNeeded = sizeInBytes/(drive->bytesPerSector);
				if(sizeInBytes > numFullBlocksNeeded*drive->bytesPerSector)
					numFullBlocksNeeded++; //This means we need an extra block
				((fs_record*)meta)[i].endBlock = ((fs_record*)meta)[i].startBlock + numFullBlocksNeeded - 1; //start block is a valid block, so we subtract 1
				((fs_record*)meta)[i].size = sizeInBytes;
				uint32_t nameSize = strlen(fileName) + 1; //include the zero character
				kernel_memcpy(fileName,((fs_record*)meta)[i].name,nameSize);
				//dump the sector back
				_ATA_writeSector(drive->hostbus,searchSector,meta,drive->bytesPerSector);
				return FS_SUCCESS;
			}				
			else if(record.type == FS_META_END)
			{
				kernel_printf("Found the meta end");
				//we are at the last record
				i = drive->bytesPerSector; //Just escape from the for loop, this number could be anything
			}
			else
			{
				//this means it is either a file that exists or is deleted but too small
				//either way we need to know the ending block
				lastBlock = record.endBlock;
			}
		}
		searchSector--; //decrement
	}
	
	//The only way we are here is if we found the end of the records
	searchSector++; //undo the last decrement
	recordNum--; //undo the last increment
	//now we have two cases: either we have space left in this sector of meta or we do not
	//If we do not, then we need to create a new sector
	//TODO: This does not bother to check if there is enough space to do this. IDGAF at this point.
	
	//We are good. We are going to overwite the end tag and re-write it next
	((fs_record*)meta)[recordNum].type = USED;
	((fs_record*)meta)[recordNum].osDat = osDat;
	((fs_record*)meta)[recordNum].startBlock = lastBlock+1; //it is one after the last allocated block
	//start block remains unchanged, but end block may change
	uint32_t numFullBlocksNeeded = sizeInBytes/drive->bytesPerSector;
	if(sizeInBytes > numFullBlocksNeeded*drive->bytesPerSector)
		numFullBlocksNeeded++; //This means we need an extra block
	((fs_record*)meta)[recordNum].endBlock = ((fs_record*)meta)[recordNum].startBlock + numFullBlocksNeeded - 1; //start block is a valid block, so we subtract 1
	((fs_record*)meta)[recordNum].size = sizeInBytes;
	uint32_t nameSize = strlen(fileName) + 1; //include the zero character
	kernel_memcpy(fileName,((fs_record*)meta)[recordNum].name,nameSize);
	
	if(recordNum < drive->bytesPerSector/FILE_RECORD_SIZE)
	{
		kernel_printf("Good to go!");
		((fs_record*)meta)[recordNum+1].type = FS_META_END; //no other stuff is important
		
		//dump the sector back
		_ATA_writeSector(drive->hostbus,searchSector,meta,drive->bytesPerSector);
	}
	else
	{
		//Need a new sector. Ugh.
		
		//write the current sector back to store the new file
		_ATA_writeSector(drive->hostbus,searchSector,meta,drive->bytesPerSector);
		
		searchSector--;
		//update meta
		meta = _ATA_readSector(drive->hostbus,searchSector);
		
		//clear it
		kernel_memclr(meta,drive->bytesPerSector);
		//write the new record
		record = ((fs_record*)meta)[0]; //it has to be the first entry
		((fs_record*)meta)[0].type = FS_META_END;
		
		//write the new sector
		_ATA_writeSector(drive->hostbus,searchSector,meta,drive->bytesPerSector);
	}
	return FS_SUCCESS;
}	

//Returns NULL if the file doesn't exist, otherwise returns a valid pointer
uint8_t* readFile(char fileName[],uint8_t osDat, hdd* drive)
{
	//first, get the file's record
	//First, find the file
	uint32_t LBA;
	uint32_t offset;
	uint8_t ret = findFileRecord(fileName,osDat,&LBA,&offset,drive);
	if(ret == NO_FILE)
		return 0;
	
	//get the record
	//we found it!
	uint8_t* recs = _ATA_readSector(drive->hostbus,LBA);
	fs_record record = ((fs_record*)recs)[offset];
	
	//Now check its size
	uint32_t numBlocksCurrent = record.endBlock - record.startBlock + 1; //+1 because startblock is valid
	
	//how much is left to write
	uint32_t newBufferSize = numBlocksCurrent*drive->bytesPerSector;
	
	//allocate memory for it - currently using kernel_malloc!
	uint8_t* retval = (uint8_t*)kernel_malloc(numBlocksCurrent*drive->bytesPerSector);
	uint8_t* buf = retval; //what we are reading into
	//read it into retval
	if(!retval)
		return 0; //no space
	
	//OK, at this point we have a file to read from. We need to do this sector by sector.
	uint32_t bufferOffset = 0;
	for(uint32_t sect = record.startBlock; sect <= record.endBlock; sect++)
	{
		
		buf = buf + bufferOffset;
		//read the sector
		uint8_t* readSect = _ATA_readSector(drive->hostbus,sect);
		//two cases: either we are writing a full sector or we are not
		if(newBufferSize <= drive->bytesPerSector)
		{
			kernel_memcpy(readSect,buf,newBufferSize);
		}
		else
		{
			//we are writing a whole sector
			kernel_memcpy(readSect,buf,drive->bytesPerSector);
			newBufferSize -= drive->bytesPerSector; //reduce it
		}
		offset += drive->bytesPerSector;
	}
	return retval;
}

//HUGE TODO: This doesn't look for space at all, it just assumes we can write it
uint8_t updateFile(uint32_t newBufferSize,uint8_t* buf, char fileName[],uint8_t osDat, hdd* drive)
{
	//First, find the file
	uint32_t LBA;
	uint32_t offset;
	uint8_t ret = findFileRecord(fileName,osDat,&LBA,&offset,drive);
	if(ret == NO_FILE)
		return NO_FILE;
	
	//get the record
	//we found it!
	uint8_t* recs = _ATA_readSector(drive->hostbus,LBA);
	fs_record record = ((fs_record*)recs)[offset];
	kernel_printf("Updating file: ");
	terminal_writestring(record.name); 	
	
	//Now check its size against the new buffer. If it is within the size limits we are good to go
	uint32_t numBlocksCurrent = record.endBlock - record.startBlock + 1; //+1 because startblock is valid
	
	uint32_t numFullBlocksNeeded = newBufferSize/(drive->bytesPerSector);
	if(newBufferSize > numFullBlocksNeeded*drive->bytesPerSector)
		numFullBlocksNeeded++; //This means we need an extra block
	
	if(numFullBlocksNeeded > numBlocksCurrent) //we need a new file
	{
		//delete it and recreate it. The other functions will handle that part
		deleteFile(fileName,osDat,drive);
		createFile(newBufferSize,fileName,osDat,drive);
		findFileRecord(fileName,osDat,&LBA,&offset,drive);
		recs = _ATA_readSector(drive->hostbus,LBA);
		record = ((fs_record*)recs)[offset];
	}
	else
	{
		kernel_printf("No new blocks. Setting the size to %d\n",newBufferSize);
		//just need to update the size
		((fs_record*)recs)[offset].size = newBufferSize;
		((fs_record*)recs)[offset].startBlock = record.startBlock;
		((fs_record*)recs)[offset].endBlock = record.startBlock + numFullBlocksNeeded - 1; //remember, start block is valid
		_ATA_writeSector(drive->hostbus,LBA,recs,drive->bytesPerSector);
	}
	
	//OK, at this point we have a file to write to. We need to do this sector by sector.
	uint32_t bufferOffset = 0;
	for(uint32_t sect = record.startBlock; sect <= record.endBlock; sect++)
	{
		
		buf = buf + bufferOffset;
		//two cases: either we are writing a full sector or we are not
		if(newBufferSize <= drive->bytesPerSector)
		{
			_ATA_writeSector(drive->hostbus,sect,buf,newBufferSize);
		}
		else
		{
			//we are writing a whole sector
			_ATA_writeSector(drive->hostbus,sect,buf,drive->bytesPerSector);
			newBufferSize -= drive->bytesPerSector; //reduce it
		}
		offset += drive->bytesPerSector;
	}
	return FS_SUCCESS;
}

uint8_t findFileRecord(char fileName[],uint8_t osDat, uint32_t* recordLBA, uint32_t* recordOffset, hdd* drive)
{
	//Basically, we search the meta area and modify the two pointer variables, or return an error code
	//start the search at the last sector
	uint32_t searchSector = drive->totSectors-1; //-1 because sectors start at 0
	uint8_t* meta;
	fs_record record;
	record.type = FS_NOTHING; //starting with nothing
	
	//read the first sector and check that this drive has anything in it
	meta = _ATA_readSector(drive->hostbus,searchSector);
	record = ((fs_record*)meta)[0];
	

	while(record.type != FS_META_END)
	{
		//search each sector of the file records
		meta = _ATA_readSector(drive->hostbus,searchSector);
		for(int i = 0; i < drive->bytesPerSector/FILE_RECORD_SIZE; i++)
		{
			record = ((fs_record*)meta)[i];
			if(record.type == FS_META_END)
				return NO_FILE;
			if(strcmp(record.name,fileName) == 0 && osDat == record.osDat)
			{
				*recordLBA = searchSector;
				*recordOffset = i;
				return FS_SUCCESS;
			}
		}
		searchSector--;
	}
	
	//The only way we are here is if we find the FS_META_END
	return NO_FILE;
}

uint8_t deleteFile(char fileName[],uint8_t osDat, hdd* drive)
{
	//we first see if the file exists. If so, we read its record and set it to deleted
	uint32_t LBA;
	uint32_t offset;
	
	uint8_t ret = findFileRecord(fileName,osDat,&LBA,&offset,drive);
	
	if(ret == NO_FILE)
		return NO_FILE;
	
	//we found it!
	uint8_t* recs = _ATA_readSector(drive->hostbus,LBA);
	((fs_record*)recs)[offset].type = DELETED;
	_ATA_writeSector(drive->hostbus,LBA,recs,drive->bytesPerSector);
	return FS_SUCCESS;
}

void clearFS(hdd* drive)
{
	//This part is sort of simple: write the last sector's 0th position to contain the FS_META_END block
	//start the search at the last sector
	uint32_t searchSector = drive->totSectors-1; //-1 because sectors start at 0
	uint8_t* meta = _ATA_readSector(drive->hostbus,searchSector);
	((fs_record*)meta)[0].type = FS_META_END; //starting with nothing
	char nm[] = "Meta End Entry";
	kernel_memcpy(nm,((fs_record*)meta)[0].name,strlen(nm));
	kernel_printf(", Meta name is: ");
	terminal_writestring(((fs_record*)meta)[0].name);
	kernel_printf("Go team is: %d\n",((fs_record*)meta)[0].type);
	_ATA_writeSector(drive->hostbus,searchSector,meta,drive->bytesPerSector);
}

//This should really be part of the OS, but we need this for debugging
void listFilesByName(hdd* drive)
{
	//start the search at the last sector
	uint32_t searchSector = drive->totSectors-1; //-1 because sectors start at 0
	uint8_t* meta;
	fs_record record;
	record.type = FS_NOTHING; //starting with nothing
	
	meta = _ATA_readSector(drive->hostbus,searchSector);
	record = ((fs_record*)meta)[0];

	while(record.type != FS_META_END)
	{
		//search each sector of the file records
		meta = _ATA_readSector(drive->hostbus,searchSector);
		for(int i = 0; i < drive->bytesPerSector/FILE_RECORD_SIZE; i++)
		{
			record = ((fs_record*)meta)[i];
			if(record.type == FS_META_END)
				return;
			terminal_writestring(record.name);
			kernel_printf(" Type: %d, Size: %d, Start: %d, End: %d, osData: %d\n",record.type,record.size,record.startBlock,record.endBlock,record.osDat);
		}
		searchSector--;
	}
}