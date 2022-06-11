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
#include <utmpx.h>
#include <utmp.h>
#include <time.h>
#include <pwd.h>

int32_t check_login_bit(int32_t loginfd)
{
	int32_t is_logged_in = 0;
	int32_t br = read(loginfd, &is_logged_in, sizeof(is_logged_in));
	if (br == -1)
	{
		printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (lseek(loginfd, 0, SEEK_SET) == (off_t)-1)
	{
		printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if(is_logged_in == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

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
	static const char *users = "users";
	static const char *login = "login";

	// creating the fifo
	errno = 0;
	if (mkfifo(sercli, 0600) == -1)
	{
		if (errno != EEXIST) // if it already exists, do not end program, reuse fifo
		{
			printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	// creating the other fifo
	errno = 0;
	if (mkfifo(cliser, 0600) == -1)
	{
		if (errno != EEXIST) // if it already exists, do not end program, reuse fifo
		{
			printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	//creating the file that holds the login info:
	//0 -> user is not logged in
	//1 -> user is logged in
	int32_t loginfd = open(login, O_RDWR | O_TRUNC, 0600);
	if(loginfd == -1)
	{
		printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}
	//by default, the user is not logged in
	//so we ll write 0 in here
	int32_t zero_default = 0;
	int32_t default_write = write(loginfd, &zero_default, sizeof(zero_default));
	if(default_write == -1)
	{
		printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}
	//lseek to the beginnnign
	if(lseek(loginfd, 0, SEEK_SET) == (off_t) -1)
	{
		printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	//this is the file that contains the usernames
	int32_t usersfd = open(users, O_RDONLY);
	if( usersfd == -1)
	{
		printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
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
	ssize_t bytes_read;
	ssize_t bytes_written;
	int32_t message_length;
	char* buffer = NULL;

	while ((bytes_read = read(cliserfd, &message_length,sizeof(message_length)) ) > 0)
	{
		//reading the size of the message (the read call is above..comments are fun)
		printf("[SERVER] Read %ld bytes: %d\n", bytes_read, message_length);
		fflush(stdout);

		buffer = malloc(sizeof(char) * message_length);
		if (buffer == NULL)
		{
			printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}

		//now we read the whole message into buffer
		bytes_read = read(cliserfd, buffer, message_length);
		buffer[bytes_read] = '\0';
		printf("[SERVER] Received %ld bytes: %s\n----------\n", bytes_read, buffer);

		//if we received a login call, we use pipe.
		//if we receive any other call, we use socketpair
		if(strstr(buffer, "login : ") != NULL) //we received a login call
		{
			//opening a pipe for handling the commands
			//pipefd[0] -> read
			//pipefd[1] -> write
			int32_t pipefd[2];
			if( pipe(pipefd) == -1)
			{
				printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
				exit(EXIT_FAILURE);
			}

			pid_t login_fork = fork();
			if(login_fork == -1)
			{
				printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
				exit(EXIT_FAILURE);
			}
			else
			{
				if(login_fork == 0) //child process to execute login
				{
					if(check_login_bit(loginfd) == 1) //you are already logged in
					{
						char *reply = "You are already logged in.\n";
						char *temp = NULL;
						//pack_message(reply, temp);
						const int32_t reply_length = strlen(reply);
						temp = malloc( sizeof(char) * (sizeof(reply_length) + reply_length));
						memcpy(temp, &reply_length, sizeof(reply_length));
						memcpy( (temp + sizeof(reply_length)), reply, reply_length);


						bytes_written = write(pipefd[1], temp, sizeof(reply_length) + reply_length);
						if(bytes_written == -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
							exit(EXIT_FAILURE);
						}
						free(temp);
					}
					else //verify if the username exists in the file
					{
						FILE *fp;
						const char *search_string = (buffer + 8);
						fp = fopen(users, "r");
						char temp[512]; // Either char* buf or char buf[<your_buffer_size>]
						while(( fgets(temp, 512, fp) != NULL))
						{
							//null terminate the username from the users table
							temp[strlen(temp) - 1] = '\0';
							if(strcmp(search_string, temp) == 0)
							{
								const int32_t one = 1;
								write(loginfd, &one, sizeof(one));

								const char* reply = "Successfull Logged in.\n";
								const int32_t reply_length = strlen(reply);
								char* temp1 = malloc( sizeof(char) * (sizeof(reply_length) + reply_length));
								memcpy(temp1, &reply_length, sizeof(reply_length));
								memcpy( (temp1 + sizeof(reply_length)), reply, reply_length);

								bytes_written = write(pipefd[1], temp1, sizeof(reply_length) + reply_length);
								if(bytes_written == -1)
								{
									printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
									exit(EXIT_FAILURE);
								}
								free(temp1);
								if(lseek(loginfd, 0, SEEK_SET) == (off_t) -1)
								{
									printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));

									exit(EXIT_FAILURE);
								}

								break;
							}
						}

						const char* reply = "Cannot log in.\n";
						const int32_t reply_length = strlen(reply);
						char* temp2 = malloc( sizeof(char) * (sizeof(reply_length) + reply_length));
						memcpy(temp2, &reply_length, sizeof(reply_length));
						memcpy( (temp2 + sizeof(reply_length)), reply, reply_length);

						bytes_written = write(pipefd[1], temp2, sizeof(reply_length) + reply_length);
						if(bytes_written == -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
							exit(EXIT_FAILURE);
						}
						free(temp2);
						if(lseek(loginfd, 0, SEEK_SET) == (off_t) -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));

							exit(EXIT_FAILURE);
						}
						//closing fp
						fclose(fp);
					}

					//closing the pipe
					close(pipefd[0]);
					close(pipefd[1]);

					//returning from the child
					return 0;
				}

				//parent
				wait(NULL);

				//forward the message to the client
				int32_t reply_length;
				bytes_read = read(pipefd[0], &reply_length, sizeof(reply_length));
				if(bytes_read == -1)
				{
					printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
					exit(EXIT_FAILURE);
				}

				char* message = malloc( sizeof(char) * reply_length);
				bytes_read = read(pipefd[0], message, reply_length);
				if(bytes_read == -1)
				{
					printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
					exit(EXIT_FAILURE);
				}

				char* reply_to_client = malloc( sizeof(char) * (sizeof(reply_length) + reply_length));
				memcpy(reply_to_client, &reply_length, sizeof(reply_length));
				memcpy(( reply_to_client + sizeof(reply_length)), message, reply_length);
				//printf("[PARENT] I received %ld bytes: %s\n", bytes_read, message);

				bytes_written = write(serclifd, reply_to_client, sizeof(reply_length) + reply_length );
				printf("[SERVER] Written %ld bytes.\n-------------\n", bytes_written);
				fflush(stdout);

				//closing the flle descriptors
				close(pipefd[0]);
				close(pipefd[1]);
				free(message);
				free(reply_to_client);
			}
		}

		//logout
		else if(strstr(buffer, "logout") != NULL)
		{
			//logout is using a socketpair
			int32_t socketpairfd[2];

			//creating the socket pair
			if( socketpair(AF_LOCAL, SOCK_STREAM, 0, socketpairfd) == -1)
			{
				printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
				exit(EXIT_FAILURE);
			}

			//we have two pipe ends
			//0 -> parent
			//1 -> kiddo

			pid_t logout_fork = fork();
			if(logout_fork == -1)
			{
				printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
				exit(EXIT_FAILURE);
			}
			else
			{
				if(logout_fork == 0) //child
				{
					if(check_login_bit(loginfd) == 0) //logout while you are logged out
					{
						//tell the server that you are already logged out
						const char* already_logged_out = "You are logged out.\n";
						const int32_t length = strlen(already_logged_out);

						//creating the packed message
						char* message = malloc( sizeof(char) * (sizeof(length) + length));
						memcpy(message, &length, sizeof(length));
						memcpy( (message + sizeof(length)), already_logged_out, length);

						//sending the packed message to the server on socketpairfd[1]
						bytes_written = write(socketpairfd[1], message, sizeof(length) + length);
						if(bytes_written == -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
							exit(EXIT_FAILURE);
						}

						//freeing
						free(message);
					}
					else //you are logged in and want to log out
					{
						//seeking to the beginning of the file
						if(lseek(loginfd, 0, SEEK_SET) == (off_t) -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));

							exit(EXIT_FAILURE);
						}

						//changing the 4 bytes in loginfd
						bytes_written = write(loginfd, &zero_default, sizeof(zero_default));
						if(bytes_written == -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));

							exit(EXIT_FAILURE);
						}

						//seeking again
						if(lseek(loginfd, 0, SEEK_SET) == (off_t) -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));

							exit(EXIT_FAILURE);
						}

						//writing to the parent
						const char* successfully_logged_out = "Successfully logged out.\n";
						const int32_t length = strlen(successfully_logged_out);

						//creating the packed message
						char* message = malloc( sizeof(char) * (sizeof(length) + length));
						memcpy(message, &length, sizeof(length));
						memcpy( (message + sizeof(length)), successfully_logged_out, length);

						//sending the packed message to the server on socketpairfd[1]
						bytes_written = write(socketpairfd[1], message, sizeof(length) + length);
						if(bytes_written == -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
							exit(EXIT_FAILURE);
						}

						//freeing
						free(message);
					}

					//closing
					close(socketpairfd[0]);
					close(socketpairfd[1]);
					return 0;
				}
				else //parent
				{
					//waiting for the kid
					wait(NULL);

					//forward the message to the client
					int32_t reply_length;
					bytes_read = read(socketpairfd[0], &reply_length, sizeof(reply_length));
					if(bytes_read == -1)
					{
						printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
						exit(EXIT_FAILURE);
					}

					char* message = malloc( sizeof(char) * reply_length);
					bytes_read = read(socketpairfd[0], message, reply_length);
					if(bytes_read == -1)
					{
						printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
						exit(EXIT_FAILURE);
					}

					char* reply_to_client = malloc( sizeof(char) * (sizeof(reply_length) + reply_length));
					memcpy(reply_to_client, &reply_length, sizeof(reply_length));
					memcpy(( reply_to_client + sizeof(reply_length)), message, reply_length);
					//printf("[PARENT] I received %ld bytes: %s\n", bytes_read, message);

					bytes_written = write(serclifd, reply_to_client, sizeof(reply_length) + reply_length );
					printf("[SERVER] Written %ld bytes.\n-------------\n", bytes_written);
					fflush(stdout);

					//closing the flle descriptors
					close(socketpairfd[0]);
					close(socketpairfd[1]);
					free(message);
					free(reply_to_client);
				}
			}
		}

		else if(strcmp(buffer, "get-logged-users") == 0)
		{
			//get-logged-users is using a socketpair
			int32_t socketpairfd[2];

			//creating the socket pair
			if( socketpair(AF_LOCAL, SOCK_STREAM, 0, socketpairfd) == -1)
			{
				printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
				exit(EXIT_FAILURE);
			}

			//we have two pipe ends
			//0 -> parent
			//1 -> kiddo
			pid_t get_logged_users_fork = fork();
			if(get_logged_users_fork == -1)
			{
				printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
				exit(EXIT_FAILURE);
			}
			else
			{
				if(get_logged_users_fork == 0) //child
				{
					//verify if you are logged in
					if(check_login_bit(loginfd) == 0) //you are logged out
					{
						//tell the server that you are logged out
						const char* logged_out = "You are logged out.\n";
						const int32_t length = strlen(logged_out);

						//creating the packed message
						char* message = malloc( sizeof(char) * (sizeof(length) + length));
						memcpy(message, &length, sizeof(length));
						memcpy( (message + sizeof(length)), logged_out, length);

						//sending the packed message to the server on socketpairfd[1]
						bytes_written = write(socketpairfd[1], message, sizeof(length) + length);
						if(bytes_written == -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
							exit(EXIT_FAILURE);
						}

						//freeing
						free(message);
					}
					else //you are logged in
					{
						//finging the length of the date
						time_t now;
						time(&now);

						//defining the struct
						struct utmpx* entry = NULL;
						int32_t one_entry_size = (3 + UT_NAMESIZE + UT_HOSTSIZE + strlen(ctime(&now)));

						//here we ll hold all the information from the utmpx struct
						char* message = malloc(sizeof(char) * 2048);
						for(int32_t i = 0; i < 2048; i++)
							message[i] = 0;

						//documentation reccomends calling setutxtemt first
						setutxent();

						//iterating through all the fields of the /var/run/utmp file
						while ((entry = getutxent()) != NULL)
						{
							if(entry -> ut_type == USER_PROCESS)
							{
								//dinamically assigning space to message
								char* temporary = malloc ( sizeof(char) * one_entry_size);
								if(temporary == NULL)
								{
									printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
									exit(EXIT_FAILURE);
								}
								time_t my_time = entry->ut_tv.tv_sec;
								sprintf(temporary,"%s %s %s", entry->ut_user, entry->ut_host, ctime(&my_time));

								//concatenate to message
								strncat(message, temporary, strlen(temporary));
								free(temporary);
							}
						}
						//as per documentation
						endutxent();
						printf("%s\n", message);

						//writing to the parent
						const int32_t length = strlen(message);

						//creating the packed message
						char* get_logged_users_message = malloc( sizeof(char) * (sizeof(length) + length));
						memcpy(get_logged_users_message, &length, sizeof(length));
						memcpy( (get_logged_users_message + sizeof(length)), message, length);

						//sending the packed message to the server on socketpairfd[1]
						bytes_written = write(socketpairfd[1], get_logged_users_message, sizeof(length) + length);
						if(bytes_written == -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
							exit(EXIT_FAILURE);
						}

						//freeing
						free(message);
						free(get_logged_users_message);
						close(socketpairfd[0]);
						close(socketpairfd[1]);
					}

					return 0;
				}
				else //server
				{
					wait(NULL);

					//reading from the child
					int32_t reply_length;
					bytes_read = read(socketpairfd[0], &reply_length, sizeof(reply_length));
					if(bytes_read == -1)
					{
						printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
						exit(EXIT_FAILURE);
					}

					char* message = malloc( sizeof(char) * reply_length);
					bytes_read = read(socketpairfd[0], message, reply_length);
					if(bytes_read == -1)
					{
						printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
						exit(EXIT_FAILURE);
					}

					char* reply_to_client = malloc( sizeof(char) * (sizeof(reply_length) + reply_length));
					memcpy(reply_to_client, &reply_length, sizeof(reply_length));
					memcpy(( reply_to_client + sizeof(reply_length)), message, reply_length);
					//printf("[PARENT] I received %ld bytes: %s\n", bytes_read, message);

					bytes_written = write(serclifd, reply_to_client, sizeof(reply_length) + reply_length );
					printf("[SERVER] Written %ld bytes.\n-------------\n", bytes_written);
					fflush(stdout);

					//closing the flle descriptors
					close(socketpairfd[0]);
					close(socketpairfd[1]);
					free(message);
					free(reply_to_client);
				}
			}

		}

		else if(strstr(buffer, "get-proc-info : ") != NULL)
		{
			//get-proc-info is using a socketpair
			int32_t socketpairfd[2];

			//creating the socket pair
			if( socketpair(AF_LOCAL, SOCK_STREAM, 0, socketpairfd) == -1)
			{
				printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
				exit(EXIT_FAILURE);
			}

			//we have two pipe ends
			//0 -> parent
			//1 -> kiddo

			int32_t get_proc_info_fork = fork();
			if(get_proc_info_fork == -1)
			{
				printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
				exit(EXIT_FAILURE);
			}
			else
			{
				if(get_proc_info_fork == 0)//child
				{
					if(check_login_bit(loginfd) == 0) //no log in
					{
						//tell the server that you are logged out
						const char* logged_out = "You are logged out.\n";
						const int32_t length = strlen(logged_out);

						//creating the packed message
						char* message = malloc( sizeof(char) * (sizeof(length) + length));
						memcpy(message, &length, sizeof(length));
						memcpy( (message + sizeof(length)), logged_out, length);

						//sending the packed message to the server on socketpairfd[1]
						bytes_written = write(socketpairfd[1], message, sizeof(length) + length);
						if(bytes_written == -1)
						{
							printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
							exit(EXIT_FAILURE);
						}

						//freeing
						free(message);
					}
					else //we are logged in
					{
						const char* number_string = buffer + 16;
						const int32_t number_decimal = atoi(number_string);

						//creating the path
						char path_to_proc[30] = "";
						strcat(path_to_proc, "/proc/");
						strcat(path_to_proc, number_string);
						strcat(path_to_proc, "/status");
						path_to_proc[strlen(path_to_proc)] = '\0';

						//verify if the folder exists
						FILE* proc_status = fopen(path_to_proc, "r");
						if( proc_status == NULL)
						{
							//this path to proc does not exist. write accordingly
							const char* logged_out = "This pid does not exist in the folder.\n";
							const int32_t length = strlen(logged_out);

							//creating the packed message
							char* message = malloc( sizeof(char) * (sizeof(length) + length));
							memcpy(message, &length, sizeof(length));
							memcpy( (message + sizeof(length)), logged_out, length);

							//sending the packed message to the server on socketpairfd[1]
							bytes_written = write(socketpairfd[1], message, sizeof(length) + length);
							if(bytes_written == -1)
							{
								printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
								exit(EXIT_FAILURE);
							}

							//freeing
							free(message);
						}
						else //the file exists
						{
							char string[2048] = "";
							char temp[100];
							while(( fgets(temp, 100, proc_status) != NULL))
							{
								//(name, state, ppid, uid, vmsize)
								if(strstr(temp, "Name") != NULL)
								{
									strncat(string, temp, strlen(temp));
								}
								else if(strstr(temp, "State") != NULL)
								{
									strncat(string, temp, strlen(temp));
								}
								else if(strstr(temp, "PPid") != NULL)
								{
									strncat(string, temp, strlen(temp));
								}
								else if(strstr(temp, "Uid") != NULL)
								{
									strncat(string, temp, strlen(temp));
								}
								else if(strstr(temp, "VmSize") != NULL)
								{
									strncat(string, temp, strlen(temp));
								}
							}

							//we have the information, now we just make the message
							int32_t get_proc_info_length = strlen(string);
							char* get_proc_info_message = malloc( sizeof(char) * ( sizeof(get_proc_info_length) + get_proc_info_length ) );
							memcpy(get_proc_info_message, &get_proc_info_length, sizeof(get_proc_info_length));
							memcpy((get_proc_info_message + sizeof(get_proc_info_length)), string, get_proc_info_length);

							//sending the packed message to the server on socketpairfd[1]
							bytes_written = write(socketpairfd[1], get_proc_info_message, sizeof(get_proc_info_length) + get_proc_info_length);
							if(bytes_written == -1)
							{
								printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
								exit(EXIT_FAILURE);
							}

							free(get_proc_info_message);
						}
					}

					close(socketpairfd[0]);
					close(socketpairfd[1]);

					return 0;
				}
				else //parent
				{
					wait(NULL);

					//reading from the child
					int32_t reply_length;
					bytes_read = read(socketpairfd[0], &reply_length, sizeof(reply_length));
					if(bytes_read == -1)
					{
						printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
						exit(EXIT_FAILURE);
					}

					char* message = malloc( sizeof(char) * reply_length);
					bytes_read = read(socketpairfd[0], message, reply_length);
					if(bytes_read == -1)
					{
						printf("Error on line %d. Reason: %s\n", __LINE__, strerror(errno));
						exit(EXIT_FAILURE);
					}

					char* reply_to_client = malloc( sizeof(char) * (sizeof(reply_length) + reply_length));
					memcpy(reply_to_client, &reply_length, sizeof(reply_length));
					memcpy(( reply_to_client + sizeof(reply_length)), message, reply_length);
					//printf("[PARENT] I received %ld bytes: %s\n", bytes_read, message);

					bytes_written = write(serclifd, reply_to_client, sizeof(reply_length) + reply_length );
					printf("[SERVER] Written %ld bytes.\n-------------\n", bytes_written);
					fflush(stdout);
				}
			}
		}

		else if(strcmp(buffer, "quit") == 0)
		{
			close(serclifd);
			close(cliserfd);
			close(loginfd);
			close(usersfd);
			free(buffer);
			exit(EXIT_SUCCESS);
		}
		else
		{
			const char* invalid_message = "Invalid command\n";
			int32_t buffer_length = strlen(invalid_message);
			printf("[SERVER] Buffer has %d length.\n", buffer_length);
			fflush(stdout);

			char* message = malloc( sizeof(char) * (sizeof(buffer_length) + buffer_length) );
			memcpy(message, &buffer_length, sizeof(buffer_length));
			memcpy((message + sizeof(buffer_length)), invalid_message, buffer_length);

			printf("[SERVER] The message contains %ld in size and the content is %s\n", sizeof(buffer_length), message + 4);
			fflush(stdout);

			bytes_written = write(serclifd, message, sizeof(buffer_length) + buffer_length );
			printf("[SERVER] Written %ld bytes.\n-------------\n", bytes_written);
			fflush(stdout);
		}
	}

	// closign
	free(buffer);
	close(cliserfd);
	close(serclifd);
	close(loginfd);
	close(usersfd);


	return EXIT_SUCCESS;
}
