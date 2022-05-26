//gcc -g *.c -o exe

#include "fat16.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//Função para menu interativo
void filesMenu(BootRecord br, int offset, FILE *file)
{
    format83 *p;
    //Verifica os arquivos no diretório atual
    format83 *current_dir = readDir(br, offset, file);
    int i, arq;

    //Verifica se o diretorio está vazio
    if (current_dir == NULL)
    {
        printf("Diretório vazio!\n");
        return;
    }
    //Printa os arquivos | diretórios
    for(i = 0, p = current_dir; p->attribute == 0x10 || p->attribute == 0x20 ; i++, p++)
        printf("[%d] %s\n", i, p->filename);
    printf("Escolha qual arquivo ou subdiretório deseja acessar: \nDigite -1 para sair\n");
    scanf("%d",&arq);


    if (arq == -1 )
        return;
    //Caso seja um diretório
    else if (current_dir[arq].attribute == 0x10)
    {
        //Verifica se está tentando acessar o diretório raiz
        if(current_dir[arq].first_cluster == 0x0)
            filesMenu(br, rootDirOffset(br), file);
        else
        {
            //Verifica o offset do diretório a ser acessado
            int dir_offset = findCluster(br, current_dir[arq].first_cluster);
            filesMenu(br, dir_offset, file);
        }
    }
    //Caso seja um arquivo
    else if(current_dir[arq].attribute == 0x20)       
    {
        fat16 *fat=NULL;
        //Aloca a FAT
        fat = readFat(br,1,file);
        int *file_clusters = NULL;
        //Verifica quais são os clusters do arquivo
        file_clusters = getFileClusters(br, fat, current_dir[arq]);
        //Printa o arquivo
        printFile(br, file_clusters, current_dir[arq], file);

        //Libera os arquivos
        free(fat);
        free(file_clusters);

        //Mantêm no diretório
        filesMenu(br, offset, file);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Insira o nome do arquivo de imagem!\n");
        return 0;
    }
    char *filename = argv[1];
    FILE *file;
    file = openFile(filename);

    BootRecord br;
    readBootRecord(&br, file);
    format83 *f83 = readDir(br, rootDirOffset(br), file);
    format83 *p = f83;

    int arq, i;

    filesMenu(br,rootDirOffset(br),file);

    return 0;
}
