#include <stdio.h>

#include "verify.h"

int is_char(char current_char)
{
    return ((current_char >= 65 && current_char <= 90) || (current_char >= 97 && current_char <= 122));
}