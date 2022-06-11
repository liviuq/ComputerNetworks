#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOCAL_MESSAGE_LEN 256

int main(int argc, char *argv[])
{
	int32_t exit_client = 0;
	int32_t port;
	int32_t sd;                                // descriptorul de socket
	struct sockaddr_in server; // structura folosita pentru conectare

	/* exista toate argumentele in linia de comanda? */
	if (argc != 3)
	{
		printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
		return -1;
	}

	/* stabilim portul */
	port = atoi(argv[2]);

	/* cream socketul */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("%s on line %d\n", strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}

	/* umplem structura folosita pentru realizarea conexiunii cu serverul */
	/* familia socket-ului */
	server.sin_family = AF_INET;
	/* adresa IP a serverului */
	server.sin_addr.s_addr = inet_addr(argv[1]);
	/* portul de conectare */
	server.sin_port = htons(port);

	/* ne conectam la server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{
		printf("%s on line %d\n", strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}

	while (!exit_client)
	{
		// reading the number of bytes from the server
		int32_t length_in = 0;
		if (read(sd, &length_in, sizeof(length_in)) == -1)
		{
			printf("%s on line %d\n", strerror(errno), __LINE__);
			exit(EXIT_FAILURE);
		}
		printf("[CLIENT] Length of message is %d\n", length_in);

		// reading the whole message
		char *incoming_message = malloc(sizeof(char) * length_in);
		memset(incoming_message, 0, length_in);

		if (read(sd, incoming_message, length_in) == -1)
		{
			printf("%s on line %d\n", strerror(errno), __LINE__);
			exit(EXIT_FAILURE);
		}

		// displaying the message
		printf("%s\n", incoming_message);

		// reading the input from the user
		char *message = malloc(sizeof(char) * LOCAL_MESSAGE_LEN);
		memset(message, 0, LOCAL_MESSAGE_LEN);
		if (read(STDIN_FILENO, message, LOCAL_MESSAGE_LEN) == -1)
		{
			printf("%s on line %d\n", strerror(errno), __LINE__);
			exit(EXIT_FAILURE);
		}

		// writing to the server the response's length
		int32_t length_out = strlen(message);
		if (write(sd, &length_out, sizeof(length_out)) == -1)
		{
			printf("%s on line %d\n", strerror(errno), __LINE__);
			exit(EXIT_FAILURE);
		}

		// writing to the server the whole response
		if (write(sd, message, length_out) == -1)
		{
			printf("%s on line %d\n", strerror(errno), __LINE__);
			exit(EXIT_FAILURE);
		}

		// freeing the memory
		free(incoming_message);
		free(message);
	}

	close(sd);
	return 0;
}