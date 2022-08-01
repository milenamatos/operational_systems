/*

----- Integrantes do grupo ------

Amanda Aparecida Machado Goulart, RA: 133569
Milena de Matos Siqueira, RA: 122.044
Pedro Gabriel da Silva, RA: 120.887

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>

#define BUFSIZE 512
#define N_SECONDS 5

struct fileArgs
{
    pthread_t tid;
    char sourceFile[1000], targetFile[1000];
};

int isValidSubDir(char *dirName)
{
    if (strcmp(dirName, ".") != 0 && strcmp(dirName, "..") != 0)
    {
        return 1;
    }
    return 0;
}

void *copyFile(void *args)
{
    int from, to, nr, nw, n;

    char buf[BUFSIZE];

    struct fileArgs file = *(struct fileArgs *)args;

    if ((from = open(file.sourceFile, O_RDONLY)) < 0)
    {
        perror("Erro ao abrir arquivo fonte");
        exit(1);
    }

    if ((to = creat(file.targetFile, 0666)) < 0)
    {
        perror("Erro ao criar/copiar arquivo de destino");
        exit(2);
    }

    while ((nr = read(from, buf, sizeof(buf))) != 0)
    {
        if (nr < 0)
        {
            perror("Erro ao ler arquivo fonte");
            exit(3);
        }

        nw = 0;

        do
        {
            if ((n = write(to, &buf[nw], nr - nw)) < 0)
            {
                perror("Erro ao escrever no arquivo de destino");
                exit(4);
            }

            nw += n;
        } while (nw < nr);
    }

    printf("\n--> Arquivo '%s' copiado com sucesso para '%s'\n", file.sourceFile, file.targetFile);

    close(from);
    close(to);
    pthread_exit(NULL);
}

int createBackupDir(char *dirName)
{
    struct stat inode;
    if (stat(dirName, &inode) == -1)
    {
        return mkdir(dirName, 0700);
    }

    return 0;
}

void readDirectory(char *sourceDir, char *targetDir)
{
    if (createBackupDir(targetDir) == -1)
    {
        perror("Erro ao criar diretório de backup");
        exit(1);
    }

    char sourceFileName[1000], targetFileName[1000];
    struct dirent *dirEntry;
    struct stat inode, inodeBkp;
    struct fileArgs file[100];

    DIR *dir = opendir(sourceDir);
    int i = 0;

    while ((dirEntry = readdir(dir)) != 0)
    {
        //formata o nome dos arquivos
        sprintf(sourceFileName, "%s/%s", sourceDir, dirEntry->d_name);
        sprintf(targetFileName, "%s/%s", targetDir, dirEntry->d_name);

        lstat(sourceFileName, &inode);

        if (S_ISDIR(inode.st_mode))
        {
            //se é um diretorio chama a função recursivamente
            if (isValidSubDir(dirEntry->d_name) == 1)
            {
                printf("\n----> Verificando diretório '%s'\n", dirEntry->d_name);
                readDirectory(sourceFileName, targetFileName);
            }
        }

        else
        {
            //arquivo normal
            int res = lstat(targetFileName, &inodeBkp);
            if (res == -1 || inode.st_mtim.tv_sec > inodeBkp.st_mtim.tv_sec)
            {
                //se ainda não existe ou houve alteração, faz a cópia
                printf("\n--> Copiando arquivo '%s'\n", dirEntry->d_name);

                sprintf(file[i].sourceFile, "%s", sourceFileName);
                sprintf(file[i].targetFile, "%s", targetFileName);

                pthread_create(&file[i].tid, NULL, copyFile, &file[i]);
                i++;
            }
            else
            {
                printf("\n--> Arquivo '%s' não teve modificação\n", dirEntry->d_name);
            }
        }
    }

    for (int j = 0; j < i; j++)
    {
        pthread_join(file[j].tid, NULL);
    }
}

int main()
{
    char sourceDir[100], targetDir[200];

    printf("Informe o nome do diretório a ser copiado\n");
    scanf("%s", sourceDir);

    sprintf(targetDir, "%s_%s", sourceDir, "backup");

    DIR *dir = opendir(sourceDir);
    if (dir == 0)
    {
        perror("Erro ao abrir diretório fonte");
        exit(1);
    }
    closedir(dir);

    while (1)
    {
        printf("\n-----------CHECANDO ARQUIVOS DO DIRETÓRIO-------------\n");

        readDirectory(sourceDir, targetDir);
        sleep(N_SECONDS);
    }
}
