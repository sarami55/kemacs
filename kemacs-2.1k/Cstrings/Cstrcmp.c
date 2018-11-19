#include "Cstrings.h"

int
Cstrcmp(c1, c2)
    register Char *c1, *c2;
{
    if (c1 == c2) return 0;
    if (!c1) return -1;
    if (!c2) return 1;
    while (*c1 && *c1 == *c2) c1++, c2++;
    return (int)(*c1-*c2);
}
