#include <stdio.h>
#include "Cstrings.h"

Char *
Cstrncat(c1, c2, n)
    Char *c1, *c2;
    int n;
{
    Char *c3 = c1;

    if (!c1 || !c2) return NULL;
    c1 += Cstrlen(c1);
    (void)Cstrncpy(c1, c2, n);
    return c3;
}
