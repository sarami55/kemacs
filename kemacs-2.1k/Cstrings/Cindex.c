#include <stdio.h>
#include "Cstrings.h"

Char *
Cindex(c, cc)
    register Char *c, cc;
{
    if (!c || !cc) return NULL;
    while (*c && *c != cc) c++;
    return *c? c: NULL;
}
