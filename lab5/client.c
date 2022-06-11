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
		if (errno != EEXIST) // if it already exists, do not end program, reuse fifo
		{
			printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	// creating the fifo
	if (mkfifo(sercli, 0600) == -1)
	{
		if (errno != EEXIST) // if it already exists, do not end program, reuse fifo
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
	if (cliserfd == -1)
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

	ssize_t bytes_read;
	ssize_t bytes_written;
	do
	{
		printf("[CLIENT] Input the command: ");
		fflush(stdout);

		//reading client's command
		bytes_read = read(STDIN_FILENO, buffer, 100);
		buffer[bytes_read - 1] = '\0';
		printf("[CLIENT] Got from stdin %ld bytes: %s\n", bytes_read, buffer);
		fflush(stdout);

		//preparing the command to be sent to the server
		int32_t buffer_length = strlen(buffer);
		printf("[CLIENT] Buffer has %d length.\n", buffer_length);
		fflush(stdout);

		char* message = malloc( sizeof(char) * (sizeof(buffer_length) + buffer_length) );
		memcpy(message, &buffer_length, sizeof(buffer_length));
		memcpy((message + sizeof(buffer_length)), buffer, buffer_length);

		printf("[CLIENT] The message contains %ld in size and the content is %s\n", sizeof(buffer_length), message + 4);
		fflush(stdout);

		bytes_written = write(cliserfd, message, sizeof(buffer_length) + buffer_length);
		printf("[CLIENT] Written %ld bytes\n", bytes_written);
		fflush(stdout);

		// length of the message
		/*int32_t message_length;
		char digits;
		bytes_read = read(serclifd, &digits, sizeof(digits));
		int32_t digits_decimal = atoi(&digits);
		printf("Length has %d digits.\n", digits_decimal);
		fflush(stdout);

		char temp[digits_decimal];
		bytes_read = read(serclifd, &temp, digits_decimal);
		message_length = atoi(temp);
		printf("[CLIENT] Received %d bytes. The message_length is %d\n", bytes_read, message_length);
		fflush(stdout);

		char *message_buffer = malloc(sizeof(char) * message_length + 1);
		bytes_read = read(serclifd, message_buffer, message_length);
		message_buffer[bytes_read] = '\0';
		printf("[CLIENT] Received %d bytes: %s\n-----------\n", bytes_read, message_buffer);
		free(message_buffer);
		fflush(stdout);*/

		int32_t message_length;
		bytes_read = read(serclifd, &message_length, sizeof(message_length));
		printf("[CLIENT] Read %ld bytes: %d\n", bytes_read, message_length);
		fflush(stdout);

		char* buffer = malloc( sizeof(char) * message_length);
		bytes_read = read(serclifd, buffer, message_length);
		buffer[bytes_read] = '\0';
		printf("[CLIENT] Received %ld bytes: %s\n-----------\n", bytes_read, buffer);
		free(buffer);
		fflush(stdout);

	} while ( bytes_read > 0);

	// closing
	free(buffer);
	close(cliserfd);
	close(serclifd);

	return EXIT_SUCCESS;
}
