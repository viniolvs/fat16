//gcc -g *.c -o exe

#include "fat16.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    //char *filename = argv[1];
    char *filename = "fat16_4sectorpercluster.img";
    FILE *file;
    file = openFile(filename);

    BootRecord br;
    readBootRecord(&br, file);
    format83 *f83 = getRootDir(br, file);
    format83 *p = f83;

    int arq;
    printf("DIRETORIO RAIZ:\n");
    for(int i = 0; p->attribute == 0x10 || p->attribute == 0x20 ; i++, p++)
        printf("[%d] %s\n",i,p->filename);
    
    printf("Escolha qual arquivo deseja acessar: ");
    scanf("%d", &arq);
    
    fat16 *fat=NULL;
    fat = readFat(br,1,file);

    int *file_clusters = getFileClusters(br,fat,f83[arq]);
    for (int i = 0; i < 3; i++)
        printf("%d\n", file_clusters[i]);
    
    printf("%d\n",rootDirOffset(br));
    printf("%d\n",fatOffset(br,1));
    
    printFile(br, file_clusters, f83[arq], file);
    return 0;
}
