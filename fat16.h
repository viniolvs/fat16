#include <stdio.h>
#include <stdlib.h>

typedef struct BootRecord
{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	    bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;
 
	//this will be cast to it's specific type once the driver actually knows what type of FAT this is.
	unsigned char		extended_section[54];
 
}__attribute__((packed)) BootRecord;

typedef unsigned short fat16;

//
FILE* openFile(char *filename);

void readBootRecord(BootRecord *br, FILE *file);

//retorna o tamnho de  uma fat em bytes no arquivo
int fatSize(BootRecord br);

//aloca uma fat
fat16* newFat(BootRecord br);

//lê uma fat no arquivo
fat16* readFat(BootRecord br, int fat_number, FILE *file);

//retorna a posição em bytes do diretório raiz a partir do byte 0
int rootDirOffset(BootRecord br);

//retorna a posição em bytes da fat<fat_number> a partir do byte 0
int fatOffset(BootRecord br, short fat_number);

//retorna a posição em bytes da seção de dados a partir do byte 0
int dataSectionOffset(BootRecord br); 

