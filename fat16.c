#include "fat16.h"
#include <stdlib.h>
#include <stdio.h>

//
void openFile(FILE *file, char *filename)
{
    file = fopen(filename, "rb");
}

void readBootRecord(BootRecord *br, FILE *file)
{
    fseek(file, 0, SEEK_SET);
    fread(br, sizeof(BootRecord),1, file);
}

int fatSize(BootRecord br)
{
    return br.table_size_16*br.bytes_per_sector;
}

fat16* newFat(BootRecord br)
{
    fat16 *fat;
    fat = (fat16*)malloc(br.table_size_16 * br.bytes_per_sector);
    return fat;
}

void readFat(BootRecord br, fat16 *fat, int fat_number, FILE *file)
{
    fseek(file, fatOffset(br,fat_number), SEEK_SET); 
    fread(fat, fatSize(br), 1,  file);
}

//retorna a posição em bytes do diretório raiz a partir do byte 0
int rootDirOffset(BootRecord br)
{
    return ((br.table_size_16) * (br.table_count) + (br.reserved_sector_count)) * (br.bytes_per_sector);
}

//retorna a posição em bytes da fat<fat_number> a partir do byte 0
int fatOffset(BootRecord br, short fat_number)
{
    return (br.bytes_per_sector) * (br.reserved_sector_count) * (fat_number);
}

//retorna a posição em bytes da seção de dados a partir do byte 0
int dataSectionOffset(BootRecord br)
{
    return (br.root_entry_count * 32) *  br.bytes_per_sector;
} 