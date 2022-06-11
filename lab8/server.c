/*Implementati un server TCP concurent care sa
permita descarcarea unui fisier in paralel de
catre mai multi clienti.*/

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
#include <pthread.h>

typedef struct
{
    int32_t id;
    int32_t clientfd;
    int32_t is_used;
} Thread;

void* handle_connection(void *argument)
{
    Thread *thread = (Thread *)argument;

}

#define PORT 64000

int main(int argc, char **argv)
{
    int32_t threadnumber = 5;
    if (argc != 2)
    {
        printf("[CLIENT] Usage: %s <file_to_share>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // checking the file
    if (access(argv[1], F_OK) == -1)
    {
        if (errno != EACCES)
        {
            printf("[CLIENT] Usage: %s <file_to_share>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // creating the socket
    int32_t serversocket;
    if ((serversocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("[CLIENT] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // struct used by the client
    struct sockaddr_in server;
    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    // using SO_REUSEADDR
    int32_t on = 1;
    setsockopt(serversocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // binding the socket to the IP provided
    if (bind(serversocket, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        printf("[SERVER] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Listening on port %d\n", PORT);

    // listenning for incoming connections
    if (listen(serversocket, 5) == -1)
    {
        printf("[SERVER] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // the thread pool
    Thread *threadPool = malloc(threadnumber * sizeof(Thread));

    // mutex initialiser
    pthread_mutex_t threadlock = PTHREAD_MUTEX_INITIALIZER;

    for(int i = 0; i < threadnumber; i++)
    {
    }
    return EXIT_SUCCESS;
}