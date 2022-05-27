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
    //Aloca a FAT
    fat16 *fat = newFat(fatSize(br));
    //Posiciona o ponteiro no inicio da FAT
    fseek(file, fatOffset(br,fat_number), SEEK_SET); 
    //Lê a FAT
    fread(fat, fatSize(br), 1,  file);
    return fat;
}

int getFileClusterCount(format83 f83, BootRecord br)
{
    int n=0;
    //Caso o arquivo possua um tamanho menor que o tamanho de um cluster
    //Ele retorna 1
    if (f83.file_size <= br.bytes_per_sector)
        return 1;
    //Se não, ele verifica se tem um cluster "sobrando"
    else if(f83.file_size%br.bytes_per_sector)
        n = 1;
    //Faz o cálculo para verificar quantos clusters possui
    n+=(f83.file_size/br.bytes_per_sector) / br.sectors_per_cluster;
    return n;
}

int* getFileClusters(BootRecord br, fat16 *fat, format83 f83)
{
    //Recebe quantos clusters possui
    int cluster_count = getFileClusterCount(f83, br);
    //Aloca um vetor de int para mostrar os clusters
    int *clusters = (int*)malloc(cluster_count);

    //Pega o primeiro cluster
    unsigned short aux = fat[f83.first_cluster];
    //O primeiro cluster é o primeiro cluster do arquivo
    clusters[0] = f83.first_cluster;

    for (int i = 1; i < cluster_count; i++)
    {
        //A posição atual do vetor vai receber o cluster anterior 
        clusters[i] = aux;
        //Se não chegou ao fim, vai continuar lendo os clusters
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
        //Verifica se está vazio
        if(f83->filename[0] == 0x0)
            break;
        //Verifica se é um longfile ou um arquivo deletado
        //Caso seja um diretorio ou arquivo valido, ele contabiliza +1 arquivo
        if((f83->attribute == 0x10 || f83->attribute == 0x20) && (f83->filename[0] != 0xe5))
            k++;
    }
    //Caso seja um diretório vazio, ele retorna
    if (k==0)
        return NULL;

    //Aloca um vetor de format83 para armazenar os arquivos válidos
    format83 *aux = (format83*)malloc(sizeof(format83) * k);
    //Reinicia o ponteiro do arquivo
    fseek(file, offset, SEEK_SET);
    for (i = 0, j = 0; i < l; i++)
    {
        //Pega os dados do arquivo
        read83(f83, file);
        //Verifica novamente se é um arquivo valido
        if((f83->attribute == 0x10 || f83->attribute == 0x20) && (f83->filename[0] != 0xe5))
            //Caso seja, ele vai armazenar os dados no vetor auxiliar
            aux[j++] = *f83;
    }
    //Libera lixos
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
    //Recebe o tamanho do arquivo
    int file_size = f83.file_size;
    //Recebe o número de clusters
    int count_cluster = getFileClusterCount(f83,br);
    //Verifica se possui um cluster "sobrando"
    int modulo = file_size % (count_cluster * clusterSize(br));
    //Vetor para leitura dos clusters
    char *arquivo = NULL;

    for (int i = 0; i < count_cluster; i++)
    {
        //Posiciona o ponteiro no inicio do cluster atual
        fseek(file,findCluster(br,clusters[i]), SEEK_SET);
        //Verifica se está no final do arquivo
        if (i + 1 == count_cluster && modulo != 0)
        {
            //Aloca um vetor de char para armazenar o tamanho (cluster com o tamanho do cluster "sobrando")
            arquivo = (char*)malloc(modulo);
            fread(arquivo, sizeof(char), modulo, file);
        }
        else
        {
            //Aloca um vetor de char para armazenar o tamanho (cluster com o tamanho do cluster inteiro)
            arquivo = (char*)malloc(clusterSize(br));
            fread(arquivo, sizeof(char), clusterSize(br), file);
        }
        //Printa o arquivo
        printf("%s",arquivo);
        //Libera o vetor
        if(arquivo != NULL)
            free(arquivo);
    }
    printf("\n");
}