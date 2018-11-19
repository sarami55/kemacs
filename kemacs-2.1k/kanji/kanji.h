#ifndef KFILE

#include "../econfig.h"

#ifndef FILE
#include <stdio.h>
#endif

#ifndef makedev
#include <sys/types.h>
#endif

#ifdef __STDC__
# include <stdlib.h>
#else
# ifdef MALLOC_CHARSTAR
char *malloc(), *realloc();
# else
void *malloc(), *realloc();
# endif
#endif

#if BSD_LEGACY
# include <strings.h>
#else
# include <string.h>
#endif

#if HANDLE_UTF
# include <iconv.h>
# if ICONV_2ARG_CONST
#  define Iconv(d, i, ib, o, ob) iconv((d), (const char**)(i), (ib), (o), (ob))
# else
#  define Iconv iconv
# endif
#endif

#define numberof(x) (sizeof(x)/sizeof(*(x)))
#define tailof(x) ((x)+numberof(x))

#define MAX_U8LEN 6
#define MAXPEND ((MAX_U8LEN)-1)	/* max size of pending stream data */

typedef struct {
  caddr_t	ks_id;		/* pointer to stream structure */
  int (*ks_openf)();	/* pointer to stream open function */
  int (*ks_closef)();	/* pointer to stream close function */
  int (*ks_getf)();	/* pointer to stream get function */
  int (*ks_putf)();	/* pointer to stream put function */
  union ks_un {
    unsigned u_flag; /* controls interpretation of stream */
    struct ks_str {
      unsigned s_thru:1;	/* through 8-bit */
      unsigned s_interp:2;	/* interpretation of stream */
      unsigned s_code:2;	/* KANJI code system */
      unsigned s_pass:1;	/* don't process SI/SO of JIS */
      unsigned s_ki:2;	/* kanji version */
      unsigned s_ri:2;	/* alphabet set */
      unsigned s_eolint:1;	/* EOL interpretation of stream (Add Nide) */
      unsigned s_eol:2;	/* type of EOL (Add Nide; only for fileIO) */
    } u_str;
  } ks_flag;
  int ks_gstate;		/* stream input status */
  int ks_pstate;		/* stream output status */
  int ks_queue[BUFSIZ];	/* buffer for queued characters */
  int ks_qp;		/* queue pointer */
  int ks_ql;		/* queue length */
  int ks_ul;		/* length of unbuffered data (Add Nide) */
  int ks_buf[MAXPEND];	/* buffer for pending stream data */
  int ks_bl;		/* used length of ks_buf[] */
} KSTREAM;

typedef union ks_un	KS_FLAG;
#define KS_VALUE(f)	(f).u_flag
#define KS_THRU(f)	(f).u_str.s_thru
#define KS_INTERP(f)	(f).u_str.s_interp
# define KS_KANJI  0
# define KS_BINARY 1
# define KS_UKANJI 2
#define KS_CODE(f)	(f).u_str.s_code
# define KS_JIS  0
# define KS_UJIS 1
# define KS_SJIS 2
# if HANDLE_UTF
#  define KS_UTF8 3
# endif
#define KS_PASS(f)	(f).u_str.s_pass
#define KS_KI(f)	(f).u_str.s_ki
# define KS_OLDJIS	0
# define KS_NEWJIS	1
#define KS_RI(f)	(f).u_str.s_ri
# define KS_ROMAJI	0
# define KS_ASCII	1
# define KS_BOGUS	2
#define KS_EOLINT(f)	(f).u_str.s_eolint
# define KS_EOLKNOWN	0
# define KS_EOLUK	1
#define KS_EOL(f)	(f).u_str.s_eol
# define KS_LF		0
# define KS_CR		1
# define KS_CRLF	2

#define KFILE		KSTREAM

#define KS_BINMODE	ks_binmode_flag()

#ifdef iscntrl
# undef iscntrl
#endif

#define iscntrl(c) ((c) <= ' ' || (c) == 0177)
#define isascii(c) (((c) & 0xff00) == 0)
#define iskana(c) (((c) & 0xff00) == 0x0100)
#define iskanji(c) (((c) & 0xff00) > 0x0100)
  
#define enkana(k, c) ((k) = (((c) & 0x7f) | 0x0100))
#define dekana(k, c) ((c) = ((k) & 0x7f))
  
#define enkanji(k, c1, c2) ((k) = ((((c1) & 0x7f) << 8) | ((c2) & 0x7f)))
#define dekanji(k, c1, c2) ((((c1) = ((k) >> 8) & 0x7f)), ((c2) = (k) & 0x7f))
  
#define ESCAPE		0x1b
#define SHFTIN		0x0f
#define SHFTOU		0x0e
#define SSHFT2		0x8e
#define SSHFT3		0x8f
#define CNTRLZ		0x1a

extern KSTREAM *	kalloc();
extern int		kfree();
extern int		kgetc();
extern int		kputc();
extern KFILE *		kopen();
extern void		kclose();
extern int		jtos();
extern int		stoj();

#endif /* !KFILE */
