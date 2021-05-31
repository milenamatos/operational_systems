/*
    Milena de Matos Siqueira, RA: 122044
    Pedro Gabriel da Silva, RA: 120887

    Trabalho 1: Shell
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define MAX 50

//encadeamento de comandos para comunicação entre processos
#define PIPE '|'
#define SEMICOLON ';'
#define UNIQUE_AND '&'

#define AND 1
#define OR 2

#define BACKGROUND 1

// flag para gerenciar o comando condicional (AND=4, OR=5)
int current_condition = 0;

// flag para gerenciar comando em background, 0 = false, 1 = true
int background = 0;

// conta quantidade total do character informado
int count_characters(char character, char **argv)
{
    int count = 0, i = 0;
    while (argv[i])
    {
        if (*argv[i] == character)
            count++;
        i++;
    }
    return count;
}

// retorna primeira posicao encontrada do character informado
int get_char_pos(char character, int i, char **argv)
{
    while (argv[i])
    {
        if (*argv[i] == character)
            return i;
        i++;
    }
    return -1;
}

// retorna posicao do primeiro '&&' ou '||' que encontrar
int get_cond_char_pos(char **argv)
{
    int i = 0;
    while (argv[i])
    {
        if ((argv[i][0] == '&' && argv[i][1] == '&') ||
            (argv[i][0] == '|' && argv[i][1] == '|'))
            return i;
        i++;
    }
    return -1;
}

int exec_command(char **cmd)
{
    pid_t pid; //instancia o indentificador do processo

    pid = fork();            //criação do processo filho a partir do pid pai como uma replica em memoria
    if (pid == 0)            //se for o filho executa os comandos, ja que filho é uma copia do pai com pid = 0 para ele mesmo no retorno, os outros o indentificam com outro pid
    {                        // filho executa comando
        execvp(cmd[0], cmd); //fornece um vetor de ponteiros representando a lista de argumentos para o processo
        return 0;
    }
    else if (pid > 0) //pai assume pid > 0
    {                 // pai aguarda filho se não for processo background
        int status;
        if (background != BACKGROUND) //se background = 0, entao há processos que precisam ser executados em background,
                                      //se background = 1, entao 1!=0 logo esse processo não precisa ser executado em segundo plano
        {
            waitpid(pid, &status, 0); //pai aguarda termino do filho
        }
        return WEXITSTATUS(status); // esta macro retorna o status do processo filho
    }
    else // não foi criado processo filho e portanto houve retorno negativo para o pid
    {
        return -1;
    }
}

int exec_command_pipes(char **argv, int n_pipes)
{
    int fd[2], i = 0, n; // fd[2] representa a criação de dois canais de comunicação, 1 para escrita, 0 para leitura de dados
    int aux = STDIN_FILENO;

    for (int j = 0; j <= n_pipes; j++)
    {
        // formata o comando atual de acordo com a posicao do pipe retornada
        n = get_char_pos(PIPE, i, argv);
        char **cmd = &argv[i]; //copia o comando referente a posição na matriz de argumento
        if (n != -1)
            cmd[n - i] = NULL;

        if (pipe(fd) < 0)
        {
            perror("pipe");
            return -1;
        }

        pid_t filho = fork(); // cria um processo filho com seu pid
        if (filho == 0)
        {                            // filho executa comando
            close(fd[0]);            //fecha o canal para leitura
            dup2(aux, STDIN_FILENO); //duplica pipe sobre entrada padrao

            if (j < n_pipes)
                dup2(fd[1], STDOUT_FILENO); // duplica saida padrao do filho para escrita do pipe

            execvp(cmd[0], cmd); //fornece um vetor de ponteiros representando a lista de argumentos para o processo
            return 0;
        }
        else if (filho > 0)
        { // pai
            aux = fd[0];
            close(fd[1]); // pai nao vai escrever
            waitpid(filho, NULL, 0);
        }
        else
        {
            perror("fork");
            return -1;
        }

        i = n + 1;
    }

    return 1;
}

int exec_command_conditional(char **cmd, int pos)
{
    char **cond_cmd;
    if (pos != -1)
    {
        // formata o comando atual e o prox comando corretamente
        cond_cmd = &cmd[pos + 1];
        current_condition = (cmd[pos][0] == UNIQUE_AND) ? AND : OR;
        cmd[pos] = NULL;
    }
    else
    {
        current_condition = 0;
    }

    // com o retorno do exec_command verifico se será executado o prox comando condicional
    int res = exec_command(cmd);

    if ((current_condition == AND && res == 0) ||
        (current_condition == OR && res != 0))
    {
        //rodou o comando e prossegue pro condicional '&&'
        //ou não rodou o comando e prossegue pro condicional '||'

        printf("\n");
        int pos = get_cond_char_pos(cond_cmd);
        return exec_command_conditional(cond_cmd, pos);
    }

    return res;
}

int main(int argc, char **argv) //argc é numero de argumentos e argv é a matriz de argumentos
{
    if (argc == 1) //usuario passou apenas um comando, então retorna o formato que deve ser pedido
    {
        printf("Uso: \n1 - %s <comando1> <parametros> '|' ...  <comando N> <parametros> \n", argv[0]);
        printf("2 - %s <comando1> <parametros> ... ';' <comando N> <parametros> \n", argv[0]);
        printf("Para comandos condicionais utilize: \n");
        printf("3 - %s <comando1> <parametros> ... '&&' <comando N> <parametros> \n", argv[0]);
        printf("4 - %s <comando1> <parametros> ... '||' <comando N> <parametros> \n", argv[0]);
        printf("5 - Para executar um comando em background: \n");
        printf("%s <comando1> <parametros> '&' \n", argv[0]);
        printf("Máximo 50 comandos. \n");
        return 0;
    }

    char **command = &argv[1];

    // verifica primeiro se possui algum comando condicional
    int pos = get_cond_char_pos(command);
    if (pos > 0)
    {
        exec_command_conditional(command, pos);
    }
    else
    {
        // senao tem condicional, verifica pipes
        int n = count_characters(PIPE, command);
        if (n > 0)
        {
            exec_command_pipes(command, n);
        }
        else
        {
            // senao tem pipes, verifica ponto e vírgula
            // neste caso apenas executa os comandos de forma independente
            n = count_characters(SEMICOLON, command);
            if (n > 0)
            {
                int j = 0, p;
                for (int i = 0; i <= n; i++)
                {
                    // sempre busca a posicao do proximo simbolo para chamar a funcao exec_command corretamente
                    p = get_char_pos(SEMICOLON, j, command);
                    char **current_cmd = &command[j];
                    current_cmd[p - j] = NULL;
                    exec_command(current_cmd);
                    printf("\n\n");
                    j = p + 1;
                }
            }
            else
            {
                // a ultima possibilidade é que seja apenas um comando simples
                // ou seguido de '&' que indica que vai rodar em background
                n = count_characters(UNIQUE_AND, command);
                if (n > 0)
                {
                    background = BACKGROUND;
                    pos = get_char_pos(UNIQUE_AND, 0, command);
                    command[pos] = NULL;
                }
                exec_command(command);
            }
        }
    }

    return 0;
}