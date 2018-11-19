#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*	This file is for functions dealing with execution of
	commands, command lines, buffers, files and startup files

	written 1986 by Daniel Lawrence				*/

#include	"ecomm.h"
#include	"estruct.h"
#include	"edef.h"

#if	DEBUGM
static Char outline[NSTRING];	/* global string to hold debug line text */
#endif

/* namedcmd:	execute a named command even if it is not bound
*/

namedcmd(f, n)
{
	register int (*kfunc)(); /* ptr to the requested function to bind to */
	int (*getname())();
	int nullfunc();

	/* prompt the user to type a named command */
	mlwrite(CSTR(&Cxstr[4]));

	/* and now get the function name to execute */
	kfunc = getname();
	if (kfunc == nullfunc) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[1768]):
#endif
		    CSTR(&Cxstr[547]));
		return(FALSE);
	} else if (kfunc == NULL) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[22]):
#endif
		    CSTR(&Cxstr[32]));
		return(ABORT);
	}

	/* and then execute the command */
	return((*kfunc)(f, n));
}

/*	execcmd:	Execute a command line command to be typed in
			by the user					*/

/*ARGSUSED*/
execcmd(f, n)
{
	register int status;		/* status return */
	Char cmdstr[NSTRING];		/* string holding command to execute */

	/* get the line wanted */
	if ((status = mlreply(CSTR(&Cxstr[4]),
					  cmdstr, NSTRING)) != TRUE)
		return(status);

	execlevel = 0;
	return(docmd(cmdstr));
}

/*	docmd:	take a passed string as a command line and translate
		it to be executed as a command. This function will be
		used by execute-command-line and by all source and
		startup files. Lastflag/thisflag is also updated.

	format of the command line is:

		{# arg} <command-name> {<argument string(s)>}

	Directives start with a "!" and include:

	!endm		End a macro
	!if (cond)	conditional execution
	!else
	!endif
	!return		Return (terminating current macro)
	!goto <label>	Jump to a label in the current macro

	Line Labels begin with a "*" in column 1, like:

	*LBL01

	if COMPAT option is enabled, old [endm] notation also accepted.
*/

docmd(cline)
	Char *cline;	/* command line to execute */
{
	register int f;		/* default argument flag */
	register int n;		/* numeric repeat value */
	register int i;
	int (*fnc)();		/* function to execute */
	int status;		/* return status of function */
	int oldcle;		/* old contents of clexec flag */
	int llen;		/* length of cline */
	int force;		/* force TRUE result? */
	Char *tmp;		/* tmp pointer into cline */
	struct LINE *lp;	/* a line pointer */
	Char *oldestr;		/* original exec string */
	Char tok[NSTRING];	/* next token off of command line */
	int (*fncmatch())();

#if	DEBUGM
	/* if $debug == TRUE, every line to execute
	   gets echoed and a key needs to be pressed to continue
	   ^G will abort the command */

	if (macbug) {
		int odiscmd = discmd;	/* original value of discmd */

		discmd = TRUE;
		(void)Cstrcpy(outline, CSTR(&Cxstr[1779]));
		(void)Cstrcat(outline, odiscmd == TRUE? CSTR(&Cxstr[1783]): CSTR(&Cxstr[1785]));
		(void)Cstrcat(outline, cline);
		(void)Cstrcat(outline, CSTR(&Cxstr[1787]));

		/* write out the debug line */
		mlwrite(CSTR(&Cxstr[6])); /* make sure line is clear */
		mlwrite(CSTR(&Cxstr[1281]), outline);
		update(TRUE);

		discmd = odiscmd;

		/* and get the keystroke */
		if (tgetc() == ectoc(abortc)) {
			mlwrite(
#if KMSGS
				kterminal? CSTR(&Cxstr[1791]):
#endif
				CSTR(&Cxstr[1805]));
			return(FALSE);
		}
	}
#endif /*DEBUGM*/

	/* dump comments here */
	if (*cline == ';')
		return(TRUE);

	/* eat leading spaces */
	while (*cline == ' ' || *cline == '\t')
		++cline;

	/* check to see if this line turns macro storage off */
	if (cline[0] == '!' && Cstrncmp(&cline[1], CSTR(&Cxstr[1821]), 4) == 0
#if	COMPAT
	    || Cstrncmp(cline, CSTR(&Cxstr[1826]), 5) == 0
# if	KANJI
	    || Cstrncmp(cline, CSTR(&Cxstr[1832]), 4) == 0
# endif
#endif	/*COMPAT*/
		) {
		mstore = FALSE;
		bstore = NULL;
		return(TRUE);
	}

	/* if macro store is on, just salt this away */
	if (mstore) {
		/* allocate the space for the line */
		llen = Cstrlen(cline);
		if ((lp=lalloc(llen)) == NULL) {
			mlwrite(
#if KMSGS
			    kterminal? CSTR(&Cxstr[1837]):
#endif
			    CSTR(&Cxstr[1854]));
			return (FALSE);
		}

		/* copy the text into the new line */
		for (i=0; i<llen; ++i)
			lputc(lp, i, cline[i]);

		/* attach the line to the end of the buffer */
		bstore->b_linep->l_bp->l_fp = lp;
		lp->l_bp = bstore->b_linep->l_bp;
		bstore->b_linep->l_bp = lp;
		lp->l_fp = bstore->b_linep;
		return (TRUE);
	}

	/* dump labels here */
	if (*cline == '*')
		return(TRUE);

	force = FALSE;
	oldestr = execstr;	/* save last ptr to string to execute */
	execstr = cline;	/* and set this one as current */

	/* process directives */
	if (*cline == '!') {
		/* save directive location and skip it */
		tmp = cline;
		while (*execstr && *execstr != ' ' && *execstr != '\t')
			++execstr;

		if (tmp[1] == 'f' && tmp[2] == 'o') {
			force = TRUE;
			goto do001;

		} else if (tmp[1] == 'i' && tmp[2] == 'f') {

			/* IF directive */
			/* grab the value of the logical exp */
			if (execlevel == 0) {
				if ((status = macarg(tok)) != TRUE) {
					execstr = oldestr;
					return(status);
				}
				status = stol(tok);
			} else
				status = TRUE;

			if (status) {

				/* IF (TRUE) */
				if (execlevel != 0)
					++execlevel;
			} else {

				/* IF (FALSE) */
				++execlevel;
			}

		} else if (tmp[1] == 'e' && tmp[2] == 'l') {

			/* ELSE directive */
			if (execlevel == 1)
				--execlevel;
			else if (execlevel == 0 )
				++execlevel;

		} else if (tmp[1] == 'e' && tmp[2] == 'n') {

			/* ENDIF directive */
			if (execlevel)
				--execlevel;

		} else if (tmp[1] == 'r' && tmp[2] == 'e') {

			/* RETURN directive */
			execstr = oldestr;
			return(execlevel? TRUE: RET);

		} else if (tmp[1] == 'g' && tmp[2] == 'o') {

			/* GOTO directive */
			/* .....only if we are currently executing */
			if (execlevel) {
				execstr = oldestr;
				return(TRUE);
			}

			while (*execstr == ' ' || *execstr == '\t')
				++execstr;
			(void)Cstrncpy(golabel, execstr, NPAT - 1);
			return(GOLINE);

		} else {
			mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
				kterminal? CSTR(&Cxstr[1888]):
#endif
				CSTR(&Cxstr[1901]));
			return(FALSE);
		}

		/* restore execstr and exit */
		execstr = oldestr;
		return(TRUE);
	}

    do001:
	/* if we are scanning and not executing..go back here */
	if (execlevel) {
		execstr = oldestr;
		return(TRUE);
	}

	/* first set up the default command values */
	f = FALSE;
	n = 1;

	if ((status = macarg(tok)) != TRUE) {	/* and grab the first token */
		execstr = oldestr;
		return(status);
	}

	/* process leadin argument */
	if (gettyp(tok) != TKCMD) {
		f = TRUE;
		(void)Cstrcpy(tok, getval(tok));
		n = stoi(tok);

		/* and now get the command to execute */
		if ((status = macarg(tok)) != TRUE) {
			execstr = oldestr;
			return(status);	
		}	
	}

	/* and match the token to see if it exists */
	if ((fnc = fncmatch(tok)) == NULL) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[1768]):
#endif
		    CSTR(&Cxstr[547]));
		execstr = oldestr;
		return(FALSE);
	}
	
	/* save the arguments and go execute the command */
	oldcle = clexec;		/* save old clexec flag */
	clexec = TRUE;			/* in cline execution */
	thisflag = 0;
	status = (*fnc)(f, n);		/* call the function */
	lastflag = thisflag;
	cmdstatus = status;		/* save the status */
	if (force)			/* force the status */
		status = TRUE;
	clexec = oldcle;		/* restore clexec flag */
	execstr = oldestr;
	return(status);
}

/* token:	chop a token off a string
		return a pointer past the token
*/

Char *
token(src, tok)
	Char *src, *tok;	/* source string, destination token string */
{
	register int quotef;	/* is the current string quoted? */

	/* first scan past any whitespace in the source string */
	while (*src == ' ' || *src == '\t')
		++src;

	/* scan through the source string */
	quotef = FALSE;
	while (*src) {
		/* process special characters */
		if (*src == '~') {
			++src;
			if (*src == 0)
				break;
			switch (*src++) {
				case 'r':	*tok++ = 13; break;
				case 'n':	*tok++ = 10; break;
				case 't':	*tok++ = 9;  break;
				case 'b':	*tok++ = 8;  break;
				case 'f':	*tok++ = 12; break;
				default:	*tok++ = *(src-1);
			}
		} else {
			/* check for the end of the token */
			if (quotef) {
				if (*src == '"')
					break;
			} else {
				if (*src == ' ' || *src == '\t')
					break;
			}

			/* set quote mode if qoute found */
			if (*src == '"')
				quotef = TRUE;

			/* record the character */
			*tok++ = *src++;
		}
	}

	/* terminate the token and exit */
	if (*src)
		++src;
	*tok = 0;
	return(src);
}

macarg(tok)	/* get a macro line argument */
	Char *tok;	/* buffer to place argument */
{
	int savcle;	/* buffer to store original clexec */
	int status;

	savcle = clexec;	/* save execution mode */
	clexec = TRUE;		/* get the argument */
	status = nextarg(CSTR(&Cxstr[6]),
			tok, NSTRING, ctoec('\n'));
	clexec = savcle;	/* restore execution mode */
	return(status);
}

/*	nextarg:	get the next argument	*/

nextarg(prompt, buffer, size, terminator)
	Char *prompt;	/* prompt to use if we must be interactive */
	Char *buffer;	/* buffer to put token into */
	int size;	/* size of the buffer */
	Ckey terminator; /* terminating char to be used on interactive fetch */
{
	/* if we are interactive, go get it! */
	if (clexec == FALSE)
		return(getstring(prompt, buffer, size, terminator));

	/* grab token and advance past */
	execstr = token(execstr, buffer);

	/* evaluate it */
	(void)Cstrcpy(buffer, getval(buffer));
	return(TRUE);
}

/*	storemac:	Set up a macro buffer and flag to store all
			executed command lines there			*/

storemac(f, n)
{
	register struct BUFFER *bp;	/* pointer to macro buffer */
	Char bname[NBUFN];		/* name of buffer to use */

	/* must have a numeric argument to this function */
	if (f == FALSE) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[1920]):
#endif
		    CSTR(&Cxstr[1936]));
		return(FALSE);
	}

	/* range check the macro number */
	if (n < 1 || n > 40) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[1955]):
#endif
		    CSTR(&Cxstr[1967]));
		return(FALSE);
	}

	/* construct the macro buffer name */
	(void)Cstrcpy(bname, CSTR(&Cxstr[1993]));
	bname[2] = '0' + (n / 10);
	bname[3] = '0' + (n % 10);

	/* set up the new macro buffer */
	if ((bp = bfind(bname, TRUE, BFINVS)) == NULL) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[2000]):
#endif
		    CSTR(&Cxstr[2012]));
		return(FALSE);
	}

	/* and make sure it is empty */
	(void)bclear(bp);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return(TRUE);
}

#if	PROC
/*	storeproc:	Set up a procedure buffer and flag to store all
			executed command lines there			*/

storeproc(f, n)
     int f;		/* default flag */
     int n;		/* macro number to use */
{
	register struct BUFFER *bp;	/* pointer to macro buffer */
	register int status;		/* return status */
	Char bname[NBUFN];		/* name of buffer to use */

	/* a numeric argument means its a numbered macro */
	if (f == TRUE)
		return(storemac(f, n));

	/* get the name of the procedure */
	if ((status = mlreply(
#if KMSGS
			      kterminal? CSTR(&Cxstr[2033]):
#endif
			      CSTR(&Cxstr[2040]),
			      &bname[2], NBUFN-3)) != TRUE)
	  return(status);

	/* construct the macro buffer name */
	bname[0] = bname[1] = '[';
	(void)Cstrcat(bname, CSTR(&Cxstr[1997]));

	/* set up the new macro buffer */
	if ((bp = bfind(bname, TRUE, BFINVS)) == NULL) {
		mlwrite(
#if KMSGS
			kterminal? CSTR(&Cxstr[2000]):
#endif
			CSTR(&Cxstr[2012]));
		return(FALSE);
	}

	/* and make sure it is empty */
	(void)bclear(bp);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return(TRUE);
}

/*	execproc:	Execute a procedure				*/
/*ARGSUSED*/
execproc(f, n)
{
  register BUFFER *bp;		/* ptr to buffer to execute */
  register int status;		/* status return */
  Char bufn[NBUFN+2];		/* name of buffer to execute */

  /* find out what buffer the user wants to execute */
  if ((status = mlreply(
#if KMSGS
			kterminal? CSTR(&Cxstr[2057]):
#endif
			CSTR(&Cxstr[2067]),
			&bufn[2], NBUFN-3)) != TRUE)
    return(status);

  /* construct the buffer name */
  bufn[0] =bufn[1] = '[';
  (void)Cstrcat(bufn, CSTR(&Cxstr[1997]));

  /* find the pointer to that buffer */
  if ((bp=bfind(bufn, FALSE, 0)) == NULL) {
    mlwrite(
#if KMSGS
	    kterminal? CSTR(&Cxstr[2087]):
#endif
	    CSTR(&Cxstr[2099]));
    return(FALSE);
  }

  /* and now execute it as asked */
  while (n-- > 0)
    if ((status = dobuf(bp)) != TRUE)
      return(status);
  return(TRUE);
}
#endif /* PROC */

/*	execbuf:	Execute the contents of a buffer of commands	*/

/*ARGSUSED*/
execbuf(f, n)
{
	register BUFFER *bp;		/* ptr to buffer to execute */
	register int status;		/* status return */
	Char bufn[NBUFN];		/* name of buffer to execute */

	/* find out what buffer the user wants to execute */
	if ((status = mlreply(
#if KMSGS
			kterminal? CSTR(&Cxstr[2117]):
#endif
			CSTR(&Cxstr[2129]),
							bufn, NBUFN)) != TRUE)
		return(status);

	/* find the pointer to that buffer */
	if ((bp=bfind(bufn, FALSE, 0)) == NULL) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[2146]):
#endif
		    CSTR(&Cxstr[2157]));
		return(FALSE);
	}

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return(status);
	return(TRUE);
}

/*	dobuf:	execute the contents of the buffer pointed to
		by the passed BP				*/

dobuf(bp)
	BUFFER *bp;	/* buffer to execute */
{
	register int status;		/* status return */
	register LINE *lp;		/* pointer to line to execute */
	register LINE *hlp;		/* pointer to line header */
	register int linlen;		/* length of line to execute */
	register WINDOW *wp;		/* ptr to windows to scan */
	Char *eline;			/* text of line to execute */

	/* clear IF level flags */
	execlevel = 0;

	/* starting at the beginning of the buffer */
	hlp = bp->b_linep;
	lp = hlp->l_fp;
	while (lp != hlp) {
		/* allocate eline and copy macro line to it */
		linlen = lp->l_used;
		if ((eline = (Char *)malloc((unsigned)sizeof(Char)*(linlen+1))) == NULL) {
			mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
				kterminal? CSTR(&Cxstr[2172]):
#endif
				CSTR(&Cxstr[2191]));
			return(FALSE);
		}
		(void)Cstrncpy(eline, lp->l_text, linlen);
		eline[linlen] = 0;	/* make sure it ends */

		/* trim leading whitespace */
		while (eline[0] == ' ' || eline[0] == '\t')
			(void)Cstrcpy(eline, &eline[1]);

		/* if it is not a comment, execute it */
		if (eline[0] != 0 && eline[0] != ';') {
			status = docmd(eline);

			/* if it is a !GOTO directive, deal with it */
			if (status == GOLINE) {
			    register LINE *glp;	/* line to goto */

			    linlen = Cstrlen(golabel);
			    glp = hlp->l_fp;
			    for (glp = hlp->l_fp; glp != hlp; glp = glp->l_fp) {
				register Char *l = glp->l_text;

				while (*l == ' ' || *l == '\t')
				    l++;
				if (*l == '*' &&
				    Cstrncmp(golabel, ++l, linlen) == 0 &&
				    (glp->l_used == linlen+1 ||
				     *l == ' ' || *l == '\t')) {
				    /* label found */
				    lp = glp;
				    status = TRUE;
				    break;
				}
			    }
			}

			if (status == GOLINE) {
			    mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
				kterminal? CSTR(&Cxstr[2229]):
#endif
				CSTR(&Cxstr[2245]));
			    return(FALSE);
			}

			/* if it is a !RETURN directive...do so */
			if (status == RET) {
				free((char *)eline);
				break;
			}

			/* check for a command error */
			if (status != TRUE) {
				/* look if buffer is showing */
				wp = wheadp;
				while (wp != NULL) {
					if (wp->w_bufp == bp) {
						/* and point it */
						wp->w_dotp = lp;
						wp->w_doto = 0;
						wp->w_flag |= WFHARD;
					}
					wp = wp->w_wndp;
				}
				/* in any case set the buffer . */
				bp->b_dotp = lp;
				bp->b_doto = 0;
				free((char *)eline);
				execlevel = 0;
				return(status);
			}
		}

		/* on to the next line */
		free((char *)eline);
		lp = lp->l_fp;
	}

	/* exit the current function */
	execlevel = 0;
	return(TRUE);
}

/*ARGSUSED*/
execfile(f, n)	/* execute a series of commands in a file */
{
	register int status;	/* return status of name query */
	Char fname[NSTRING];	/* name of file to execute */

	if ((status = mlreply(
#if KMSGS
			kterminal? CSTR(&Cxstr[2260]):
#endif
			CSTR(&Cxstr[2272]),
							 fname, NSTRING -1)) != TRUE)
		return(status);

	/* otherwise, execute it */
	while (n-- > 0)
		if ((status=dofile(fname)) != TRUE)
			return(status);

	return(TRUE);
}

/*	dofile: yank a file into a buffer and execute it
		if there are no errors, delete the buffer on exit */

dofile(fname)
	Char *fname;	/* file name to execute */
{
	register BUFFER *bp;	/* buffer to place file to exeute */
	register BUFFER *cb;	/* temp to hold current buf while we read */
	register int status;	/* results of various calls */
	Char bname[NBUFN];	/* name of buffer */

	makename(bname, fname);		/* derive the name of the buffer */
	unqname(bname);			/* make it unique */
	if ((bp = bfind(bname, TRUE, 0)) == NULL) /* get the needed buffer */
		return(FALSE);

	bp->b_mode = MDVIEW;	/* mark the buffer as read only */
	cb = curbp;		/* save the old buffer */
	curbp = bp;		/* make this one current */
	/* and try to read in the file to execute */
	if ((status = readin(fname, FALSE)) != TRUE) {
		curbp = cb;	/* restore the current buffer */
		return(status);
	}

	/* go execute it! */
	curbp = cb;		/* restore the current buffer */
	if ((status = dobuf(bp)) != TRUE)
		return(status);

	/* if not displayed, remove the now unneeded buffer and exit */
	if (bp->b_nwnd == 0)
		(void)zotbuf(bp);
	return(TRUE);
}

/*	cbuf:	Execute the contents of a numbered buffer	*/

/*ARGSUSED*/
cbuf(f, n, bufnum)
	int bufnum;	/* number of buffer to execute */
{
	register BUFFER *bp;		/* ptr to buffer to execute */
	register int status;		/* status return */
	Char bufname[NBUFN];

	/* make the buffer name */
	(void)Cstrcpy(bufname, CSTR(&Cxstr[1993]));
	bufname[2] = '0' + (bufnum / 10);
	bufname[3] = '0' + (bufnum % 10);

	/* find the pointer to that buffer */
	if ((bp=bfind(bufname, FALSE, 0)) == NULL) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[2290]):
#endif
		    CSTR(&Cxstr[2304]));
		return(FALSE);
	}

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return(status);
	return(TRUE);
}

cbufn(f, n)
{
	Char buf[NSTRING];
	/*
	 * get the number of macro to execute
	 */
	if (mlreply(
#if KMSGS
		    kterminal? CSTR(&Cxstr[2322]):
#endif
		    CSTR(&Cxstr[2331]),
		    buf, NSTRING) != TRUE) {
		return(FALSE);
	}
	return cbuf(f, n, stoi(buf));
}

cbuf1(f, n)
{
	return cbuf(f, n, 1);
}

cbuf2(f, n)
{
	return cbuf(f, n, 2);
}

cbuf3(f, n)
{
	return cbuf(f, n, 3);
}

cbuf4(f, n)
{
	return cbuf(f, n, 4);
}

cbuf5(f, n)
{
	return cbuf(f, n, 5);
}

cbuf6(f, n)
{
	return cbuf(f, n, 6);
}

cbuf7(f, n)
{
	return cbuf(f, n, 7);
}

cbuf8(f, n)
{
	return cbuf(f, n, 8);
}

cbuf9(f, n)
{
	return cbuf(f, n, 9);
}

cbuf10(f, n)
{
	return cbuf(f, n, 10);
}

cbuf11(f, n)
{
	return cbuf(f, n, 11);
}

cbuf12(f, n)
{
	return cbuf(f, n, 12);
}

cbuf13(f, n)
{
	return cbuf(f, n, 13);
}

cbuf14(f, n)
{
	return cbuf(f, n, 14);
}

cbuf15(f, n)
{
	return cbuf(f, n, 15);
}

cbuf16(f, n)
{
	return cbuf(f, n, 16);
}

cbuf17(f, n)
{
	return cbuf(f, n, 17);
}

cbuf18(f, n)
{
	return cbuf(f, n, 18);
}

cbuf19(f, n)
{
	return cbuf(f, n, 19);
}

cbuf20(f, n)
{
	return cbuf(f, n, 20);
}

cbuf21(f, n)
{
	return cbuf(f, n, 21);
}

cbuf22(f, n)
{
	return cbuf(f, n, 22);
}

cbuf23(f, n)
{
	return cbuf(f, n, 23);
}

cbuf24(f, n)
{
	return cbuf(f, n, 24);
}

cbuf25(f, n)
{
	return cbuf(f, n, 25);
}

cbuf26(f, n)
{
	return cbuf(f, n, 26);
}

cbuf27(f, n)
{
	return cbuf(f, n, 27);
}

cbuf28(f, n)
{
	return cbuf(f, n, 28);
}

cbuf29(f, n)
{
	return cbuf(f, n, 29);
}

cbuf30(f, n)
{
	return cbuf(f, n, 30);
}

cbuf31(f, n)
{
	return cbuf(f, n, 31);
}

cbuf32(f, n)
{
	return cbuf(f, n, 32);
}

cbuf33(f, n)
{
	return cbuf(f, n, 33);
}

cbuf34(f, n)
{
	return cbuf(f, n, 34);
}

cbuf35(f, n)
{
	return cbuf(f, n, 35);
}

cbuf36(f, n)
{
	return cbuf(f, n, 36);
}

cbuf37(f, n)
{
	return cbuf(f, n, 37);
}

cbuf38(f, n)
{
	return cbuf(f, n, 38);
}

cbuf39(f, n)
{
	return cbuf(f, n, 39);
}

cbuf40(f, n)
{
	return cbuf(f, n, 40);
}
