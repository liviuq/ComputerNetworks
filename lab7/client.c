/*Exercitii - modelul Client/Server TCP/IP iterativ
1.  Scrieti un server TCP care primeste de la client
un numar. Acesta va intoarce numarul incrementat
si numarul de ordine a clientului respectiv.
2. Consideram un fisier cu o configuratie de tipul
cheie:comanda.
Exemplu de fisier:
1111:/bin/ls:
2222:/bin/pwd:
3333:/bin/ps:
Sa se scrie un server TCP iterativ care functioneaza
astfel:
atunci cand un client se conecteaza la server
si ii trimite una din key-urile aflate in fisier,
atunci serverul va executa comanda asociata
si va intoarce clientului rezultatul acesteia;
pentru orice alta cheie, serverul va trimite
raspunsul ”Unknown key” clientului si va
inchide conexiunea cu acesta.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    // usage
    if (argc != 4)
    {
        printf("Usage: %s <ip> <port> <number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // client socket descriptor
    int32_t clientsocket;
    if ((clientsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
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
    if (connect(clientsocket, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        printf("[CLIENT] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // reading the number
    int32_t number = atoi(argv[3]); 
    printf("The number is: %d\n", number);

    // writing the number to the server
    if (write(clientsocket, &number, sizeof(number)) == -1)
    {
        printf("[CLIENT] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // reading the two numbers returned by the server
    int32_t numberinc, position;
    if ( read(clientsocket, &position, sizeof(position)) == -1)
    {
        printf("[CLIENT] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if( read(clientsocket, &numberinc, sizeof(numberinc)) == -1)
    {
        printf("[CLIENT] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    //printing the numbers
    printf("The number incremented is: %d and the position in line is: %d\n", numberinc, position);
    return EXIT_SUCCESS;
}