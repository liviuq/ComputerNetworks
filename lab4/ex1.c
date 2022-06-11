/*
1. Creati un program in care tatal trimite un sir fiului.
Acesta concateneaza la sirul primit un alt sir si va
intoarce procesului tata raspunsul obtinut.
Pentru comunicare se va folosi primitiva socketpair.
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
#include <sys/socket.h>

int main(int argc, char **argv)
{
    // parent sends to kid string1
    // kid sends to parent string1string2

    // arg checking
    if (argc != 3)
    {
        printf("Usage: %s string1 string2\n", *argv);
        exit(EXIT_FAILURE);
    }

    // creating the socketpair
    int32_t sockfd[2]; // 0 -> parent
                       // 1 -> child
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd) == -1)
    {
        printf("Error on line %d. %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid_t cpid = fork();
    if (cpid == -1)
    {
        printf("Error on line %d. %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (cpid == 0) // child
    {
        // size of the message
        int32_t length;
        int32_t bytes = read(sockfd[1], &length, sizeof(int32_t));
        if (bytes == -1)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // initialising the buffer that we will send back
        char *buffer = malloc(sizeof(char) * (length + 1 + strlen(argv[2])));
        if (buffer == NULL)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // reading the string
        char *string1 = malloc(sizeof(char) * length);
        if (string1 == NULL)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
        bytes = read(sockfd[1], string1, length);
        if (bytes == -1)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // concatenating the strings
        if (snprintf(buffer, (length + strlen(argv[2]) + 1), "%s%s", string1, argv[2]) < 0)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // writing the size of the buffer first
        int32_t str2len = strlen(buffer);
        bytes = write(sockfd[1], &str2len , sizeof(int32_t));
        if (bytes == -1)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // writing the whole result
        bytes = write(sockfd[1], buffer, (length + strlen(argv[2]) + 1));
        if (bytes == -1)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // closing
        free(buffer);
        close(sockfd[0]);
        close(sockfd[1]);
    }
    else // parent
    {
        // sending the size of string1
        int32_t str1len = strlen(argv[1]);
        int32_t bytes = write(sockfd[0], &str1len, sizeof(int32_t));
        if (bytes == -1)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // sending string1
        bytes = write(sockfd[0], argv[1], strlen(argv[1]));
        if (bytes == -1)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // reading the size of the full buffer
        int32_t bufferlen;
        bytes = read(sockfd[0], &bufferlen, sizeof(int32_t));
        if (bytes == -1)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // create the buffer
        char *buffer = malloc(sizeof(char) * bufferlen);
        if (buffer == NULL)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // reading the result into buffer
        bytes = read(sockfd[0], buffer, bufferlen);
        if (bytes == -1)
        {
            printf("Error on line %d. %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // printing buffer
        printf("The res is %s\n", buffer);

        // closing
        free(buffer);
        close(sockfd[0]);
        close(sockfd[1]);
    }
    return EXIT_SUCCESS;
}