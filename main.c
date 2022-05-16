//gcc -g *.c -o exe

#include "fat16.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    //char *filename = argv[1];
    char *filename = "fat16_1sectorpercluster.img";
    FILE *file;
    file = openFile(filename);

    BootRecord br;
    readBootRecord(&br, file);
    fat16 *fat=NULL;
    format83 *f83 = getRootDir(br, file);
    format83 *p = f83;

    int arq;
    printf("ROOT DIRECTORIE:\n");
    for(int i = 0; p->attribute == 0x10 || p->attribute == 0x20 ; i++, *p++)
        printf("[%d] %s\n",i,p->filename);
    
    printf("Escolha qual arquivo deseja acessar: ");
    scanf("%d", &arq);
    
    
    return 0;
}
