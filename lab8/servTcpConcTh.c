/* servTCPConcTh.c - Exemplu de server TCP concurent folosind thread-uri
   Asteapta un nr de la clienti; intoarce clientului numarul incrementat
   !! Are o eroare la Nr. threadului...de ce?

   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

void* treat(void *arg); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(int);
int i = 0; // contor pentru evidenta pthread_t

int main()
{
  struct sockaddr_in server; // structura folosita de server
  struct sockaddr_in from;
  int sd; // descriptorul de socket
  pthread_t th[100]; // Identificatorii thread-urilor care se vor crea

  /* crearea unui socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }
  /* utilizarea optiunii SO_REUSEADDR */
  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  /* pregatirea structurilor de date */
  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
  {
    int client;
    socklen_t length = sizeof(from);

    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }

    /* s-a realizat conexiunea, se astepta mesajul */
    i++;
    // cream thread-ul
    pthread_create(&th[i], NULL, &treat, (void *)(intptr_t)client);

    //??? Ce se intimpla daca apelam close(client);????
    //fiindca folosim pthread, nu se copiaza address space ul ca la fork. asadar, daca
    //facem close(client) vom inchide conexiunea cu client in acest thread dar si in toate 
    //celelalte threaduri si in main.
  } // while
}

void *treat(void *arg)
{
  printf("[thread]- %d - Asteptam mesajul...\n", i);
  fflush(stdout);

  pthread_detach(pthread_self());
  raspunde((intptr_t)arg);
  /* am terminat cu acest client, inchidem conexiunea */
  close((intptr_t)arg);
  return (NULL);
};

void raspunde(int cl)
{
  int nr;
  if (read(cl, &nr, sizeof(int)) <= 0)
  {
    printf("[Thread %d]\n", i);
    perror("Eroare la read() de la client.\n");
  }

  printf("[Thread %d]Mesajul a fost receptionat...%d\n", i, nr);

  /*pregatim mesajul de raspuns */
  nr++;
  printf("[Thread %d]Trimitem mesajul inapoi...%d\n", i, nr);

  /* returnam mesajul clientului */
  if (write(cl, &nr, sizeof(int)) <= 0)
  {
    printf("[Thread %d] ", i);
    perror("[Thread]Eroare la write() catre client.\n");
    // continue;		/* continuam sa ascultam */
  }
  else
    printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", i);
}
