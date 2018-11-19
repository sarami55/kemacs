#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*
 * Version related routines.
 */
#include	"ecomm.h"
#include	"estruct.h"
#include	"edef.h"
#include	"eversion.h"

/*
 * Returns version string.
 */
Char *
version()
{
	return(VERSION);
}

/*
 * Returns program name.
 */
Char *
prgname()
{
	return
#if KMSGS
	kterminal? KPRGNAME:
#endif
	PRGNAME;
}

/*
 * Returns last revised date.
 */
Char *
revdate()
{
	return(REVDATE);
}
