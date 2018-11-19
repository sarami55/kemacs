#include <stdio.h>
#include "Cstrings.h"

Char *
Crindex(c, cc)
   register Char *c, cc;
{
    register Char *c1 = c;

    if (!c || !cc) return NULL;
    c += Cstrlen(c);
    while (--c >= c1 && *c != cc) ;
    return c >= c1? c: NULL;
}
