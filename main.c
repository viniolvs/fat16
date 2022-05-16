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

    fat16 *fat=NULL;
    
    format83 f83;
    fseek(file, rootDirOffset(br), SEEK_SET);
    for (int j = 0; j < 4; j++)
    {
        read83(&f83, file);
        for (int i = 0; i < 11; i++)
        {
            printf("%c", f83.filename[i]);
        }
        printf("\n");
        printf("%x\n", f83.attribute);
    }
    
    return 0;
}
