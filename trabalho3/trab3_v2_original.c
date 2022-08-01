#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N_TICKS 100
#define MV_SIZE 6 // memoria virtual
#define MR_SIZE 4 // memoria real
#define MAX_BITS 8
#define BITS 2

typedef struct
{
    int id;
    int contador;
    int bits[MAX_BITS];
    int bitsDec;
    int tiques;
} pagina;

int sortearPagina()
{
    return rand() % MV_SIZE;
}

void sortearBitR(int bitR[])
{
    for (int i = 0; i < MR_SIZE; i++)
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

int converteBinario(int b[]){
    int d = 0;

    for(int i = 0; i < MAX_BITS; i++){
        if(b[i] == 1){
            d = d + pow(2, (MAX_BITS-1)-i);
        }
    }

    return d;
}


void addBit(pagina *memoria, int i, int bit)
{
    // int max = memoria[i].contador;

    // memoria[i].bits[max] = bit;

    for (int max = MAX_BITS; max > 0; max--){
        memoria[i].bits[max] = memoria[i].bits[max - 1];
    }

    memoria[i].bits[0] = bit;
    memoria[i].bitsDec = converteBinario(memoria[i].bits);
}

void inicia(pagina *paginas, pagina *memoria)
{
    int i;

    for (i = 0; i < MV_SIZE; i++)
    {
        paginas[i].id = i;
        paginas[i].contador = 0;
        paginas[i].tiques = 0;
        for (int j = 0; j < MAX_BITS; j++)
        {
            paginas[i].bits[j] = 0;
        }
    }

    for (i = 0; i < MR_SIZE; i++)
    {
        //preenche a memoria na inicialização (com paginas 0,1,2,3)  mas sem realizar contagem
        pagina pagina = encontrarPagina(paginas, i);
        memoria[i] = pagina;
         memoria[i].bitsDec = 0;
        for (int b = 0; b < MAX_BITS; b++)
        {
            memoria[i].bits[b] = 0;
        }
    }
}

int encontrarMRU(pagina *memoria)
{
    int i, minimum = N_TICKS, pos;

    for (i = 0; i < MR_SIZE; i++)
    {
        if (memoria[i].bitsDec < minimum)
        {
            minimum = memoria[i].contador;
            pos = i;
        }
    }
    return pos;
}

void cleanBits(pagina pagina)
{
    for (int i = 0; i < MAX_BITS; i++)
    {
        pagina.bits[i] = 0;
    }
}

void atualizarMemoria(pagina *paginas, pagina *memoria, int id, int pageMiss, int bitR[])
{
    int i;

    if (pageMiss == 0)
    {
        for (i = 0; i < MR_SIZE; i++)
        {
            if (bitR[i] == 1)
            {
                memoria[i].contador++;
                memoria[i].tiques++;
                addBit(memoria, i, 1);
            }
            else
            {
                memoria[i].tiques++;
                addBit(memoria, i, 0);
            }
            // if (memoria[i].id == id)
            // {
            //     addBit(memoria, i, 1);
            //     memoria[i].contador++;
            //     break;
            // }
        }
    }
    else
    {
        int pos = encontrarMRU(memoria);
        printf("substituir página id=%d \n", memoria[pos].id);

        pagina pagina = encontrarPagina(paginas, id);
        cleanBits(pagina);
        pagina.bits[0] = 1;
        pagina.bitsDec = converteBinario(pagina.bits);
        pagina.contador++;

        memoria[pos] = pagina;
    }
}

void listarBitsR(int bitR[])
{
    for (int i = 0; i < MR_SIZE; i++)
    {
        printf("%d\t", bitR[i]);
    }
}

void listarMemoria(pagina *memoria)
{
    int i, j;

    printf("MEMORIA: \n");
    for (i = 0; i < MR_SIZE; i++)
    {
        printf("id=%d, contagem=%d \n", memoria[i].id, memoria[i].contador);
        printf("Pagina: %d ", i);
        for (j = 0; j < MAX_BITS; j++)
        {
            printf("[%d] ", memoria[i].bits[j]);
        }
        printf(" --> %d \n", memoria[i].bitsDec);
    }

    printf("\n");
}

int main()
{
    pagina contador[MV_SIZE];
    pagina memoria[MR_SIZE];

    inicia(contador, memoria);

    int i, j;

    int bitR[MR_SIZE];

    for (i = 0; i < N_TICKS; i++)
    {
        int id = sortearPagina();
        sortearBitR(bitR);

        printf("*********BIT R************\n");

        listarBitsR(bitR);

        int pageMiss = 1;

        for (j = 0; j < MR_SIZE; j++)
        {
            if (memoria[j].id == id)
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
        listarMemoria(memoria);
    }

    return 1;
}