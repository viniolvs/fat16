#include "fat16.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    char *filename = "fat16_4sectorpercluster.img";
    FILE *file;
    file = openFile(filename);
    
    BootRecord br;
    readBootRecord(&br, file);
    printf("Bytes per sector %hd \n", br.bytes_per_sector);
    printf("Root Dir offset = %d\n", rootDirOffset(br));

    /*fat16 *fat[br.table_count]; 
    for (int i = 0; i < br.table_count; i++)
    {
        fat[i] = newFat(br);
        readFat(br, fat[i], i, file);
    }*/

    fat16 *fat=NULL;
    
    fat = readFat(br ,1,file);
    printf("%x\n",fat[7]);
    
    return 0;
}
