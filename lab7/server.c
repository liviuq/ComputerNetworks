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

// the port we ll be using
#define PORT 64000

int main(int argc, char **argv)
{
    struct sockaddr_in server; // struct used by the server
    struct sockaddr_in client;

    int32_t serversocket; // the socket we ll create to listen to
    if ((serversocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("[SERVER] Line %d: Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 0'ing sockaddr_in's
    memset(&server, 0, sizeof(struct sockaddr_in));
    memset(&client, 0, sizeof(struct sockaddr_in));

    // configuring the structure
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    //using SO_REUSEADDR
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

    // iterative approach to serve clients
    uint32_t order_number = 0; // position in line of a client
    while (1)
    {
        printf("[SERVER] Wainting for connection on port %d\n", PORT);
        fflush(stdout);

        // accepting the connection
        uint32_t clientlength = sizeof(client);
        int32_t clientsocket = accept(serversocket, (struct sockaddr *)&client, &clientlength);
        if (clientsocket == -1)
        {
            printf("[SERVER] Line %d: Reason: %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
        // successfully connected to the client.
        printf("[SERVER] Conenction successfull.\n");

        // waiting for an integer to return back to the client
        int32_t client_number = 0;
        if (read(clientsocket, &client_number, sizeof(client_number)) == -1)
        {
            printf("[SERVER] Line %d: Reason: %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        printf("[SERVER] Read %d.\n", client_number);
        // increment the position in line of the client
        order_number++;
        client_number++;

        // sending the incremented number and position in line of this client
        if (write(clientsocket, &order_number, sizeof(order_number)) == -1)
        {
            printf("[SERVER] Line %d: Reason: %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (write(clientsocket, &client_number, sizeof(client_number)) == -1)
        {
            printf("[SERVER] Line %d: Reason: %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("[SERVER] Written number %d and position %d\n", client_number, order_number);
        // cleaning up
        close(clientsocket);
    }

    return EXIT_SUCCESS;
}