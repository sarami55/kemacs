#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
#include "econfig.h"
#include <stdio.h>

/* string library header has different name */
#if BSD_LEGACY
# include <strings.h>
#else
# include <string.h>
#endif

/* names of character/byte functions */
#if BSD_LEGACY
# define INDEX		index
# define RINDEX		rindex
# define memcmp		bcmp
# define memcpy(s, d, n) bcopy((d), (s), (n)) /* but bcopy() is void */
#else
# define INDEX		strchr
# define RINDEX		strrchr
#endif

/* definitions of KANJI related routines */
#if KANJI
# include "kanji.h"
# include "Cstrings.h"

Char *safeC();
char *safec();

# define SafeCfromc(s)	safeC(Cfromc(s)) /* Copy Cstring to safe place */
# define SafecfromC(s)	safec(cfromC(s)) /* Copy string to safe place */
# define Free(s)	free((char*)(s))

#else /*!KANJI*/

# define Char		char

# define Cstrcat	strcat
# define Cstrncat	strncat
# define Cstrcmp	strcmp
# define Cstrncmp	strncmp
# define Cstrcpy	strcpy
# define Cstrncpy	strncpy
# define Cstrlen	strlen
# define Cindex		INDEX
# define Crindex	RINDEX
# define Cfromc(x)	(x)
# define cfromC(x)	(x)

# define SafeCfromc(s)	(s)	/* no need to save */
# define SafecfromC(s)	(s)
# define Free(s)	/**/

#endif /*!KANJI*/

/* ignore marker recognized by 'Cstr' program. */
# define CSTR(x)	(x)
# define CCHR(x)	(x)
