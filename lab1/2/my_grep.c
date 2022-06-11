#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        printf("Usage: %s input_file word\n", *argv);
        exit(EXIT_FAILURE);
    }

    FILE* input = fopen(argv[1], "r"); //opening the file in read only mode
    if(input == NULL)
    {
        printf("Error at openning. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Successfully opened input file.\n");
    }

    int line_size = 256; //maximum number of characters per line
    char* buffer = malloc(sizeof(char) * line_size); //should check the ret val
    while(!feof(input)) //while we still have characters in the input file
    {
        if((buffer = fgets(buffer, line_size, input)) == NULL) //store a line in buffer
        {
            printf("EOF or error. Reason: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if(strstr(buffer, argv[2]) == NULL) //check if there are any matches of word in buffer
        {
            continue;
        }
        else
        {
            printf("%s\n", buffer);
        }
    }

    return EXIT_SUCCESS;
}