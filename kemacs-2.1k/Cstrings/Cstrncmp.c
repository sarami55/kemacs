#include "Cstrings.h"

int
Cstrncmp(c1, c2, n)
    register Char *c1, *c2;
    register int n;
{
    if (c1 == c2) return 0;
    if (!c1) return -1;
    if (!c2) return 1;
    while (n > 0 && *c1 && *c1 == *c2) c1++, c2++, n--;
    return !n? 0: (int)(*c1-*c2);
}
