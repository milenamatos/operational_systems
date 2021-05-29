#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX 50

#define PIPE 1
#define SEMICOLON 2

int count_characters(int type, char **argv)
{
    char character = (type == PIPE) ? '|' : ';';
    int count = 0, i = 0;
    while (argv[i])
    {
        if (*argv[i] == character)
            count++;
        i++;
    }
    return count;
}

int get_char_pos(int type, int i, char **argv)
{
    char character = (type == PIPE) ? '|' : ';';
    while (argv[i])
    {
        if (*argv[i] == character)
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
    }
    else if (pid > 0)
    { // pai aguarda filho
        waitpid(pid, NULL, 0);
    }
    return 1;
}

int exec_command_pipes(char **argv, int n_pipes)
{
    int fd[2], i = 0, n;
    int aux = STDIN_FILENO;

    for (int j = 0; j <= n_pipes; j++)
    {
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

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        printf("Uso: %s <comando1> <parametros> '|' ...  <comando N> <parametros> \n", argv[0]);
        printf("ou \n %s <comando1> <parametros> ... ';' <comando N> <parametros> \n", argv[0]);
        printf("Máximo 50 comandos. \n");
        return 0;
    }

    char **command = &argv[1];

    int n = count_characters(PIPE, command);

    if (n == 0)
    {
        n = count_characters(SEMICOLON, command);
        if (n == 0) {
            exec_command(command);
        }
        else {
            int j = 0, p;
            for (int i = 0; i <= n; i++) {
                p = get_char_pos(SEMICOLON, j, command);
                char **current_cmd = &command[j];
                current_cmd[p-j] = NULL;
                exec_command(current_cmd);
                printf("\n\n");
                j = p+1;
            }
        }
    }
    else
    {
        exec_command_pipes(command, n);
    }

    return 0;
}