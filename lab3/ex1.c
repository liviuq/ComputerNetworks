/*Exercitiu - pipe() 

1. Scrieti un program in care se foloseste un singur pipe pentru
comunicare si in care:
tatal scrie intr-un pipe un numar;
fiul verifica daca numarul este prim 
si transmite prin pipe tatalui raspunsul (yes, no);
tatal va afisa raspunsul primit.
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

int8_t is_prime(int32_t number)
{
    if (number <= 1)
    {
        return 0;
    }

    if (number <= 3)
    {
        return 1;
    }

    if (number % 2 == 0 || number % 3 == 0)
    {
        return 0;
    }

    for (uint32_t i = 5; i * i <= number; i = i + 6)
    {
        if (number % i == 0 || number % (i + 2) == 0)
        {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s number\n", *argv);
        exit(EXIT_FAILURE);
    }

    //creating the pipe
    int32_t pipefd[2];
    if (pipe(pipefd) == -1)
    {
        printf("Error on pipe. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //forking the execution
    pid_t cpid = fork();
    if (cpid == -1)
    {
        printf("Error on forking. REason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //child process
    if (cpid == 0)
    {
        //storage for the number in the pipe
        int32_t number;

        //reading from the pipe
        int32_t bytes = read(pipefd[0], &number, sizeof(number));
        if (bytes == -1)
        {
            printf("Error on read. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        //closing the reading end
        close(pipefd[0]);

        //checking the number
        if (is_prime(number))
        {
            bytes = write(pipefd[1], "yes", 4);
            if (bytes == -1)
            {
                printf("Error on write. Reason: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            bytes = write(pipefd[1], "no", 3);
            if (bytes == -1)
            {
                printf("Error on write. Reason: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        //closing the write end
        close(pipefd[1]);

        return EXIT_SUCCESS;
    }
    else //parent process
    {
        //writing the number to the pipe
        int32_t number_int = atoi(argv[1]);
        int32_t bytes = write(pipefd[1], &number_int, sizeof(int32_t));
        if (bytes == -1)
        {
            printf("Error on write. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        //closing the write end
        close(pipefd[1]);

        //waiting for the child process to finish
        wait(NULL);

        uint8_t *buffer = malloc(sizeof(uint8_t) * 8);
        if (buffer == NULL)
        {
            printf("Error at malloc. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        //reading from the pipe
        bytes = read(pipefd[0], buffer, 8);
        if (bytes == -1)
        {
            printf("Error on reading. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        //closing the remaining ends
        close(pipefd[0]);

        //printing the answer
        printf("Answer: %s\n", buffer);

        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}