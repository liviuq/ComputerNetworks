#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char** argv)
{
    if(argc != 1)
    {
        printf("Usage: %s\n", *argv);
        exit(EXIT_FAILURE);
    }

    struct passwd* information; //get password file entry
    errno = 0; //man getpwuid() -> RETURN VALUE
    if((information = getpwuid(getuid())) == NULL)
    {
        printf("Error at getting the info from /etc/passwd. REason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Name: %s\n", information->pw_name);
    printf("Shell: %s\n", information->pw_shell);

    return EXIT_SUCCESS;
}