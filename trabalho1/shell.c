#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define MAX 50

#define PIPE 1
#define SEMICOLON 2
#define UNIQUE_AND 3
#define AND 4
#define OR 5

#define BACKGROUND 1

// flag para gerenciar o comando condicional (AND=4, OR=5)
int current_condition = 0;

// flag para gerenciar comando em background, 0 = false, 1 = true
int background = 0;

// conta quantidade total do character informado
int count_characters(int type, char **argv)
{
    char character;
    switch (type)
    {
    case PIPE:
        character = '|';
        break;
    case SEMICOLON:
        character = ';';
        break;
    case UNIQUE_AND:
        character = '&';
        break;
    }

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
int get_char_pos(int type, int i, char **argv)
{
    char character;
    switch (type)
    {
    case PIPE:
        character = '|';
        break;
    case SEMICOLON:
        character = ';';
        break;
    case UNIQUE_AND:
        character = '&';
        break;
    }

    while (argv[i])
    {
        if (*argv[i] == character)
            return i;
        i++;
    }
    return -1;
}

// retorna posicao do primeira '&&' ou '||' que encontrar
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
    pid_t pid;
    pid = fork();
    if (pid == 0)
    { // filho executa comando
        if (execvp(cmd[0], cmd) < 0)
        {
            perror("execvp exec_command");
            return -1;
        }
        return 0;
    }
    else if (pid > 0)
    { // pai aguarda filho se não for processo background
        int status;
        if (background != BACKGROUND)
        {
            waitpid(pid, &status, 0);
        }
        return WEXITSTATUS(status);
    }
    else
    {
        return -1;
    }
}

int exec_command_pipes(char **argv, int n_pipes)
{
    int fd[2], i = 0, n;
    int aux = STDIN_FILENO;

    for (int j = 0; j <= n_pipes; j++)
    {
        // formata o comando atual de acordo com a posicao do pipe retornada
        n = get_char_pos(PIPE, i, argv);
        char **cmd = &argv[i];
        if (n != -1)
            cmd[n - i] = NULL;

        if (pipe(fd) < 0)
        {
            perror("pipe");
            return -1;
        }

        pid_t filho = fork();
        if (filho == 0)
        { // filho executa comando
            close(fd[0]);
            dup2(aux, STDIN_FILENO); //duplica leitura do pipe sobre entrada padrao

            if (j < n_pipes)
                dup2(fd[1], STDOUT_FILENO); // duplica saida padrao do filho para escrita do pipe

            if (execvp(cmd[0], cmd) < 0)
            {
                perror("execvp pipe filho");
                return -1;
            }
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
        current_condition = (cmd[pos][0] == '&') ? AND : OR;
        cmd[pos] = NULL;
    }
    else
    {
        current_condition = 0;
    }

    // com o retorno do exec_command verifico se será executado o prox comando condicional
    int res = exec_command(cmd);

    if ((current_condition == AND && res == 0) ||
        (current_condition == OR && res > 0))
    {
        //rodou o comando e prossegue pro condicional '&&'
        //ou não rodou o comando e prossegue pro condicional '||'

        printf("\n");
        int pos = get_cond_char_pos(cond_cmd);
        return exec_command_conditional(cond_cmd, pos);
    }

    return res;
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        printf("Uso: %s <comando1> <parametros> '|' ...  <comando N> <parametros> \n", argv[0]);
        printf("ou \n %s <comando1> <parametros> ... ';' <comando N> <parametros> \n", argv[0]);
        printf("Para comandos condicionais utilize: \n");
        printf("%s <comando1> <parametros> ... '&&' <comando N> <parametros> \n", argv[0]);
        printf("ou \n %s <comando1> <parametros> ... '||' <comando N> <parametros> \n", argv[0]);
        printf("E para executar um comando em background: \n");
        printf("%s <comando1> <parametros> '&' \n", argv[0]);
        printf("Máximo 50 comandos. \n");
        return 0;
    }

    errno = 0;
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