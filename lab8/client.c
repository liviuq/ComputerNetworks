#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("[CLIENT] Usage: %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // creating the socket
    int32_t socketdescriptor;
    if ((socketdescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("[CLIENT] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // getting the port
    int32_t port = atoi(argv[2]);

    // struct used by the client
    struct sockaddr_in server;
    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, argv[1], &server.sin_addr);

    // connecting to the server
    if (connect(socketdescriptor, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        printf("[CLIENT] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // input the name of the file to be saved
    char *filename = malloc(sizeof(char) * 256);
    filename[255] = 0;
    if (read(0, filename, 255) == -1)
    {
        printf("[CLIENT] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // opening or creating the file
    int32_t filefd = open(filename, O_RDWR | O_CREAT, 0664);
    if (filefd == -1)
    {
        printf("[CLIENT] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // cloning the file
    char *buffer = malloc(4096 * sizeof(char));
    int32_t bytesread = 0, byteswritten = 0;
    while ((bytesread = read(socketdescriptor, buffer, 4096)) > 0)
    {
        if ((byteswritten = write(filefd, buffer, 4096)) == -1)
        {
            printf("[CLIENT] Line %d: Reason: %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    return EXIT_SUCCESS;
}