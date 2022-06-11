#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("Usage: %s file\n", *argv);
        exit(EXIT_FAILURE);
    }

    struct stat status; //here we ll store the info about the file we re about to stat()
    if(stat(argv[1], &status) == -1)
    {
        printf("Error at stating. Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Successfilly statted.\n");
    }

    char* file_name; // buffer to store the name
    if((status.st_mode & S_IFMT) == S_IFDIR)
    {
        //we have a directory as input
       file_name = get_current_dir_name();
       printf("The name is %s\n", file_name);
       free(file_name);
    }

    char* time = malloc(sizeof(char) * 100); //check ret
    strftime(time, 100, "%Y-%m-\%d %H:%M:\%S", localtime(&status.st_mtim.tv_sec));
    printf("Last modification: %s\n", time);
    free(time);
    

    return EXIT_SUCCESS;
}