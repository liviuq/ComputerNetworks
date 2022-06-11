/*
Exercitiu - Primitiva fork() 

1. Scrieti un program in care procesul tata testeaza daca procesul fiu are un PID par. 
In cazul in care rezultatul este pozitiv, tatal ii scrie fiului mesajul “fortune”, mesaj care va fi afisat de fiu. 
Daca PID-ul este impar, tatal va scrie mesajul “lost” si moare inaintea fiului.

Detalii: Mesajul va fi un sir de caractere care va fi scris de procesul tata 
intr-un fisier si apoi citit de fiu. Amandoi cunosc denumirea fisierului.
*/
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    //creating an unnamed pipe with pipe()
    int32_t pipefd[2]; //[0] is for reading, [1] is for writing
    if (pipe(pipefd) == -1)
    {
        printf("Error on creating the pipe. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //forking
    pid_t process = fork();
    if (process == -1) //verify the return value of fork()
    {
        printf("Error at forking. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (process == 0) //the child process
    {
        //closing unused file descriptors from the pipe.
        close(pipefd[1]);

        //creating a buffer for the incoming message
        uint8_t *buffer = malloc(sizeof(char) * 256);
        if (buffer == NULL)
        {
            printf("Error at malloc. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        //reading from the pipe
        uint32_t bytes_read = read(pipefd[0], buffer, 256);
        if (bytes_read == -1)
        {
            printf("Error at reading. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        //waiting for the parent to end execution
        printf("%s\n", buffer);
    }

    if (process != 0) //we are in the parent
    {
        //closing unused file descriptors
        close(pipefd[0]);

        //checking child id
        if (process % 2 == 0)
        {
            if (write(pipefd[1], "fortune", 10) == -1)
            {
                printf("Error at writing. Reason: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            //closing the pipe write end
            close(pipefd[1]);
            wait(NULL);
        }
        else
        {
            if (write(pipefd[1], "lost", 5) == -1)
            {
                printf("Error at writing. Reason: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            printf("Parent dies.\n");
            close(pipefd[1]);
        }
    }

    return EXIT_SUCCESS;
}