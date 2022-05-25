#include "fat16.h"
#include <stdlib.h>
#include <stdio.h>

FILE* openFile(char *filename)
{
    FILE *file;
    file = fopen(filename, "rb");
    return file;
}

int bytes2sectors(int bytes, int bytes_per_sector)
{
	return (bytes / bytes_per_sector);
}

void readBootRecord(BootRecord *br, FILE *file)
{
    fseek(file, 0, SEEK_SET);
    fread(br, sizeof(BootRecord),1, file);
}

void read83(format83 *f83, FILE *file)
{
    fread(f83, sizeof(format83),1, file);
}

int fatSize(BootRecord br)
{
    return (br.table_size_16 * br.bytes_per_sector);
}

fat16* newFat(int fat_size)
{
    fat16 *fat;
    fat = (fat16*)malloc(fat_size);
    return fat;
}

fat16* readFat(BootRecord br, int fat_number, FILE *file)
{
    fat16 *fat = newFat(fatSize(br));
    fseek(file, fatOffset(br,fat_number), SEEK_SET); 
    fread(fat, fatSize(br), 1,  file);
    return fat;
}

int getFileClusterCount(format83 f83, BootRecord br)
{
    int n=0;
    if (f83.file_size <= br.bytes_per_sector)
        return 1;
    else if(f83.file_size%br.bytes_per_sector)
        n = 1;
    n+=(f83.file_size/br.bytes_per_sector) / br.sectors_per_cluster;
    return n;
}

int* getFileClusters(BootRecord br, fat16 *fat, format83 f83)
{
    int cluster_count = getFileClusterCount(f83, br);
    int *clusters = (int*)malloc(cluster_count);

    unsigned short aux = fat[f83.first_cluster];
    clusters[0] = f83.first_cluster;

    for (int i = 1; i < cluster_count; i++)
    {
        clusters[i] = aux;
        if(fat[aux]!=0xf)
            aux = fat[aux];
    }
    return clusters;
}

int clusterSize(BootRecord br)
{
    return (br.bytes_per_sector * br.sectors_per_cluster);
}

int findCluster(BootRecord br, int cluster)
{
    return (dataSectionOffset(br) + clusterSize(br) * (cluster - 2)); 
}

format83* readDir(BootRecord br, int offset, FILE *file)
{
    format83 *f83 = (format83*)malloc(sizeof(format83));
    fseek(file, offset, SEEK_SET);
    int k=0, j, i, l = 0;
    for (i = 0; ; i++, l++)
    {
        read83(f83, file);
        if(f83->filename[0] == 0x0)
            break;
        if((f83->attribute == 0x10 || f83->attribute == 0x20) && (f83->filename[0] != 0xe5))
            k++;
    }
    if (k==0)
        return NULL;
    format83 *aux = (format83*)malloc(sizeof(format83) * k);
    fseek(file, offset, SEEK_SET);
    for (i = 0, j = 0; i < l; i++)
    {
        read83(f83, file);
        if((f83->attribute == 0x10 || f83->attribute == 0x20) && (f83->filename[0] != 0xe5))
            aux[j++] = *f83;
    }
    free(f83);
    return aux;
}

int rootDirSize(BootRecord br)
{
    return (dataSectionOffset(br) - rootDirOffset(br));
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
    return br.root_entry_count * 32 + rootDirOffset(br);
}

void printFile(BootRecord br, int *clusters, format83 f83, FILE *file)
{
    int file_size = f83.file_size;
    int count_cluster = getFileClusterCount(f83,br);
    int modulo = file_size % (count_cluster * clusterSize(br));
    char *arquivo = NULL;

    for (int i = 0; i < count_cluster; i++)
    {
        fseek(file,findCluster(br,clusters[i]), SEEK_SET);
        if (i + 1 == count_cluster)
        {
            arquivo = (char*)malloc(modulo);
            fread(arquivo, sizeof(char), modulo, file);
        }
        else
        {
            arquivo = (char*)malloc(clusterSize(br));
            fread(arquivo, sizeof(char), clusterSize(br), file);
        }
        printf("%s",arquivo);
        if(arquivo != NULL)
            free(arquivo);
    }
    printf("\n");
}