#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*	tcap:	Unix V5, V7 and BS4.2 Termcap video driver
		for MicroEMACS
*/

#define termdef 1			/* don't define "term" external */

#include	"ecomm.h"
#include	"estruct.h"
#include	"edef.h"

static Char *dummy = CSTR(&Cxstr[6]);

#define MARGIN	8
#define SCRSIZ	48
#define NPAUSE	10			/* # times thru update to pause */
#define BEL	0x07
#define ESC	0x1B

extern int	ttopen();
extern Char	ttgetc();
extern int	ttputc();
extern int	tgetnum();
extern int	ttflush();
extern int	ttclose();
extern int	tcapkopen();
extern int	tcapkclose();
extern int	tcapmove();
extern int	tcapeeol();
extern int	tcapeeop();
extern int	tcapbeep();
extern int	tcaprev();
extern int	tcapopen();
extern char	*tgoto();
#if	COLOR
extern	int	tcapfcol();
extern	int	tcapbcol();
#endif

#define TCAPSLEN 1024
static char tcapbuf[TCAPSLEN];
char *UP;
static char PC;
static char *CM, *CE, *CL, *SO, *SE, *VS, *VE, *TI, *TE, *KS, *KE;

struct tctab {
    char *cap;
    char **str;
};

static struct tctab tctab[] = {
    {"up", &UP}, {"cm", &CM}, {"ce", &CE}, {"cl", &CL}, {"so", &SO},
    {"se", &SE}, {"vs", &VS}, {"ve", &VE}, {"ti", &TI}, {"te", &TE},
    {"ks", &KS}, {"ke", &KE},
    {NULL, NULL}};

TERM term = {
	0,	/* these four values are set dynamically at open time */
	0,
	0,
	0,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	tcapopen,
	ttclose,
        tcapkopen,
        tcapkclose,
	ttgetc,
	ttputc,
	ttflush,
	tcapmove,
	tcapeeol,
	tcapeeop,
	tcapbeep,
        tcaprev,
#if	COLOR
	tcapfcol,
	tcapbcol
#endif
};

tcapopen()
{
	char *getenv();
	char *t, *p, *tgetstr();
	char tcbuf[1024];
	char *tv_stype;
	char err_str[72];
#if	KANJI
	char *kj;
#endif
	struct tctab *tp;

	if ((tv_stype = getenv("TERM")) == NULL) {
		puts("Environment variable TERM not defined!");
		exit(1);
	}

	if ((tgetent(tcbuf, tv_stype)) != 1) {
		(void)sprintf(err_str, "Unknown terminal type %s!", tv_stype);
		puts(err_str);
		exit(1);
	}

	get_screen_size(&term.t_nrow, &term.t_ncol);
	term.t_mrow = term.t_nrow;
	term.t_mcol = term.t_ncol;

	p = tcapbuf;
	t = tgetstr("pc", &p);
	if (t)
		PC = *t;

	for (tp = tctab; tp->cap; tp++)
	    *tp->str = tgetstr(tp->cap, &p);

	if (SO != NULL)
		revexist = TRUE;

	if(CL == NULL || CM == NULL || UP == NULL)
	{
		puts("Incomplete termcap entry\n");
		exit(1);
	}

	if (CE == NULL)		/* will we be able to use clear to EOL? */
		eolexist = FALSE;

#if KANJI
# if defined(DEF_T_CODE)
	KS_INTERP(term.t_code) = KS_KANJI;
	KS_CODE(term.t_code) = langcode(DEF_T_CODE);
	KS_THRU(term.t_code) = DEF_T_THRU;
#  if defined(DEF_T_KI) && defined(DEF_T_RI)
	KS_KI(term.t_code) = DEF_T_KI;
	KS_RI(term.t_code) = DEF_T_RI;
#  endif
#  if KMSGS
	kterminal = TRUE;
#  endif
# else /* !DEF_T_CODE */
	KS_INTERP(term.t_code) = KS_BINARY;
#  if KMSGS
	kterminal = FALSE;
#  endif
# endif /* !DEF_T_CODE */
	KS_EOLINT(term.t_code) = KS_EOLKNOWN; /* EOL type is fixed on KS_LF */
	KS_EOL(term.t_code) = KS_LF; /* fixed on KS_LF */
# if defined(KANJI_TERMCAP)
	/* get terminal's KANJI spec from TERMCAP */
	if ((kj = tgetstr(KANJI_TERMCAP, &p)) != NULL) {
	  /* full spec can be retrieved */
	  KS_INTERP(term.t_code) = KS_KANJI;
#  if KMSGS
	  kterminal = TRUE;
#  endif
	  switch (*kj) {
	  case 'J': case 'j':
	    KS_CODE(term.t_code) = KS_JIS;
	    KS_THRU(term.t_code) = (*kj++ == 'J');
	    switch (*kj) {
	    case '@':
	      KS_KI(term.t_code) = KS_OLDJIS;
	      break;
	    case 'B':
	      KS_KI(term.t_code) = KS_NEWJIS;
	      break;
	    }
	    if (*kj++) {
	      switch (*kj) {
	      case 'B':
		KS_RI(term.t_code) = KS_ASCII;
		break;
	      case 'H':
		KS_RI(term.t_code) = KS_BOGUS;
		break;
	      case 'J':
		KS_RI(term.t_code) = KS_ROMAJI;
		break;
	      }
	    }
	    break;
	  case 'u': case 'U':
	    KS_CODE(term.t_code) = KS_UJIS;
	    KS_THRU(term.t_code) = 1;
	    break;
	  case 's': case 'S':
	    KS_CODE(term.t_code) = KS_SJIS;
	    KS_THRU(term.t_code) = 1;
	    break;
	  /* UTF8 not yet supported */
	  }
	} else if (tgetflag(KANJI_TERMCAP)) {
#  if KMSGS
	  kterminal = TRUE;
#  endif
	  KS_INTERP(term.t_code) = KS_KANJI;
	}
# endif /* KANJI_TERMCAP */
#endif /* KANJI */
	
	if (p >= &tcapbuf[TCAPSLEN])
	{
		puts("Terminal description too big!\n");
		exit(1);
	}
	ttopen();
}

tcapkopen()
{
#if KANJI
	kkopen();
	putpad(KS);
#endif
	putpad(TI);
	putpad(VS);
}

tcapkclose()
{
	putpad(VE);
	putpad(TE);
#if KANJI
	putpad(KE);
	kkclose();
#endif
}

tcapmove(row, col)
	register int row, col;
{
	putpad(tgoto(CM, col, row));
}

tcapeeol()
{
	putpad(CE);
}

tcapeeop()
{
	putpad(CL);
}

tcaprev(state)		/* change reverse video status */
	int state;	/* FALSE = normal video, TRUE = reverse video */
{
	static int revstate = FALSE;
	if (state && !revstate) {
		if (SO != NULL)
			putpad(SO);
	}
	if (!state && revstate) {
		if (SE != NULL)
			putpad(SE);
	}
	revstate = state;
}

#if	COLOR
tcapfcol()	/* no colors here, ignore this */
{
}

tcapbcol()	/* no colors here, ignore this */
{
}
#endif

tcapbeep()
{
	ttputc(BEL);
}

putpad(str)
	char	*str;
{
	putnpad(str, 1);
}

putnpad(str, n)
	char	*str;
{
	if(str != NULL)
		tputs(str, n, ttputc);
}

#if	FLABEL
/*ARGSUSED*/
fnclabel(f, n)		/* label a function key */
{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif /*FLABEL*/

#include <signal.h>
#if HAVE_TERMIOS
# include <termios.h>
#else
# if USG
#  include <termio.h>
# endif
#endif
#ifdef SIGWINCH
# include <sys/ioctl.h>
#endif

/* Get terminal size */
 /* modified by N.Nide; use LINES and COLUMNS if we have them (for uum) */
get_screen_size(rowp, colp)
int *rowp, *colp;
{
	int row, col, i;
	char	*p;

#ifdef TIOCGWINSZ
 /* try TIOCGWINSZ first, since some systems have #define TIOCGSIZE TIOCGWINSZ
    but don't have struct winsize. */
	struct winsize wsz;

	if(ioctl(fileno(stdout), TIOCGWINSZ, &wsz) == -1)
		wsz.ws_row = wsz.ws_col = 0;
#else
#ifdef TIOCGSIZE
	struct ttysize tty;

	if (ioctl(fileno(stdout), TIOCGSIZE, &tty) == -1)
		tty.ts_lines = tty.ts_cols = 0;
#endif
#endif

#if USE_ENVSZ
	if(NULL != (p = getenv("LINES"))){
		row = atoi(p) - 1;
	} else
#endif
#ifdef TIOCGWINSZ
	if(wsz.ws_row){
		row = wsz.ws_row - 1;
	} else
#else
#ifdef TIOCGSIZE
	if(tty.ts_lines){
		row = tty.ts_lines - 1;
	} else
#endif
#endif
	if((i = tgetnum("li")) != -1){
		row = i - 1;
	} else {
		puts("termcap entry incomplete (lines)");
		exit(1);
	}

#if USE_ENVSZ
	if(NULL != (p = getenv("COLUMNS"))){
		col = atoi(p);
	} else
#endif
#ifdef TIOCGWINSZ
	if(wsz.ws_col){
		col = wsz.ws_col;
	} else
#else
#ifdef TIOCGSIZE
	if(tty.ts_cols){
		col = tty.ts_cols;
	} else
#endif
#endif
	if((i = tgetnum("co")) != -1){
		col = i;
	} else {
		puts("Termcap entry incomplete (columns)");
		exit(1);
	}

	if (row > MAX_SCREEN_LINES) {
		puts("Number of lines on screen is larger than configration.");
		exit(1);
	}
	if (col > MAX_SCREEN_COLUMNS) {
		puts("Number of columns on screen is larger than configration.");
		exit(1);
	}
	*rowp = row;
	*colp = col;
}
