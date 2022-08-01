#include "stdio.h"
#include "stdlib.h"

//memória real
#define MR_SIZE 4

//memória virtual (> MR)
#define MV_SIZE 6

int n_ticks;

typedef struct
{
    int id;
    int contador;
} pagina;

int sortearPagina()
{
    //sorteia um valor entre 0 e 8
    return rand() % MV_SIZE;
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

void inicia(pagina *paginas, pagina *memoria)
{
    int i;

    for (i = 0; i < MV_SIZE; i++)
    {
        paginas[i].id = i;
        paginas[i].contador = 0;
    }

    for (i = 0; i < MR_SIZE; i++)
    {
        //preenche a memoria na inicialização (com paginas 0,1,2,3) mas sem realizar contagem
        pagina pagina = encontrarPagina(paginas, i);
        memoria[i] = pagina;
    }
}

// retorna posição do menos recentemente utilizado, buscando a página que tem o menor contador
int encontrarMRU(pagina *memoria)
{
    int i, minimum = n_ticks, pos;

    for (i = 0; i < MR_SIZE; i++)
    {
        if (memoria[i].contador < minimum)
        {
            minimum = memoria[i].contador;
            pos = i;
        }
    }
    return pos;
}

void atualizarMemoria(pagina *paginas, pagina *memoria, int id, int pageMiss)
{
    int i;

    if (pageMiss == 0)
    {
        //soma o contador na página sorteada
        for (i = 0; i < MR_SIZE; i++)
        {
            if (memoria[i].id == id)
            {
                memoria[i].contador++;
                break;
            }
        }
    }
    else
    {
        //faz a substituição
        int pos = encontrarMRU(memoria);
        printf("substituir página id=%d \n", memoria[pos].id);

        pagina pagina = encontrarPagina(paginas, id);
        pagina.contador++;

        memoria[pos] = pagina;
    }
}

void listarMemoria(pagina *memoria)
{
    int i;

    printf("MEMÓRIA: \n");
    for (i = 0; i < MR_SIZE; i++)
    {
        printf("id=%d, contagem=%d \n", memoria[i].id, memoria[i].contador);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Uso comando: %s <número total de ticks> \n", argv[0]);
        return 0;
    }

    n_ticks = atoi(argv[1]);

    pagina contador[MV_SIZE];
    pagina memoria[MR_SIZE];

    inicia(contador, memoria);

    int i, j;

    for (i = 0; i < n_ticks; i++)
    {
        int id = sortearPagina();
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
            printf("\npágina sorteada id=%d: page miss \n", id);
        }
        else
        {
            printf("\npágina sorteada id=%d: não ocorreu page miss \n", id);
        }

        atualizarMemoria(contador, memoria, id, pageMiss);
        listarMemoria(memoria);
    }

    return 0;
}