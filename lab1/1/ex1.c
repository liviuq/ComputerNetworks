#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "verify.h"

int main(int argc, char** argv)
{
    //checking argc
    if(argc != 3)
    {
        printf("Usage: %s input output\n", *argv);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Parsed input parameters\n");
    }

    //opening the files
    int input_file = open(argv[1], O_RDONLY);
    if(input_file == -1)
    {
        printf("Error at opening the input file. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Opened the input file %s\n", *argv);
    }

    int output_file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
    if(output_file == -1)
    {
        printf("Error at opening the output file. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Opened the output file %s\n", *argv);
    }

    //allocating space for two strings, aka 2 * 256 bytes
    char* buffer = malloc(sizeof(char) * 2 * 256);
    if(buffer == NULL)
    {
        printf("Error at allocating memory for buffer. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //reading the contents of input_file into buffer
    if(read(input_file, buffer, 2 * 256) == -1)
    {
        printf("Error at reading the input file. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("The line is '%s'\n", buffer);
    }

    //concatenating the two strings
    char* concatenated_string = malloc(sizeof(char) * 256 * 2); //check malloc ret

    char* current_char = buffer; //pointer to the first valid character
    int word_count = 0; //stop concatenating after 2 words

    while (*current_char != '\0' && word_count != 2) 
    {
        if(is_char(*current_char))
        {
            char* string = current_char;
            char* end_string = current_char;
            while(is_char(*end_string)) //incrementing the end_string untill we stumble upon a non alphabet character
            {
                end_string++;
            }
            strncat(concatenated_string, string, (end_string - string)); //append to concatenated_string the first (end_string - string) bytes starting from string pointer
            current_char = end_string;
            word_count++;
        }

        current_char++;
    }
    
    //printing hello world
    printf("Hello world!\n");

    //writing the concatenated string to the output
    int bytes;
    if( (bytes = write(output_file, concatenated_string, strlen(concatenated_string))) == -1)
    {
        printf("Error writing to output file. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Successfully written %d bytes to the output file.\n", bytes);
    }

    //freeing the memory and closing the files
    free(buffer);
    free(concatenated_string);
    close(input_file);
    close(output_file);

    return EXIT_SUCCESS;
}
