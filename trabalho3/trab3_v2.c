/*

----- Integrantes do grupo ------

Amanda Aparecida Machado Goulart, RA: 133569
Milena de Matos Siqueira, RA: 122.044
Pedro Gabriel da Silva, RA: 120.887

*/

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#define N_TICKS 100

// memoria real (MR)
#define MR_SIZE 4

// memoria virtual MV (> MR)
#define MV_SIZE 6

#define MAX_BITS 8
#define MAX_DEC 255
#define BITS 2

typedef struct
{
    int id;
    int contador;
    int bits[MAX_BITS];
    int bitsDec;
} pagina;

int sortearPagina()
{
    //sorteia um valor entre 0 e 6
    return rand() % MV_SIZE;
}

void sortearBitR(int bitR[])
{
    for (int i = 0; i < MV_SIZE; i++)
    {
        bitR[i] = rand() % BITS;
    }
}

pagina encontrarPagina(pagina *paginas, int id)
{
    int i;

    for (i = 0; i < MV_SIZE; i++)
    {
        if (paginas[i].id == id)
        {
            return paginas[i];
        }
    }
}

//conversão de binário para decimal
int converteBinario(int b[])
{
    int d = 0;

    for (int i = 0; i < MAX_BITS; i++)
    {
        if (b[i] == 1)
        {
            d = d + pow(2, (MAX_BITS - 1) - i);
        }
    }

    return d;
}

//adiciona o bit na posição mais significativa
void adicionarBit(pagina pagina[], int i, int bit)
{
    for (int max = MAX_BITS; max > 0; max--)
    {
        pagina[i].bits[max] = pagina[i].bits[max - 1];
    }

    pagina[i].bits[0] = bit;
    pagina[i].bitsDec = converteBinario(pagina[i].bits);
}

void listarMV(pagina *memoria)
{
    int i, j;

    printf("\nMEMORIA VIRTUAL: \n");
    for (i = 0; i < MV_SIZE; i++)
    {
        printf("Pagina --> ");
        printf("id=%d, contagem=%d \n", memoria[i].id, memoria[i].contador);
        for (j = 0; j < MAX_BITS; j++)
        {
            printf("[%d] ", memoria[i].bits[j]);
        }
        printf(" --> %d \n\n", memoria[i].bitsDec);
    }
}

void listarMR(int *memoria)
{
    int i;

    printf("MEMORIA REAL: \n");
    for (i = 0; i < MR_SIZE; i++)
    {
        printf("[%d] ", memoria[i]);
    }

    printf("\n\n");
}

void inicia(pagina *paginas, int *memoria)
{
    int i;

    for (i = 0; i < MV_SIZE; i++)
    {
        paginas[i].id = i;
        paginas[i].contador = 0;
        paginas[i].bitsDec = 0;
        for (int j = 0; j < MAX_BITS; j++)
        {
            paginas[i].bits[j] = 0;
        }
    }

    for (i = 0; i < MR_SIZE; i++)
    {
        //preenche a memoria na inicialização (com id das paginas 0,1,2, 3)
        memoria[i] = i;
    }
}

// retorna posição do menos recentemente utilizado, buscando a página que tem o menor valor decimal de bits R
int encontrarMRU(pagina *paginas, int *memoria)
{
    int i, minimum = MAX_DEC, pos = 0;

    for (i = 0; i < MR_SIZE; i++)
    {
        pagina pagina = encontrarPagina(paginas, memoria[i]);
        if (pagina.bitsDec < minimum)
        {
            minimum = pagina.bitsDec;
            pos = i;
        }
    }
    return pos;
}

void limparBits(pagina pagina)
{
    for (int i = 0; i < MAX_BITS; i++)
    {
        pagina.bits[i] = 0;
    }
}

void atualizarMemoria(pagina *paginas, int *memoria, int id, int pageMiss, int bitR[])
{
    int i;

    for (i = 0; i < MV_SIZE; i++)
    {
        if (bitR[i] == 1)
        {
            paginas[i].contador++;
            adicionarBit(paginas, i, 1);
        }
        else
        {
            adicionarBit(paginas, i, 0);
        }
    }

    if (pageMiss == 1)
    {
        int pos = encontrarMRU(paginas, memoria);
        printf("substituir página id=%d \n", memoria[pos]);

        pagina pagina = encontrarPagina(paginas, id);
        limparBits(pagina);
        pagina.bits[0] = 1;
        pagina.bitsDec = converteBinario(pagina.bits);
        pagina.contador++;

        memoria[pos] = id;
    }
}

void listarBitsR(int bitR[])
{
    for (int i = 0; i < MV_SIZE; i++)
    {
        printf("%d\t", bitR[i]);
    }
}

int main()
{
    pagina contador[MV_SIZE];
    int memoria[MV_SIZE];

    inicia(contador, memoria);

    int i, j;

    int bitR[MR_SIZE];

    srand(time(NULL));

    listarMV(contador);
    listarMR(memoria);

    for (i = 0; i < N_TICKS; i++)
    {
        int id = sortearPagina();
        sortearBitR(bitR);

        printf("*********BIT R************\n");

        listarBitsR(bitR);

        int pageMiss = 1;

        for (j = 0; j < MR_SIZE; j++)
        {
            if (memoria[j] == id)
            {
                pageMiss = 0;
                break;
            }
        }

        if (pageMiss == 1)
        {
            printf("\npagina sorteada id=%d: page miss \n", id);
        }
        else
        {
            printf("\npagina sorteada id=%d: nao ocorreu page miss \n", id);
        }

        atualizarMemoria(contador, memoria, id, pageMiss, bitR);

        listarMV(contador);
        listarMR(memoria);
    }

    return 1;
}