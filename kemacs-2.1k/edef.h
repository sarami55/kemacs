#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*	EDEF:		Global variable definitions for
			MicroEMACS 3.2

			written by Dave G. Conroy
			modified by Steve Wilhite, George Jones
			greatly modified by Daniel Lawrence
*/

#if	SHORTNAME
/* shorten long names */
# define mlreply	mlrepl
# define mlreplyt	mlrept
# define tcapeeol	tcpeol
# define tcapeeop	tcpeop
# define scrnextup	scnxup
# define scrnextdw	scnxdw
# define matchline	mtclin
# define matchlen	mtclen
# if	KANJI
#  define kinsoku1	kinsk1
#  define kinsoku2	kinsk2
# endif
#endif	/*SHORTNAME*/

/* some global fuction declarations */

#ifdef __STDC__
# include <stdlib.h>
# if defined(__unix__) || defined(__MACH__)
#  include <unistd.h>
# endif
#else
# ifdef MALLOC_CHARSTAR
char *malloc();
# else
void *malloc();
# endif
#endif
char *getenv();

#define itoa myitoa
Char *itoa();
Char *getval();
Char *token();
Char *ltos();
Char ectoc();
Ckey ctoec();
Char tgetc();
Ckey get1key();
#if HAVE_REGEX
# include <regex.h>
#else
# if	BSD
char *re_comp();
int re_exec();
# endif
# if	USG
char *regcmp();
char *regex();
# endif
#endif
int getccol();	/* get current column */
Char *binding_list_buffer_name();
Char *buffer_list_buffer_name();
Char *help_buffer_name();
Char *version();
Char *prgname();
Char *revdate();
#if	KANJI
Char *code_string();
int string_code();
unsigned kfclose();
#endif
Char *mkupper();
Char *mklower();
Char *getclock();
Char *key2fname();
Ckey strcmd();
#if	USE_LANG
int langcode();
#else
# define langcode(x) (x)
#endif

#ifdef	maindef
# define EXT
# define INIT(x)	= x
#else
# define EXT		extern
# define INIT(x)
#endif

/* for MAIN.C */

/* initialized global definitions */

EXT int		fillcol		INIT(72);			/* Current fill column	*/
EXT Ckey	kbdm[NKBDM]	INIT({CTLX|')'});	/* Macro				*/
EXT Char	*execstr	INIT(NULL);			/* pointer to string to execute	*/
EXT Char	golabel[NPAT] INIT({'\0'});		/* current line to go to */
EXT int		execlevel	INIT(0);			/* execution IF level	*/
EXT int		eolexist	INIT(TRUE);			/* does clear to EOL exist */
EXT int		revexist	INIT(FALSE);		/* does reverse video exist? */
EXT Char	*modename[]						/* name of modes		*/
#ifdef maindef
	= {	CSTR(&Cxstr[140]), CSTR(&Cxstr[145]), CSTR(&Cxstr[151]),
		CSTR(&Cxstr[157]), CSTR(&Cxstr[163]), CSTR(&Cxstr[168]),
# if MAGIC
		CSTR(&Cxstr[173]),
# endif
# if CRYPT
		CSTR(&Cxstr[179]),
# endif
# if ASAVE
		CSTR(&Cxstr[185]),
# endif
# if KANJI
		CSTR(&Cxstr[191]),
# endif
		NULL}
#endif /* maindef */
	;
EXT Char	modecode[]						/* letters to represent modes */
#ifdef maindef
	= {	'W', 'C', 'S', 'E', 'V', 'O',
# if MAGIC
		'M',
# endif
# if CRYPT
		'Y',
# endif
# if ASAVE
		'A',
# endif
# if KANJI
		'B',
# endif
		'\0'}
#endif /* maindef */
		;
EXT int		gmode		INIT(0);			/* global editor mode		*/
#if COLOR
EXT int		gfcolor		INIT(7);			/* global forgrnd color (white) */
EXT int		gbcolor		INIT(0);			/* global backgrnd color (black)*/
#endif
#if ASAVE
EXT int		gasave		INIT(256);			/* global ASAVE size		*/
EXT int		gacount		INIT(256);			/* count until next ASAVE	*/
#endif
EXT int		sgarbf		INIT(TRUE);			/* TRUE if screen is garbage */
EXT int		mpresf		INIT(FALSE);		/* TRUE if message in last line */
EXT int		clexec		INIT(FALSE);		/* command line execution flag	*/
EXT int		mstore		INIT(FALSE);		/* storing text to macro flag	*/
EXT int		discmd		INIT(TRUE);			/* display command flag		*/
EXT int		disinp		INIT(TRUE);			/* display input characters	*/
EXT struct BUFFER *bstore INIT(NULL);		/* buffer to store macro text to */
EXT int		vtrow		INIT(0);			/* Row location of SW cursor */
EXT int		vtcol		INIT(0);			/* Column location of SW cursor */
EXT int		ttrow		INIT(HUGE);			/* Row location of HW cursor */
EXT int		ttcol		INIT(HUGE);			/* Column location of HW cursor */
EXT int		lbound		INIT(0);			/* leftmost column of current	*/
											/* line being displayed 		*/
EXT int		taboff		INIT(0);			/* tab offset for display	*/
EXT Ckey	metac		INIT(CTLFLG|'[');		/* current meta character */
EXT Ckey	ctlxc		INIT(CTLFLG|'X');		/* current control X prefix char */
EXT Ckey	reptc		INIT(CTLFLG|'U');		/* current universal repeat char */
EXT Ckey	abortc		INIT(CTLFLG|'G');		/* current abort command char	*/

EXT Char	quotec		INIT(0x11);			/* quote char during mlreply() */
#if COLOR
EXT Char	*cname[]						/* names of colors		*/
# ifdef maindef
	= {	CSTR(&Cxstr[198]), CSTR(&Cxstr[204]), CSTR(&Cxstr[208]), CSTR(&Cxstr[214]),
		CSTR(&Cxstr[221]), CSTR(&Cxstr[226]), CSTR(&Cxstr[234]), CSTR(&Cxstr[239]) }
# endif
	;
#endif /* COLOR */
EXT KILL	*kbufp		INIT(NULL);			/* current kill buffer chunk ptr */
EXT KILL	*kbufh		INIT(NULL);			/* kill buffer header pointer	*/
EXT int		kused		INIT(KBLOCK);		/* # of bytes used in kill buffer*/
EXT WINDOW	*swindow	INIT(NULL);			/* saved window pointer			*/
#if KMSGS
EXT int		kterminal	INIT(0);			/* use KANJI messages			*/
#endif
#if AUTOMODE
EXT int		npatent		INIT(1);			/* # of pattern table entry	*/
EXT PATTBL	pattbl[NMPAT] 	 				/* pattern table		*/
# ifdef maindef
	= {	{ "\\.[ch]$", MDCMOD } }
# endif
	;
#endif /* AUTOMODE */
#if	CRYPT
EXT int		cryptflag	INIT(FALSE);		/* currently encrypting?	*/
#endif
EXT Ckey	*kbdend		INIT(&kbdm[0]);		/* ptr to end of the keyboard	*/
EXT int		kbdmode		INIT(STOP);			/* current keyboard macro mode	*/
EXT int		kbdrep		INIT(0);			/* number of repetitions	*/
#if	RESTRICT
EXT int		restflag	INIT(FALSE);		/* restricted use?		*/
#endif
EXT int		lastkey		INIT(0);			/* last keystoke		*/
EXT int		seed		INIT(0);			/* random number seed		*/
EXT int		macbug		INIT(FALSE);		/* macro debuging flag		*/
EXT Char	*errorm		INIT(CSTR(&Cxstr[245])); /* error literal		*/
EXT Char	*truem		INIT(CSTR(&Cxstr[251]));	/* true literal			*/
EXT Char	*falsem		INIT(CSTR(&Cxstr[256])); /* false litereal		*/
EXT int		cmdstatus	INIT(TRUE);			/* last command status		*/
EXT int		reclevel	INIT(0);			/* recursive level of editing	*/
EXT BUFFER	*blistp		INIT(NULL);			/* Buffer for C-X C-B		*/
EXT BUFFER	*bdlistp	INIT(NULL);			/* Binding List Buffer		*/
EXT BUFFER	*hlpbp		INIT(NULL);			/* Help Buffer			*/
#ifndef maindef
EXT KEYTAB	keytab[];
EXT NBIND	names[];
#endif
#if KANJI
EXT Char	*gmodeline	INIT(CSTR(&Cxstr[262]));
#else
EXT Char	*gmodeline	INIT(CSTR(&Cxstr[315]));
#endif
					/* Global mode line format	*/
#if	CLOCK
EXT int		clflag		INIT(0);	/* flag for clock	*/
EXT int		cperiod		INIT(CPERIOD);	/* timer interval	*/
#endif

/* uninitialized global definitions */

EXT int		currow;			/* Cursor row			*/
EXT int		curcol;			/* Cursor column		*/
EXT int		thisflag;		/* Flags, this command		*/
EXT int		lastflag;		/* Flags, last command		*/
EXT int		curgoal;		/* Goal for C-P, C-N		*/
EXT WINDOW	*curwp;			/* Current window		*/
EXT BUFFER	*curbp;			/* Current buffer		*/
EXT WINDOW	*wheadp;		/* Head of list of windows	*/
EXT BUFFER	*bheadp;		/* Head of list of buffers	*/

EXT BUFFER	*bfind();		/* Lookup a buffer by name	*/
EXT WINDOW	*wpopup();		/* Pop up window creation	*/
EXT LINE	*lalloc();		/* Allocate a line		*/
#if	KANJI
EXT KS_FLAG	kcode;			/* global file code		*/
#endif	/*KANJI*/
EXT Ckey	*kbdptr;		/* current position in keyboard buf */
EXT Char    pat[NPAT];		/* Search pattern		*/
EXT Char	tap[NPAT];		/* Reversed pattern array.	*/
EXT Char	rpat[NPAT];		/* replacement pattern		*/

/* The variable matchlen holds the length of the matched
 * string - used by the replace functions.
 * The variable patmatch holds the string that satisfies
 * the search command.
 * The variables matchline and matchoff hold the line and
 * offset position of the start of match.
 */
EXT int		matchlen		INIT(0);
EXT int		mlenold			INIT(0);
EXT Char	*patmatch		INIT(NULL);
EXT LINE	*matchline		INIT(NULL);
EXT int		matchoff		INIT(0);

#if	MAGIC
/*
 * The variable magical determines if there are actual
 * metacharacters in the string - if not, then we don't
 * have to use the slower MAGIC mode search functions.
 */
EXT int		magical			INIT(FALSE);
EXT MC		mcpat[NPAT];		/* the magic pattern		*/
EXT MC		tapcm[NPAT];		/* the reversed magic pattern	*/

#endif /* MAGIC */

/* terminal table defined only in TERM.C */

#ifndef termdef
extern	TERM	term;			/* Terminal information.	*/
#endif

#undef EXT
#undef INIT
