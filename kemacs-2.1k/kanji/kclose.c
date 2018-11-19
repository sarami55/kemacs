#include "kanji.h"

void
kclose(kp)
    register KFILE *kp;
{
	(void)kfree((KSTREAM *)kp);
}
