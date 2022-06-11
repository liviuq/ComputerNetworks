/*
Exercitiu - Primitivele exec()

2. Scrieti un program care executa comanda ls -a -l. Trebuiesc respectate:
procesul fiu este cel care va apela primitiva execvp
procesul tata asteapta ca fiul sa se termine si va afisa un mesaj

2’. Aceeasi problema utilizand primitiva execv.
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char** argv)
{
    char* ls[] = {
        "ls",
        "-a",
        "-l",
        NULL
    }; 
    
    pid_t kid;
    if((kid = fork()) == -1)
    {
        printf("Error at forking. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(kid == 0) //child process
    {
        if(execvp("ls", ls) == -1)
        {
            printf("Error at execvp. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if(wait(NULL) == -1)
        {
            printf("Error at wait(). Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        printf("I'm the parent process and the child executed without problems.\n");
    }

    return EXIT_SUCCESS;
}