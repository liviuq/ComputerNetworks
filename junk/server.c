/*
Dezvoltati doua aplicatii (denumite "client" si "server") ce comunica
intre ele pe baza unui protocol care are urmatoarele specificatii:

- comunicarea se face prin executia de comenzi citite de la tastatura in client si
    executate in procesele copil create de server;
- comenzile sunt siruri de caractere delimitate de new line;
- raspunsurile sunt siruri de octeti prefixate de lungimea raspunsului;
- rezultatul obtinut in urma executiei oricarei comenzi va fi afisat de client;
- procesele copil din server nu comunica direct cu clientul, ci doar cu procesul parinte;
- protocolul minimal cuprinde comenzile:
      - "login : username" - a carei existenta este validata prin utilizarea unui fisier de
            configurare, care contine toti utilizatorii care au acces la functionalitati.
            Executia comenzii va fi realizata intr-un proces copil din server;
      - "get-logged-users" - afiseaza informatii (username, hostname for remote login,
            time entry was made) despre utilizatorii autentificati pe sistemul de operare
            (vezi "man 5 utmp" si "man 3 getutent"). Aceasta comanda nu va putea fi executata
            daca utilizatorul nu este autentificat in aplicatie. Executia comenzii va fi
            realizata intr-un proces copil din server;
      - "get-proc-info : pid" - afiseaza informatii
            (name, state, ppid, uid, vmsize) despre procesul indicat
            (sursa informatii: fisierul /proc/<pid>/status).
            Aceasta comanda nu va putea fi executata daca utilizatorul nu este
            autentificat in aplicatie. Executia comenzii va fi realizata intr-un proces
            copil din server;
      - "logout";
      - "quit".
- in implementarea comenzilor nu se va utiliza nicio functie din familia "exec()"
    (sau alta similara, de ex. popen(), system()...) in vederea executiei
    unor comenzi de sistem ce ofera functionalitatile respective;
- comunicarea intre procese se va face folosind cel putin o
    data fiecare din urmatoarele mecanisme ce permit comunicarea:
    pipe-uri, fifo-uri si socketpair.

Observatii:
- termen de predare: laboratorul din saptamana 5;
- orice incercare de frauda, in functie de gravitate, va conduce la propunerea pentru exmatriculare a studentului in cauza sau la punctaj negativ.
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
    if (mkfifo(sercli, 0600) == -1)
    {
        if (errno != EEXIST) // if it already exist, do not end program, reuse fifo
        {
            printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // creating the other fifo
    if (mkfifo(cliser, 0600) == -1)
    {
        if (errno != EEXIST) // if it already exist, do not end program, reuse fifo
        {
            printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // opening the fifo for server->client comms
    int32_t serclifd = open(sercli, O_WRONLY);
    if (serclifd == -1)
    {
        printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // opening the fifo for client->server comms
    int32_t cliserfd = open(cliser, O_RDONLY);
    if (cliserfd == -1)
    {
        printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // writing to the fifo
    char *buffer = malloc(sizeof(char) * 256);
    if (buffer == NULL)
    {
        printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int32_t bytes_read;
    int32_t size_of_message;
    int32_t bytes_written;

    while ((bytes_read = read(cliserfd, buffer, 30)) > 0)
    {
        //if you input more than 15 bytes, then bytes_read will be 0 or -1
        //and in buffer you ll have a \0 first
        buffer[bytes_read] = '\0';
        printf("[SERVER] Received %d bytes: %s\n", bytes_read, buffer);
        fflush(stdout);
        if (strcmp(buffer, "logout") == 0)
        {
            // closing the server->client comms
            close(serclifd);
        }
        else
        {
            printf("[SERVER] Input a message: ");
            fflush(stdout);
            bytes_read = read(STDIN_FILENO, buffer, 256);
            buffer[bytes_read] = '\0';
            printf("[SERVER] Read %d bytes: %s\n",bytes_read, buffer);

            //getting buffer s length
            size_of_message = strlen(buffer);
            char *message = malloc(sizeof(size_of_message) + size_of_message);
            //snprintf(message, sizeof(size_of_message) + size_of_message, "%d%s", size_of_message, buffer);
            memcpy(message, &size_of_message, sizeof(size_of_message));
            memcpy(&message[size_of_message], buffer, size_of_message);
            printf("[SERVER] The message looks like %s\n", message);
            fflush(stdout);

            bytes_written = write(serclifd, message, strlen(message));
            printf("[SERVER] Written %d bytes.\n-------------\n", bytes_written);
            fflush(stdout);
        }
    }
    // closign
    free(buffer);
    close(cliserfd);

    return EXIT_SUCCESS;
}