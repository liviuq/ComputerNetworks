#define _POSIX_SOURCE
/*Exercitiu - Semnale

1. Sa se scrie un program care afiseaza din 3 in 3 secunde pid-ul procesului curent
si a cata afisare este. La primirea semnalului SIGUSR1 se scrie intr-un fisier mesajul 
"Am primit semnal".
Semnalul SIGINT se va ignora in primele 60 de secunde de la inceperea rularii programului 
si apoi va avea actiunea default.
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

void handler(int signum)
{
    if (signum == SIGUSR1)
    {
        //create the file where we write the message
        int32_t fd = open("out", O_RDWR | O_CREAT | O_TRUNC, 0600);
        if (write(fd, "Am primit semnal.\n", 20) == -1)
        {
            printf("Error at writing to the file. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        close(fd);
    }

    //check if the signal is the one from the alarm() call
    if (signum == SIGALRM)
    {
        //default SIGINT
        if (signal(SIGINT, SIG_DFL) == SIG_ERR)
        {
            printf("Error at defaulting the signal. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv)
{
    uint32_t seconds = 0; //counting the time
    uint32_t times_called = 0;

    //attaching the signal handler to the signal SIGUSR1
    if (signal(SIGUSR1, handler) == SIG_ERR)
    {
        printf("Error at signal(). Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //attaching the signal handler to the signal SIGALRM
    if (signal(SIGALRM, handler) == SIG_ERR)
    {
        printf("Error at signal(). Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*sigset_t set;
    if (sigemptyset(&set) == -1)
    {
        printf("Error at unsetting the mask. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&set, SIGINT) == -1)
    {
        printf("Error adding the signal to the mask. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigprocmask(SIG_SETMASK, &set, NULL) == -1)
    {
        printf("Error setting the mask. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    */
    if (signal(SIGINT, SIG_IGN) == SIG_ERR)
    {
        printf("Error at signal(). Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //alarm() always executes successfully, as for the RETURN paragraph in man 2 alarm
    alarm(60); //check for SIGALRM
    while (1)
    {
        if (seconds % 3 == 0)
        {
            times_called++;
            printf("[PID %d]I have been called %d times\n", getpid(), times_called);
        }
        /*if (seconds == 60)
        {
            if (sigemptyset(&set) == -1)
            {
                printf("Error at unsetting the mask. Reason: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (sigprocmask(SIG_SETMASK, &set, NULL) == -1)
            {
                printf("Error setting the mask. Reason: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        */
        sleep(1);
        seconds++;
    }

    return EXIT_SUCCESS;
}