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
    // checking the arguments
    if (argc != 1)
    {
        printf("Usage: %s\n", *argv);
        exit(EXIT_FAILURE);
    }

    // setting up the communication between the server and client through a fifo
    static const char *cliser = "cliser";
    static const char *sercli = "sercli";

    // creating the fifo
    if (mkfifo(cliser, 0600) == -1)
    {
        if (errno != EEXIST) // if it already exist, do not end program, reuse fifo
        {
            printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // creating the fifo
    if (mkfifo(sercli, 0600) == -1)
    {
        if (errno != EEXIST) // if it already exist, do not end program, reuse fifo
        {
            printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // opening the fifo for server->client comms
    int32_t serclifd = open(sercli, O_RDONLY);
    if (serclifd == -1)
    {
        printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // opening the fifo for client->server comms
    int32_t cliserfd = open(cliser, O_WRONLY);
    if (serclifd == -1)
    {
        printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // buffer where we store the command
    char *buffer = malloc(sizeof(char) * 256);
    if (buffer == NULL)
    {
        printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int32_t bytes_read;
    int32_t bytes_written;
    do
    {
        printf("[CLIENT] Input the command: ");
        fflush(stdout);

        //reading client's command
        bytes_read = read(STDIN_FILENO, buffer, 100);
        buffer[bytes_read - 1] = '\0';
        printf("[CLIENT] Got from stdin %d bytes: %s\n", bytes_read, buffer);
        fflush(stdout);

        printf("[CLIENT] Writing to server: %s\n", buffer);
        fflush(stdout);
        bytes_written = write(cliserfd, buffer, bytes_read - 1);
        printf("[CLIENT] Written %d bytes\n", bytes_written);
        fflush(stdout);

        // length of the message
        int32_t message_length;
        char temp[4];
        bytes_read = read(serclifd, &temp, sizeof(message_length));
        message_length = atoi(temp);
        printf("[CLIENT] Received %d bytes. The message_length is %d\n", bytes_read, message_length);
        fflush(stdout);

        char *message_buffer = malloc(sizeof(char) * message_length + 1);
        bytes_read = read(serclifd, message_buffer, message_length);
        message_buffer[bytes_read] = '\0';
        printf("[CLIENT] Received %d bytes: %s\n-----------\n", bytes_read, message_buffer);
        fflush(stdout);
    } while ( bytes_read > 0);

        // closing
        free(buffer);
    close(cliserfd);
    close(serclifd);

    return EXIT_SUCCESS;
}