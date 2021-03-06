                     

1. Scrieti un program in care se foloseste un singur pipe pentru
comunicare si in care:
tatal scrie intr-un pipe un numar;
fiul verifica daca numarul este prim 
si transmite prin pipe tatalui raspunsul (yes, no);
tatal va afisa raspunsul primit.
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

int8_t is_prime(int32_t number)
{
    if (number <= 1)
    {
        return 0;
    }

    if (number <= 3)
    {
        return 1;
    }

    if (number % 2 == 0 || number % 3 == 0)
    {
        return 0;
    }

    for (uint32_t i = 5; i * i <= number; i = i + 6)
    {
        if (number % i == 0 || number % (i + 2) == 0)
        {
            return 0;
        }
    }

    return 1;
