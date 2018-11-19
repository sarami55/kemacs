#include "Cstrings.h"

int
Cstrlen(c)
   register Char *c;
{
    register int n;

    for (n = 0; *c++; n++) ;
    return n;
}
