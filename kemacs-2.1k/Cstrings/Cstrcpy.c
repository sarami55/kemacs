#include <stdio.h>
#include "Cstrings.h"

Char *
Cstrcpy(c1, c2)
    register Char *c1, *c2;
{
    Char *c3 = c1;

    if (!c1 || !c2) return NULL;
    while (*c1++ = *c2++) ;
    return c3;
}
