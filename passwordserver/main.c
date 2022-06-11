#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int32_t port = 60123;
	int32_t exit_server = 0;   // condition for server shutdown
	struct sockaddr_in server; // structura folosita de server
	struct sockaddr_in from;
	int32_t sd; // descriptorul de socket

	/* crearea unui socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("%s on line %d\n", strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}

	// reusing the address and port
	int32_t on = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	/* pregatirea structurilor de date */
	memset(&server, 0, sizeof(server));
	memset(&from, 0, sizeof(from));

	/* umplem structura folosita de server */
	/* stabilirea familiei de socket-uri */
	server.sin_family = AF_INET;
	/* acceptam orice adresa */
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	/* utilizam un port utilizator */
	server.sin_port = htons(port);

	/* atasam socketul */
	if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{
		printf("%s on line %d\n", strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}

	/* punem serverul sa asculte daca vin clienti sa se conecteze */
	if (listen(sd, 3) == -1)
	{
		printf("%s on line %d\n", strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}

	/* servim in mod concurent clientii... */
	while (!exit_server)
	{
		int client;
		socklen_t length = sizeof(from);

		printf("[SERVER] Listening on port %d\n", port);
		fflush(stdout);

		/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
		client = accept(sd, (struct sockaddr *)&from, &length);

		/* eroare la acceptarea conexiunii de la un client */
		if (client == -1)
		{
			printf("%s on line %d\n", strerror(errno), __LINE__);
			continue;
		}
		else // forking the execution, a child for every connection
		{
			int pid;
			if ((pid = fork()) == -1)
			{
				printf("%s on line %d\n", strerror(errno), __LINE__);
				close(client);
				continue;
			}
			else
			{
				if (pid != 0) // parent
				{
					close(client);
					while (waitpid(-1, NULL, WNOHANG))
						;
				}
				else // child
				{
					// closing the socket where we accepted the connection
					// because we no longer need it
					close(sd);

					printf("[SERVER] Connection established.\n");
					fflush(stdout);

					int32_t login = 0;               // login = 0 -> client not logged in
					// login = 1 -> client logged in
					int32_t exit_client = 0;
					char *username = NULL;
					char *password = NULL;

					while (!exit_client)
					{
						// enter a loop while the user logins
						while (!login)
						{
							char *input_username = "Input a username: ";
							int32_t input_username_length = strlen(input_username);

							// writing length to client
							if (write(client, &input_username_length, sizeof(input_username_length)) == -1)
							{
								printf("%s on line %d\n", strerror(errno), __LINE__);
								exit(EXIT_FAILURE);
							}

							// write message
							if (write(client, input_username, input_username_length) == -1)
							{
								printf("%s on line %d\n", strerror(errno), __LINE__);
								exit(EXIT_FAILURE);
							}

							// reading client's response
							int32_t username_length = 0;
							if (read(client, &username_length, sizeof(username_length)) == -1)
							{
								printf("%s on line %d\n", strerror(errno), __LINE__);
								exit(EXIT_FAILURE);
							}

							// malloc the username
							username = malloc(sizeof(char) * username_length);
							memset(username, 0, username_length);

							// reading the username
							if (read(client, username, username_length) == -1)
							{
								printf("%s on line %d\n", strerror(errno), __LINE__);
								exit(EXIT_FAILURE);
							}
							username[strlen(username) - 1] = '\0';

							fflush(stdout);
							printf("[SERVER] Username is %s\n", username);
							fflush(stdout);

							// the same procedure with the password
							char *input_password = "Input a password: ";
							int32_t input_password_length = strlen(input_password);

							// writing length to client
							if (write(client, &input_password_length, sizeof(input_password_length)) == -1)
							{
								printf("%s on line %d\n", strerror(errno), __LINE__);
								exit(EXIT_FAILURE);
							}

							// write message
							if (write(client, input_password, input_password_length) == -1)
							{
								printf("%s on line %d\n", strerror(errno), __LINE__);
								exit(EXIT_FAILURE);
							}

							// reading client's response
							int32_t password_length = 0;
							if (read(client, &password_length, sizeof(password_length)) == -1)
							{
								printf("%s on line %d\n", strerror(errno), __LINE__);
								exit(EXIT_FAILURE);
							}

							// malloc the password
							password = malloc(sizeof(char) * password_length);
							memset(password, 0, password_length);

							// reading the password
							int32_t bytes_read = 0;
							if ((bytes_read = read(client, password, password_length)) == -1)
							{
								printf("%s on line %d\n", strerror(errno), __LINE__);
								exit(EXIT_FAILURE);
							}
							password[strlen(password)- 1] = '\0';

							fflush(stdout);
							printf("[SERVER] Password is %s, bytes in: %d\n", password, bytes_read);
							fflush(stdout);

							if ((strcmp(username, "liviu") == 0) && (strcmp(password, "1234") == 0))
							{
								login = 1;
							}

							fflush(stdout);
							printf("strcmp cu liviu: %d, strcmp cu 1234: %d\n", strcmp(username, "liviu"), strcmp(password, "1234"));
							fflush(stdout);

							// free
							free(username);
							free(password);
						}
						printf("You made it.\n");
						break;
						fflush(stdout);

						/*if (read(client, msg, 100) <= 0)
						{
								perror("[server]Eroare la read() de la client.\n");
								close(client);
								exit(EXIT_SUCCESS);
						}
						else
						{
								printf("[server]Mesajul a fost receptionat...%s\n", msg);

								pregatim mesajul de raspuns
								bzero(msgrasp, 100);
								strcat(msgrasp, "Hello ");
								strcat(msgrasp, msg);

								printf("[server]Trimitem mesajul inapoi...%s\n", msgrasp);

								 returnam mesajul clientului
								if (write(client, msgrasp, 100) <= 0)
								{
										perror("[server]Eroare la write() catre client.\n");
										continue;
								}
						}*/
					}
					/* am terminat cu acest client, inchidem conexiunea */
					close(client);
					exit(EXIT_SUCCESS);
				}
			}
		}

	}
}