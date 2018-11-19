#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
#include "ecomm.h"
#include "estruct.h"
#include "edef.h"

static Char *dummy = CSTR(&Cxstr[6]);

/* get/put data with file stream */
/*
 * there are two set of streams. One for terminal, another for file.
 * file stream is initialized each time it is opened.
 */

#if KANJI

# ifndef ESC_PERIOD

/* default of maximum period between ESC sequence characters */
#  define ESC_PERIOD	200	/* millisecond */

# endif

/* Only one file stream can be opened simultaneously */
static KFILE *kfp;

/*
 * open KANJI stream for file I/O.
 * On reading, specified code is used as the default code of stream.
 * On writing, specified code is used.
 */
kfopen(ffp, code)
     FILE *ffp;
     unsigned code;
{
  kfp = kopen(ffp, code, 0);
}

/*
 * close KANJI stream for file I/O.
 * On reading, code is set according to stream data.
 * On writing, code is not used anywhere.
 */
unsigned
kfclose()
{
  unsigned code;

  code = KS_VALUE(kfp->ks_flag);
  kclose(kfp);
  return code;
}

/*ARGSUSED*/
Char
kfgetc(fp)
    FILE *fp;
{
    return kgetc(kfp);
}

/*ARGSUSED*/
kfputc(c, fp)
    Char c;
    FILE *fp;
{
    kputc(c, kfp);
}

static KFILE *kip, *kop;
/*
 * terminal I/O.
 */
kkopen()
{
  KS_FLAG icode
#if	STRASSOK
	 = term.t_code;
#else
	;
#endif

#if	!STRASSOK
  (void)memcpy((char *)&icode, (char *)&term.t_code, sizeof(icode));
#endif	/*!STRASSOK*/
  if (KS_INTERP(icode) != KS_BINARY && KS_CODE(icode) == KS_JIS) {
    /* pass ^O/^N for input */
    KS_PASS(icode) = 1;
  }
  kip = kopen(stdin, KS_VALUE(icode), ESC_PERIOD);
  kop = kopen(stdout, KS_VALUE(term.t_code), 0);
}

kkclose()
{
  kclose(kip);
  kclose(kop);
}

/*ARGSUSED*/
Char
kkgetc(fp)
    FILE *fp;
{
    return kgetc(kip);
}

/*ARGSUSED*/
kkputc(cc, fp)
    register int cc;
    FILE *fp;
{
    kputc(cc, kop);
}

Char *
safeC(C)
	Char *C;
{
	register Char *CC;

	if (!(CC = (Char *)malloc((unsigned)(Cstrlen(C)+1)*sizeof(Char))))
		return NULL;
	return Cstrcpy(CC, C);
}

char *
safec(c)
	char *c;
{
	register char *cc;

	if (!(cc = malloc((unsigned)strlen(c)+1)))
		return NULL;
	return strcpy(cc, c);
}

#endif /*KANJI*/
