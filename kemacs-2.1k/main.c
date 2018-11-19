#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*
 *	Nihongo MicroEMACS (kemacs)
 *
 *	modified version of MicroEMACS 3.8 added Japanese features.
 *			original by Dave G. Conroy & Daniel M. Lawrence
 *			modified by Takanori (sanewo) Saneto
 *				Email: sanewo@strg.sony.co.jp
 */

/*
 *	MicroEMACS 3.8
 *			written by Dave G. Conroy.
 *			greatly modified by Daniel M. Lawrence
 *
 *	(C)opyright 1987 by Daniel M. Lawrence
 *	MicroEMACS 3.8 can be copied and distributed freely for any
 *	non-commercial purposes. MicroEMACS 3.8 can only be incorporated
 *	into commercial software with the permission of the current author.
 *
 * This file contains the main driving routine, and some keyboard processing
 * code, for the MicroEMACS screen editor.
 */

#include	"ecomm.h"

/* make global definitions not external */
#define maindef

#include	"estruct.h"	/* global structures and defines */
#include	"edef.h"	/* global definitions */
#include	"efunc.h"	/* function declarations and name table */
#include	"ebind.h"	/* default key bindings */

#if	CLOCK
# include	<signal.h>
#endif

main(argc, argv)
    char *argv[];
{
    register BUFFER *bp;
    register int ffile;		/* first file flag */
    register int carg;		/* current arg to scan */
    register int startf;	/* startup executed flag */
    int viewflag;		/* are we starting in view mode? */
    int gotoflag;		/* do we need to goto a line at start? */
    int gline;			/* if so, what line? */
    int searchflag;		/* Do we need to search at start? */
    Char bname[NBUFN];		/* buffer name of file to read */
    Ckey getcmd();
#if	CRYPT
    int eflag;			/* encrypting on the way in? */
    char ekey[NPAT];		/* startup encryption key */
#endif

    /* initialize the editor and process the command line arguments */
    (void)Cstrcpy(bname, CSTR(&Cxstr[3314])); /* default buffer name */
    gmodeline = safeC(gmodeline); /* fix by N.Nide;
				   not to free initial value of gmodeline */
    vtinit();			/* Displays.		*/
    edinit(bname);		/* Buffers, windows.	*/
    varinit();			/* user variables	*/
#if	CLOCK
    clinit();			/* initialize timer	*/
#endif
    int noopt = FALSE;		/* do not handle option arguments */
    viewflag = FALSE;		/* view mode defaults off in command line */
    gotoflag = FALSE;		/* set to off to begin with */
    searchflag = FALSE;		/* set to off to begin with */
    ffile = TRUE;		/* no file to edit yet */
    startf = FALSE;		/* startup file not executed yet */
#if	CRYPT
    eflag = FALSE;		/* no encryption by default */
#endif
#if	COLOR
    curwp->w_fcolor = gfcolor;		/* and set colors	*/
    curwp->w_bcolor = gbcolor;
#endif
#if	KANJI
# if defined(DEF_F_CODE)
    KS_INTERP(kcode) = KS_KANJI;
    KS_CODE(kcode) = langcode(DEF_F_CODE);
#  if defined(DEF_F_KI) && defined(DEF_F_RI) /* in the case of JIS */
    KS_KI(kcode) = DEF_F_KI;
    KS_RI(kcode) = DEF_F_RI;
#  endif /* JIS */
# else /* !defined(DEF_F_CODE) */
    KS_INTERP(kcode) = KS_BINARY;
# endif /* !defined(DEF_F_CODE) */
    KS_EOLINT(kcode) = KS_EOLKNOWN;
# if defined(DEF_F_EOL)
    KS_EOL(kcode) = DEF_F_EOL;
# else
    KS_EOL(kcode) = KS_LF;
# endif
    curwp->w_bufp->b_code = kcode;
#endif /* KANJI */

    /* scan through the command line and get the files to edit */
    for (carg = 1; carg < argc; ++carg) {
	if (noopt) {
	    goto regular_arg; /* do not handle option arguments */
	} else
	if (strcmp(argv[carg], "--") == 0) {
	    noopt = TRUE;
	} else
	/* if its a switch, process it */
	if (argv[carg][0] == '-') {
#if	KANJI
	    KS_FLAG *cp;
	    char *argp;
#endif

	    switch (argv[carg][1]) {
	    case 'v':	/* -v for View File */
	    case 'V':
		viewflag = TRUE;
		break;
	    case 'e':	/* -e for Edit file */
	    case 'E':
		viewflag = FALSE;
		break;
	    case 's':	/* -s for initial search string */
	    case 'S':
		searchflag = TRUE;
		(void)Cstrncpy(pat, Cfromc(&argv[carg][2]), NPAT);
		break;
	    case 'g':	/* -g for initial goto */
	    case 'G':	
		gotoflag = TRUE;
		gline = atoi(&argv[carg][2]);
		break;
#if	RESTRICT
	    case 'r':	/* -r restrictive use */
	    case 'R':
		restflag = TRUE;
		break;
#endif
#if	CRYPT
	    case 'k':	/* -k<key> for code key */
	    case 'K':
		eflag = TRUE;
		(void)Cstrcpy(ekey, Cfromc(&argv[carg][2]));
		break;
#endif
#if	KANJI
	    case 'b':
	    case 'B':
		gmode |= MDBINARY;
		break;
	    case 't': case 'T':
	    case 'f': case 'F':
		gmode &= ~MDBINARY;
		argp = &argv[carg][1];
		cp = (*argp == 't' || *argp == 'T') ? &term.t_code : &kcode;
		for (; *argp; argp++) {
		    switch (*argp) {
		    case 'j': case 'J':
			KS_CODE(*cp) = KS_JIS;
			if (cp == &term.t_code) KS_THRU(*cp) = (*argp == 'J');
			continue;
		    case 'u': case 'U':
		    case 'e': case 'E': /* e/E added by N.Nide */
			KS_CODE(*cp) = KS_UJIS;
			continue;
		    case 's': case 'S':
			KS_CODE(*cp) = KS_SJIS;
			continue;
#if HANDLE_UTF
		    case 'w': case 'W':
			KS_CODE(*cp) = KS_UTF8;
			continue;
#endif
		    }
		     /* EOL type settings are only for file IO;
			not for terminal */
		    if(cp == &kcode) switch(*argp){
		    case 'l': case 'L':
			KS_EOL(kcode) = KS_LF;
			break;
		    case 'c': case 'C':
			KS_EOL(kcode) = KS_CR;
			break;
		    case 'd': case 'D': /* DOS type EOL */
			KS_EOL(kcode) = KS_CRLF;
			break;
		    }
		}
		if (cp == &kcode) {
		    curwp->w_bufp->b_code = kcode;
		}
		else {	/* t_code is changed. re-open terminal */
		    TTkclose();
		    TTkopen();
		}
		break;
#endif /* KANJI */
	    case 'x': case 'X':	/* -x for execute command */
		if (argv[carg][2]) {
		    /* if we haven't run emacs.rc, do it now */
		    if (startf == FALSE) {
			(void)startup(CSTR(&Cxstr[6]));
			startf = TRUE;
#if	COLOR
			curwp->w_fcolor = gfcolor;
			curwp->w_bcolor = gbcolor;
#endif
#if	KANJI
			curwp->w_bufp->b_code = kcode;
#endif
		    }
		    (void)docmd(Cfromc(&argv[carg][2]));
		}
		break;
	    default:	/* unknown switch */
		/* ignore this for now */
		break;
	    }
	} else if (argv[carg][0]== '@') {
	    /* macro file */
	    register Char *p;

	    if (startup(p=SafeCfromc(&argv[carg][1])) == TRUE)
		startf = TRUE;	/* don't execute emacs.rc */
	    Free(p);
	} else {
	regular_arg:
	    /* process a file name */
	    /* if we haven't run emacs.rc, do it now */
	    if (startf == FALSE) {
		(void)startup(CSTR(&Cxstr[6]));
		startf = TRUE;
#if	COLOR
		curwp->w_fcolor = gfcolor;
		curwp->w_bcolor = gbcolor;
#endif
#if	KANJI
		curwp->w_bufp->b_code = kcode;
#endif
	    }

	    /* set up a buffer for this file */
	    makename(bname, Cfromc(argv[carg]));
	    unqname(bname);

#if	CRYPT
	    /* set up for de-cryption if needed */
	    if (eflag) {
		curbp->b_mode |= MDCRYPT;
		(void)copystr(&curbp->b_key, ekey);
		cryptstr((Char *)NULL, (unsigned)0);
		cryptstr(curbp->b_key, (unsigned)Cstrlen(curbp->b_key));
	    }
#endif

	    /* if this is the first file, read it in */
	    if (ffile) {
		register Char *p;

		bp = curbp;
		(void)copystr(&bp->b_bname, bname);
		(void)copystr(&bp->b_fname, Cfromc(argv[carg]));
		if (readin(p=SafeCfromc(argv[carg]), (viewflag==FALSE))
		    == ABORT) {
		    (void)copystr(&bp->b_bname, CSTR(&Cxstr[3314]));
		    (void)copystr(&bp->b_fname, CSTR(&Cxstr[6]));
		}
		Free(p);
		bp->b_dotp = bp->b_linep;
		bp->b_doto = 0;
		ffile = FALSE;
	    } else {
		/* set this to inactive */
		bp = bfind(bname, TRUE, 0);
		(void)copystr(&bp->b_fname, Cfromc(argv[carg]));
		bp->b_active = FALSE;
	    }

	    /* set the view mode appropriatly */
	    if (viewflag)
		bp->b_mode |= MDVIEW;
	}
    }

    /* if invoked with nothing, run the startup file here */
    if (startf == FALSE) {
	(void)startup(CSTR(&Cxstr[6]));
	startf = TRUE;
#if	COLOR
	curwp->w_fcolor = gfcolor;
	curwp->w_bcolor = gbcolor;
#endif
#if	KANJI
	curwp->w_bufp->b_code = kcode;
#endif
    }

    /* Deal with startup gotos and searches */

    if (gotoflag && searchflag) {
	update(FALSE);
	mlwrite(
#if KANJI
	    kterminal? CSTR(&Cxstr[3319]):
#endif
	    CSTR(&Cxstr[3341]));
    } else if (gotoflag) {
	if (gotoline(TRUE,gline) == FALSE) {
	    update(FALSE);
	    mlwrite(
#if KMSGS
		kterminal? CSTR(&Cxstr[3384]):
#endif
		CSTR(&Cxstr[3396]));
	}
    } else if (searchflag) {
	if (forwhunt(FALSE, 0) == FALSE)
	    update(FALSE);
    }

    /* setup to process commands */
    lastflag = 0;				/* Fake last flags.	*/
    curbp->b_mode |= gmode;			/* and set default modes*/
    curwp->w_flag |= WFMODE;			/* and force an update	*/

    (void)recedit(TRUE, 1);			/* recursive edit */
    /*NOTREACHED*/
}

static int exitedit = FALSE;

/*
 * RECEDIT: perform editing recursively.
 * if flag argument specifies whether top level or not.
 */
recedit(toplev, n)
    register bool toplev;
    register int n;
{
    register bool f;
    register Ckey c;
    register int mflag;
    Ckey basec;			/* c stripped of meta character */
    Ckey getcmd();

    reclevel++;
    upmode();
    while (TRUE) {
	do {
	    update(FALSE);				/* Fix up the screen	*/
	    c = getcmd();
#if	CLOCK
	    if (clflag) {
		/* timer signaled */
		(void)execute(META|SPEC|'T', FALSE, 1);
		clflag = 0;
	    }
#endif
	    if (mpresf != FALSE) {
		mlerase();
		update(FALSE);
#if	CLRMSG
		if (c == ' ')			/* ITS EMACS does this	*/
		    continue;
#endif
	    }
	    f = FALSE;
	    n = 1;

	    /* do META-# processing if needed */
	    basec = c & ~META;		/* strip meta char off if there */
	    if ((c & META) && (isDigit(basec) || basec == '-')) {
		f = TRUE;		/* there is a # arg */
		n = 0;			/* start with a zero default */
		mflag = 1;		/* current minus flag */
		c = basec;		/* strip the META */
		while (isDigit(c) || (c == '-')) {
		    if (c == '-') {
			/* already hit a minus or digit? */
			if (mflag < 0 || n > 0)
			    break;
			mflag = -1;
		    } else {
			n = n * 10 + (c - '0');
		    }
		    if ((n == 0) && (mflag == -1)) {	/* lonely - */
			mlwrite(
#if KMSGS
			    kterminal? CSTR(&Cxstr[3418]):
#endif
			    CSTR(&Cxstr[3422]));
		    } else {
			mlwrite(
#if KMSGS
			    kterminal? CSTR(&Cxstr[3427]):
#endif
			    CSTR(&Cxstr[3434]),n * mflag);
		    }

		    c = getcmd();	/* get the next key */
		}
		n = n * mflag;	/* figure in the sign */
	    }

	    /* do ^U repeat argument processing */
	    if (c == reptc) {			/* ^U, start argument	*/
		f = TRUE;
		n = 4;				/* with argument of 4 */
		mflag = 0;			/* that can be discarded. */
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[3442]):
#endif
		    CSTR(&Cxstr[3448]));
		while (c = getcmd(), isDigit(c) || c==reptc || c=='-'){
		    if (c == reptc) {
			if ((n > 0) == ((n*4) > 0)) {
			    /* no overflow */
			    n *= 4;
			} else {
			    /* overflow */
			    n = 1;
			}
		    } else if (c == '-') {
			/*
			 * If dash, and start of argument string, set arg.
			 * to -1.  Otherwise, insert it.
			 */
			if (mflag)
			    break;
			n = 0;
			mflag = -1;
		    } else {
			/*
			 * If first digit entered, replace previous argument
			 * with digit and set sign.  Otherwise, append to arg.
			 */
			if (!mflag) {
			    n = 0;
			    mflag = 1;
			}
			n = 10*n + c - '0';
		    }
		    mlwrite(
#if KMSGS
			kterminal? CSTR(&Cxstr[3427]):
#endif
			CSTR(&Cxstr[3434]), (mflag >=0) ? n : (n ? -n : -1));
		}
		/*
		 * Make arguments preceded by a minus sign negative and change
		 * the special argument "^U -" to an effective "^U -1".
		 */
		if (mflag == -1) {
			if (n == 0)
				n++;
			n = -n;
		}
	    }

	    if (c == abortc)
		    (void)ctrlg(f, n);
	    else
		    /* execute the command */
		    (void)execute(c, f, n);
	} while (!exitedit);
	exitedit = FALSE;
	if (!toplev) break;
	TTbeep();
    }
    reclevel--;
    upmode();
    return(TRUE);
}

/*
 * EXITRECEDIT: exit recursive edit.
 */
/*ARGSUSED*/
exitrecedit(f, n)
{
    exitedit = TRUE;
    return(TRUE);
}

/*
 * Initialize all of the buffers and windows. The buffer name is passed down
 * as an argument, because the main routine may have been told to read in a
 * file by default, and we want the buffer name to be right.
 */
edinit(bname)
	Char	bname[];
{
	register BUFFER *bp;
	register WINDOW *wp;

	bp = bfind(bname, TRUE, 0);		/* First buffer		*/
	wp = (WINDOW *) malloc(sizeof(WINDOW)); /* First window		*/
	if (bp==NULL || wp==NULL)
		exit(1);
	wp->w_id = 0;				/* ID of first window is 0 */
	curbp  = bp;				/* Make this current	*/
	wheadp = wp;
	curwp  = wp;
	wp->w_wndp  = NULL;			/* Initialize window	*/
	wp->w_bufp  = bp;
	bp->b_nwnd  = 1;			/* Displayed.		*/
	wp->w_linep = bp->b_linep;
	wp->w_dotp  = bp->b_linep;
	wp->w_doto  = 0;
	wp->w_markp = NULL;
	wp->w_marko = 0;
	wp->w_toprow = 0;
#if	COLOR
	/* initalize colors to global defaults */
	wp->w_fcolor = gfcolor;
	wp->w_bcolor = gbcolor;
#endif
	wp->w_ntrows = term.t_nrow-1;		/* "-1" for mode line.	*/
	wp->w_force = 0;
	wp->w_flag  = WFMODE|WFHARD;		/* Full.		*/
}

/*
 * This is the general command execution routine. It handles the fake binding
 * of all the keys to "self-insert". It also clears out the "thisflag" word,
 * and arranges to move it to the "lastflag", so that the next command can
 * look at it. Return the status of command.
 */
execute(c, f, n)
	Ckey c;
{
	register KEYTAB *ktp;
	register int	status;

	ktp = &keytab[0];			/* Look in key table.	*/
	while (ktp->k_fp != NULL) {
		if (ktp->k_code == c) {
			thisflag = 0;
			status	 = (*ktp->k_fp)(f, n);
			lastflag = thisflag;
			return (status);
		}
		++ktp;
	}

	if ((curwp->w_bufp->b_mode & MDWRAP) &&
	    !(curwp->w_bufp->b_mode & MDVIEW) &&
	    fillcol > 0 && n >= 0) {
	    /*
	     * If a space was typed, fill column is defined, the argument is non-
	     * negative, wrap mode is enabled, and we are now past fill column,
	     * and we are not read-only, perform word wrap.
	     */
	    if (c == ' ' && getccol(FALSE) > fillcol)
		(void)execute(META|SPEC|'W', FALSE, 1);
#if KANJI
	    /*
	     * wrap around condition for KANJI and KANA
	     */
	    if ((iskanji((Char)c) || iskana((Char)c)) &&
		should_wrap((Char)c, getccol(FALSE))) {
		/* insert newline and indent */
		(void)indent(FALSE, 1);
	    }
#endif /*KANJI*/
	}

	if (!(c & CK_FLAGS)) {
		if (n <= 0) {			/* Fenceposts.		*/
			lastflag = 0;
			return (n<0 ? FALSE : TRUE);
		}
		thisflag = 0;			/* For the future.	*/

		/* if we are in overwrite mode, not at eol,
		   and next char is not a tab or we are at a tab stop,
		   delete a char forword			*/
		if (curwp->w_bufp->b_mode & MDOVER &&
		    curwp->w_doto < curwp->w_dotp->l_used &&
			(lgetc(curwp->w_dotp, curwp->w_doto) != '\t' ||
			 (curwp->w_doto) % 8 == 7))
				(void)ldelete(1L, FALSE);
		/* do the appropriate insertion */
		if (c == '}' && (curbp->b_mode & MDCMOD) != 0)
			status = insbrace(n, (Char)c);
		else if (c == '#' && (curbp->b_mode & MDCMOD) != 0)
			status = inspound();
		else {
			status = linsert(n, (Char)c);
		}
		
#if	CFENCE
		/* check for CMODE fence matching */
		if ((c == '}' || c == ')' || c == ']') &&
				(curbp->b_mode & MDCMOD) != 0)
			fmatch((Char)c);
#endif
#if ASAVE
		/* check auto-save mode */
		if (curbp->b_mode & MDASAVE)
			if (--gacount == 0) {
				/* and save the file if needed */
				(void)upscreen(FALSE, 0);
				(void)filesave(FALSE, 0);
				gacount = gasave;
			}
#endif
		lastflag = thisflag;
		return (status);
	}
	TTbeep();
	mlwrite(
#if KMSGS
	    kterminal? CSTR(&Cxstr[3455]):
#endif
	    CSTR(&Cxstr[623]));
						/* complain		*/
	lastflag = 0;				/* Fake last flags.	*/
	return (FALSE);
}

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit emacs, otherwise simply exit.
 */
quickexit(f, n)
{
	register BUFFER *bp;	/* scanning pointer to buffers */
	register BUFFER *savecur = curbp;

	bp = bheadp;
	while (bp != NULL) {
		if ((bp->b_flag & BFCHG)		/* Changed.	*/
		    && !(bp->b_flag & BFINVS)) {	/* Real.	*/
			curbp = bp;		/* make that buffer cur */
			mlwrite(
#if KMSGS
			    kterminal? CSTR(&Cxstr[3463]):
#endif
			    CSTR(&Cxstr[3473]), bp->b_fname);
			(void)filesave(f, n);
		}
		bp = bp->b_bufp;		/* on to the next buffer */
	}
	curbp = savecur;
	return quit(f, n);			/* conditionally quit	*/
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out. Normally bound to "C-X C-C".
 */
/*ARGSUSED*/
quit(f, n)
{
	register int	s;

	if (f != FALSE				/* Argument forces it.	*/
	|| anycb() == FALSE			/* All buffers clean.	*/
						/* User says it's OK.	*/
	|| (s=mlyesno(
#if KMSGS
		kterminal? CSTR(&Cxstr[3485]):
#endif
		CSTR(&Cxstr[3509])))
									     == TRUE) {
		vttidy();
		exit(0);
	}
	mlwrite(CSTR(&Cxstr[6]));
	return (s);
}

/*
 * Begin a keyboard macro.
 * Error if not at the top level in keyboard processing. Set up variables and
 * return.
 */
/*ARGSUSED*/
ctlxlp(f, n)
{
	if (kbdmode != STOP) {
		mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
			kterminal? CSTR(&Cxstr[3546]):
#endif
			CSTR(&Cxstr[3559]));
		return(FALSE);
	}
	mlwrite(
#if KMSGS
	    kterminal? CSTR(&Cxstr[3581]):
#endif
	    CSTR(&Cxstr[3591]));
	kbdptr = &kbdm[0];
	kbdend = kbdptr;
	kbdmode = RECORD;
	return (TRUE);
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
/*ARGSUSED*/
ctlxrp(f, n)
{
	if (kbdmode == STOP) {
		mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
			kterminal? CSTR(&Cxstr[3605]):
#endif
			CSTR(&Cxstr[3621]));
		return(FALSE);
	}
	if (kbdmode == RECORD) {
		mlwrite(
#if KMSGS
			kterminal? CSTR(&Cxstr[3639]):
#endif
			CSTR(&Cxstr[3649]));
		kbdmode = STOP;
	}
	return(TRUE);
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
/*ARGSUSED*/
ctlxe(f, n)
{
	if (kbdmode != STOP) {
		mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
			kterminal? 
			/**/ CSTR(&Cxstr[3661]):
#endif
			CSTR(&Cxstr[3559]));
		return(FALSE);
	}
	if (n <= 0)
		return (TRUE);
	kbdrep = n;		/* remember how many times to execute */
	kbdmode = PLAY;		/* start us in play mode */
	kbdptr = &kbdm[0];	/*    at the beginning */
	return(TRUE);
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
/*ARGSUSED*/
ctrlg(f, n)
{
	TTbeep();
	kbdmode = STOP;
	mlwrite(
#if KMSGS
		kterminal? CSTR(&Cxstr[22]):
#endif
		CSTR(&Cxstr[32]));
	return(ABORT);
}

/* tell the user that this command is illegal while we are in
   VIEW (read-only) mode				*/

rdonly()
{
	TTbeep();
	mlwrite(
#if KMSGS
	    kterminal? CSTR(&Cxstr[3671]):
#endif
	    CSTR(&Cxstr[3691]));
	return(FALSE);
}

#if	RESTRICT
resterr()
{
	TTbeep();
	mlwrite(
# if KMSGS
		kterminal? CSTR(&Cxstr[3718]):
# endif
		CSTR(&Cxstr[3735]));
	return(FALSE);
}
#endif /* RESTRICT */

meta_pfx() /* dummy function for binding to meta prefix */
{}

cex()	/* dummy function for binding to control-x prefix */
{}

unarg() /* dummy function for binding to universal-argument */
{}

/*
 * routines for dynamic strage management.
 */
/*
 * copy: copy contents pointed by src to newly allocated space
 * and store pointer to it to the cell pointed by dst.
 * if dst contains pointer but NULL, that is freed before.
 * data in src has size size.
 * returns TRUE on success, otherwise FALSE.
 */
int
copy(dst, src, size)
	char **dst, *src;
	int size;
{
	register char *p;

	if (size <= 0 ||
	    (p = malloc((unsigned)size)) == NULL) return FALSE;
	memcpy(p, src, size);
	if (*dst) free(*dst);
	*dst = p;
	return TRUE;
}

/*
 * copystr: copy string.
 */
copystr(dst, src)
	register Char **dst, *src;
{
	return copy((char **)dst, (char *)src, sizeof(Char)*(Cstrlen(src)+1));
}

#if	BSD
# include	<sys/time.h>
#endif
#if	USG
# include	<time.h>
#endif

/*
 * getclock: return string containing current time.
 */
Char *
getclock()
{
#if HAVE_TIME_T
	time_t t;
#else
	long t;
#endif
	struct tm *tmp;
	static Char buf[8];

	t = time(NULL);
	tmp = localtime(&t);
	ltoa(buf, 2, (long)tmp->tm_hour % 12);
	if (buf[0] == ' ') buf[0] = '0';
	buf[2] = ':';
	ltoa(buf+3, 2, (long)tmp->tm_min);
	if (buf[3] == ' ') buf[3] = '0';
	(void)Cstrcpy(buf+5, (tmp->tm_hour >= 12)? CSTR(&Cxstr[3764]): CSTR(&Cxstr[3767]));
	return buf;
}

#if	CLOCK
/*
 * ALRM signal catcher
 */
/* ARGSUSED */
SIGRET_T
clcatcher(SIGARG_T(dummy))
{
    (void)signal(SIGALRM, SIG_IGN);
    if (clflag) {
	int ttcsave = ttcol, ttrsave = ttrow;

	/* nothing done since last signal. do myself. */
	(void)execute(META|SPEC|'T', FALSE, 1);
	do_update(FALSE);
	movecursor(ttrsave, ttcsave);
	TTflush();
    }
    clflag = 1;
    /* re-initialize timer */
    clinit();
}

/*
 * initialize timer
 */
clinit()
{
    (void)signal(SIGALRM, clcatcher);
    (void)alarm((unsigned)cperiod);
}
#endif /* CLOCK */

#if	USE_LANG
# if USE_STRNICMP
#  define strncasecmp strnicmp
#  define strcasestr stristr
# endif
langcode(defaul) int defaul;
{
	char *lang = getenv("LANG");
	if(lang == NULL || strncasecmp(lang, "ja", 2) != 0) return defaul;
	lang += 2;
# if HANDLE_UTF
	if(strcasestr(lang, "utf8") || strcasestr(lang, "utf-8"))
		return KS_UTF8;
# endif
	if(strcasestr(lang, "mscode") || strcasestr(lang, "sjis"))
		return KS_SJIS;
	if(strcasestr(lang, ".jis") || strcasestr(lang, "iso"))
		return KS_JIS;
	return KS_UJIS;
}
#endif	/* USE_LANG */
