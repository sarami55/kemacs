#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*	EVAR.H:	Environment and user variable definitions
		for MicroEMACS

		written 1986 by Daniel Lawrence
*/

/*	structure to hold user variables and their definitions	*/

typedef struct UVAR {
	Char u_name[NVSIZE + 1];		/* name of user variable */
	Char *u_value;				/* value (string) */
} UVAR;

/*	current user variables (This structure will probably change)	*/

#define	MAXVARS		100

UVAR uv[MAXVARS];	/* user variables */

/* environment variable structure */

typedef struct ENVAR {
	Char *ev_name;			/* name of variable */
	byte ev_class;			/* local or global */
	char *ev_addr;			/* address of related object */
	Char *(*ev_gethook)();		/* routine to get the value */
	int (*ev_puthook)();		/* routine to put the value */
} ENVAR;

#define EV_BLOCAL	0	/* env variable is buffer local */
#define EV_WLOCAL	1	/* env variable is window local */
#define EV_GLOBAL	2	/* env variable is global */

static union {
	BUFFER buffer;
	WINDOW window;
} dummy_str;
#define dummy_bptr	(&(dummy_str.buffer))
#define dummy_wptr	(&(dummy_str.window))

#define BUF_OFS(f)	(char *)&(dummy_bptr->f)
#define BUF_ADR(o)	((char *)curbp+((char *)o-(char *)dummy_bptr))
#define WIN_OFS(f)	(char *)&(dummy_wptr->f)
#define WIN_ADR(o)	((char *)curwp+((char *)o-(char *)dummy_wptr))

#if	SHORTNAME
# define get_curchar_hook	get_cuch_hook
# define get_curcol_hook	get_ccol_hook
# define get_curline_hook	get_clin_hook
# define get_curwidth_hook	get_cwid_hook
# define put_curchar_hook	put_cuch_hook
# define put_curcol_hook	put_ccol_hook
# define put_curline_hook	put_clin_hook
# define put_curwidth_hook	put_cwid_hook
#endif	/*SHORTNAME*/

/*
 * hook functions: declared as following
 *
 *	Char *get_hook(ENVAR *);
 *	int put_hook(ENVAR *, Char *);
 */
/* hook for putting to readonly variables */
static int readonly_hook();
/* general get hooks */
static Char *get_int_hook(), *get_bool_hook(), *get_str_hook();
/* general put hooks */
static int put_int_hook(), put_bool_hook(), put_str_hook();

/* variable specific hooks */
static Char *get_pagelen_hook(), *get_curcol_hook(), *get_curline_hook();
static int put_pagelen_hook(), put_curcol_hook(), put_curline_hook();
static int put_curwidth_hook(), put_cbufname_hook(), put_cfname_hook();
#if KANJI
static int put_kterminal_hook();
static Char *get_kcode_hook();
static int put_kcode_hook();
static int put_tcode_hook();
static int put_bufcode_hook();
#endif
static Char *get_curchar_hook();
static int put_curchar_hook();
static int put_modeline_hook();
#if	CLOCK
static int put_cperiod_hook();
#endif

/*	list of recognized environment variables	*/
ENVAR envars[] = {
	{CSTR(&Cxstr[1402]), EV_GLOBAL,	/* current fill column */
		(char *)&fillcol, get_int_hook, put_int_hook},
	{CSTR(&Cxstr[1410]), EV_GLOBAL,	/* number of lines used by editor */
		NULL, get_pagelen_hook, put_pagelen_hook},
	{CSTR(&Cxstr[1418]), EV_BLOCAL,	/* current column pos of cursor */
		NULL, get_curcol_hook, put_curcol_hook},
	{CSTR(&Cxstr[1425]), EV_BLOCAL,	/* current line in file */
		NULL, get_curline_hook, put_curline_hook},
	{CSTR(&Cxstr[1433]), EV_GLOBAL,	/* current screen width */
		(char *)&term.t_ncol, get_int_hook, put_curwidth_hook},
	{CSTR(&Cxstr[1442]), EV_BLOCAL,	/* current buffer name */
		BUF_OFS(b_bname), get_str_hook, put_cbufname_hook},
	{CSTR(&Cxstr[1451]), EV_BLOCAL,	/* current file name */
		BUF_OFS(b_fname), get_str_hook, put_cfname_hook},
	{CSTR(&Cxstr[1458]),	EV_GLOBAL,	/* macro debugging */
		(char *)&macbug, get_bool_hook, put_bool_hook},
	{CSTR(&Cxstr[1464]), EV_GLOBAL,	/* status of last command */
		(char *)&cmdstatus, get_bool_hook, put_bool_hook},
#if KANJI
	{CSTR(&Cxstr[1471]), EV_GLOBAL,	/* display KANJI messages */
		(char *)&kterminal, get_bool_hook, put_kterminal_hook},
	{CSTR(&Cxstr[1477]), EV_GLOBAL,	/* global file code */
		(char *)&kcode, get_kcode_hook, put_kcode_hook},
	{CSTR(&Cxstr[1483]), EV_GLOBAL,	/* terminal code */
		(char *)&term.t_code, get_kcode_hook, put_tcode_hook},
	{CSTR(&Cxstr[1489]), EV_BLOCAL,	/* current buffer's file code */
		BUF_OFS(b_code), get_kcode_hook, put_bufcode_hook},
#endif
#if ASAVE
	{CSTR(&Cxstr[1495]), EV_GLOBAL,	/* # of chars between auto-saves */
	   (char *)&gasave, get_int_hook, put_int_hook},
	{CSTR(&Cxstr[1501]), EV_GLOBAL,	/* # of chars until next auto-save */
	   (char *)&gacount, get_int_hook, put_int_hook},
#endif
	{CSTR(&Cxstr[1508]), EV_GLOBAL,	/* last keyboard char struck */
	   (char *)&lastkey, get_int_hook, put_int_hook},
	{CSTR(&Cxstr[1516]), EV_GLOBAL,	/* current character under the cursor */
	   NULL, get_curchar_hook, put_curchar_hook},
	{CSTR(&Cxstr[1524]), EV_GLOBAL,	/* display commands on command line */
	   (char *)&discmd, get_bool_hook, put_bool_hook},
	{CSTR(&Cxstr[1531]), EV_GLOBAL,	/* current version number */
	   NULL, version, readonly_hook},
	{CSTR(&Cxstr[1539]), EV_GLOBAL,	/* returns current prog name */
	   NULL, prgname, readonly_hook},
	{CSTR(&Cxstr[1548]), EV_GLOBAL,	/* returns last revised date */
	   NULL, revdate, readonly_hook},
	{CSTR(&Cxstr[1556]), EV_GLOBAL,	/* current random number seed */
	   (char *)&seed, get_int_hook, put_int_hook},
	{CSTR(&Cxstr[1561]), EV_GLOBAL,	/* display command line input characters */
	   (char *)&disinp, get_bool_hook, put_bool_hook},
	{CSTR(&Cxstr[1568]), EV_GLOBAL,	/* global mode line format */
	   (char *)&gmodeline, get_str_hook, put_modeline_hook},
	{CSTR(&Cxstr[1578]), EV_BLOCAL,	/* buffer mode line format */
		BUF_OFS(b_modeline), get_str_hook, put_modeline_hook},
#if	CLOCK
	{CSTR(&Cxstr[1588]), EV_GLOBAL,	/* period of timer in seconds */
		(char *)&cperiod, get_int_hook, put_cperiod_hook},
#endif
	{CSTR(&Cxstr[1596]),	EV_GLOBAL,	/* current time string */
		NULL, getclock, readonly_hook},

	{CSTR(&Cxstr[1601]), EV_GLOBAL,	/* current search pattern */
		(char *)pat, NULL, NULL},
	{CSTR(&Cxstr[1611]), EV_GLOBAL,	/* current replace pattern */
		(char *)rpat, NULL, NULL},
	{CSTR(&Cxstr[1622]), EV_WLOCAL,	/* # of rows of text in window */
		WIN_OFS(w_ntrows), get_int_hook, readonly_hook},
	{CSTR(&Cxstr[1630]), EV_WLOCAL,	/* origin of top row of window */
		WIN_OFS(w_toprow), get_int_hook, readonly_hook},
	{CSTR(&Cxstr[1638]), EV_WLOCAL,	/* window id */
		WIN_OFS(w_id), get_int_hook, readonly_hook},
};

#define	NEVARS	(sizeof(envars) / sizeof(ENVAR))

/*	list of recognized user functions	*/

enum FUNC_ADITY {
  NILNAMIC,
  MONAMIC,
  DYNAMIC,
  TRINAMIC,
};

typedef struct UFUNC {
	Char *f_name;	/* name of function */
	enum FUNC_ADITY f_type;
} UFUNC;

UFUNC funcs[] = {
	CSTR(&Cxstr[1645]), DYNAMIC,		/* add two numbers together */
	CSTR(&Cxstr[1649]), DYNAMIC,		/* subtraction */
	CSTR(&Cxstr[1653]), DYNAMIC,		/* multiplication */
	CSTR(&Cxstr[1657]), DYNAMIC,		/* division */
	CSTR(&Cxstr[1661]), DYNAMIC,		/* mod */
	CSTR(&Cxstr[1665]), MONAMIC,		/* negate */
	CSTR(&Cxstr[1669]), DYNAMIC,		/* concatinate string */
	CSTR(&Cxstr[1673]), DYNAMIC,		/* left string(string, len) */
	CSTR(&Cxstr[1677]), DYNAMIC,		/* right string(string, pos) */
	CSTR(&Cxstr[1681]), TRINAMIC,	/* mid string(string, pos, len) */
	CSTR(&Cxstr[1685]), MONAMIC,		/* logical not */
	CSTR(&Cxstr[1689]), DYNAMIC,		/* logical equality check */
	CSTR(&Cxstr[1693]), DYNAMIC,		/* logical less than */
	CSTR(&Cxstr[1697]), DYNAMIC,		/* logical greater than */
	CSTR(&Cxstr[1701]), DYNAMIC,		/* string logical equality check */
	CSTR(&Cxstr[1705]), DYNAMIC,		/* string logical less than */
	CSTR(&Cxstr[1709]), DYNAMIC,		/* string logical greater than */
	CSTR(&Cxstr[1277]), MONAMIC,		/* evaluate indirect value */
	CSTR(&Cxstr[1713]), MONAMIC,		/* get UNIX environment */
	CSTR(&Cxstr[1414]), MONAMIC,		/* length of string */
	CSTR(&Cxstr[1717]), DYNAMIC,		/* logical and */
	CSTR(&Cxstr[1721]),  DYNAMIC,		/* logical or */
	CSTR(&Cxstr[1724]), MONAMIC,		/* uppercase string */
	CSTR(&Cxstr[1728]), MONAMIC,		/* lower case string */
	CSTR(&Cxstr[1732]), MONAMIC,		/* Truth of the universe logical test */
	CSTR(&Cxstr[1736]), MONAMIC,		/* char to integer conversion */
	CSTR(&Cxstr[1740]), MONAMIC,		/* integer to char conversion */
	CSTR(&Cxstr[1744]), NILNAMIC,		/* get 1 charater */
	CSTR(&Cxstr[1748]), MONAMIC,		/* get a random number */
	CSTR(&Cxstr[1752]), MONAMIC,		/* absolute value of a number */
	CSTR(&Cxstr[1756]), MONAMIC,		/* function name bound to this key sequence */

	CSTR(&Cxstr[1760]), MONAMIC,
	CSTR(&Cxstr[1764]), MONAMIC,
};

#define	NFUNCS	sizeof(funcs) / sizeof(UFUNC)

/* 	and its preprocesor definitions		*/

enum UF_NUMBERS {
	UFADD,
	UFSUB,
	UFTIMES,
	UFDIV,
	UFMOD,
	UFNEG,
	UFCAT,
	UFLEFT,
	UFRIGHT,
	UFMID,
	UFNOT,
	UFEQUAL,
	UFLESS,
	UFGREATER,
	UFSEQUAL,
	UFSLESS,
	UFSGREAT,
	UFIND,
	UFENV,
	UFLENGTH,
	UFAND,
	UFOR,
	UFUPPER,
	UFLOWER,
	UFTRUTH,
	UFASCII,
	UFCHR,
	UFGTKEY,
	UFRND,
	UFABS,
	UFBIND,

	UFBMODE,
	UFGMODE,
};
