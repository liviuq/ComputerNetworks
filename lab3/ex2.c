/*
Exercitiu - fifo() si dup()

2. Scrieri un program care sa simuleze comanda:
   cat prog.c | grep "include" > prog.c , folosind fifo-uri si primitiva dup.
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

    //creating the fifo
    if (mkfifo("fifo", 0600) == -1)
    {
        printf("[6]Error on mkfifo. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int32_t file = open("temp", O_RDWR | O_CREAT, 0777);
    if (file == -1)
    {
        printf("[9]Error at opening the file. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid_t cat = fork();
    if (cat == -1)
    {
        printf("Error on fork. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (cat == 0)
    {
        //close file
        close(file);

        //open the fifo
        int32_t fifofd = open("fifo", O_WRONLY);
        if (fifofd == -1)
        {
            printf("[7]Error at opening the fifo. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        //doing cat
        dup2(fifofd, 1);

        execlp("cat", "cat", "prog.c", NULL);
    }

    pid_t grep = fork();
    if (grep == -1)
    {
        printf("Error on fork. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (grep == 0)
    {
        //open the fifo
        int32_t fifofd = open("fifo", O_RDONLY);
        if (fifofd == -1)
        {
            printf("[8]Error at opening the fifo. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        //redirect
        dup2(fifofd, 0);
        close(fifofd);

        dup2(file, 1);
        close(file);

        execlp("grep", "grep", "include", NULL);
    }

    lseek(file, 0, SEEK_SET);
    int32_t prog = open("prog.c", O_RDWR, 0777);
    lseek(prog, SEEK_SET, 0);
    char* buffer = malloc( 200 * sizeof(char));
    int bytes;
    bytes = read(file, buffer, 20);
    printf("%d %s\n",bytes,  buffer);
    bytes = write(prog, buffer, 20);
    printf("written %d\n", bytes);
    return EXIT_SUCCESS;
}