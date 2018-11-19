#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*	INPUT:	Various input routines for MicroEMACS 3.8
		written by Daniel Lawrence
		5/9/86						*/

#include	"ecomm.h"
#include	"estruct.h"
#include	"edef.h"

#ifndef iscntrl
# define iscntrl(c)	((c) <= ' ' || (c) == 0x7f)
#endif

static int prntchar(), killchar();

/*
 * Ask a yes or no question in the message line. Return either TRUE, FALSE, or
 * ABORT. The ABORT status is returned if the user bumps out of the question
 * with a ^G. Used any time a confirmation is required.
 */

mlyesno(prompt)
	Char *prompt;
{
	Char c;			/* input character */
	Char buf[NPAT];		/* prompt to user */

	for (;;) {
		/* build and prompt the user */
		(void)Cstrcpy(buf, prompt);
		(void)Cstrcat(buf, CSTR(&Cxstr[3090]));
		mlwrite(CSTR(&Cxstr[1281]), buf);

		/* get the responce */
		c = tgetc();

		if (c == ectoc(abortc))		/* Bail out! */
			return(ABORT);

		if (c=='y' || c=='Y')
			return(TRUE);

		if (c=='n' || c=='N')
			return(FALSE);
	}
}

/*
 * Write a prompt into the message line, then read back a response. Keep
 * track of the physical position of the cursor. If we are in a keyboard
 * macro throw the prompt away, and return the remembered response. This
 * lets macros run at full speed. The reply is always terminated by a carriage
 * return. Handle erase, kill, and abort keys.
 */
mlreply(prompt, buf, nbuf)
    Char *prompt;
    Char *buf;
{
	return(nextarg(prompt, buf, nbuf, ctoec('\n')));
}

mlreplyt(prompt, buf, nbuf, eolchar)
	Char *prompt;
	Char *buf;
	Ckey eolchar;
{
	return(nextarg(prompt, buf, nbuf, eolchar));
}

/*	ectoc:	expanded character to character
		colapse the CTLFLG and SPEC flags back into an ascii code   */

Char
ectoc(c)
	Ckey c;
{
	if (c & CTLFLG)
		c = c & ~(CTLFLG | 0x40);
	if (c & SPEC)
		c= c & ~SPEC;
	return((Char)c);
}

/*	ctoec:	character to extended character
		pull out the CTLFLG and SPEC prefixes (if possible)	*/
Ckey
ctoec(c)
	Char c;
{
        if (c>=0x00 && c<=0x1F)
                return ((Ckey)(c | CTLFLG | 0x40));
        return ((Ckey)c);
}

/* get a command name from the command line. Command completion means
   that pressing a <SPACE> will attempt to complete an unfinished command
   name if it is unique.
*/
int (*getname())()
{
	register int cpos;	/* current column on screen output */
	register Char c;
	register Char *sp;	/* pointer to string for output */
	register NBIND *ffp;	/* first ptr to entry in name binding table */
	register NBIND *cffp;	/* current ptr to entry in name binding table */
	register NBIND *lffp;	/* last ptr to entry in name binding table */
	Char buf[NSTRING];	/* buffer to hold tentative command name */
	int (*fncmatch())();

	/* starting at the beginning of the string buffer */
	cpos = 0;

	/* if we are executing a command line get the next arg and match it */
	if (clexec) {
		if (macarg(buf) != TRUE)
			return(FALSE);
		return(fncmatch(buf));
	}

	/* build a name string from the keyboard */
	while (TRUE) {
		c = tgetc();

		/* if we are at the end, just match it */
		if (c == '\r') {
			buf[cpos] = 0;

			/* and match it off */
			return(fncmatch(&buf[0]));

		} else if (c == ectoc(abortc)) {	/* Bell, abort */
			(void)ctrlg(FALSE, 0);
			TTflush();
			return( (int (*)()) NULL);

		} else if (c == 0x7F || c == '\b') {	/* rubout/erase */
			if (cpos > 0) {
				ttcol -= killchar(buf[--cpos]);
				TTflush();
			}

		} else if (c == 0x15) { /* C-U, kill */
			while (cpos > 0)
				ttcol -= killchar(buf[--cpos]);
			TTflush();
		} else if (c == ' ') {
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
	/* attempt a completion */
	buf[cpos] = 0;		/* terminate it for us */
	ffp = &names[0];	/* scan for matches */
	while (ffp->n_func != NULL) {
		if (Cstrncmp(buf, ffp->n_name, Cstrlen(buf)) == 0) {
			/* a possible match! More than one? */
			if ((ffp + 1)->n_func == NULL ||
			   (Cstrncmp(buf, (ffp+1)->n_name, Cstrlen(buf)) != 0)) {
				/* no...we match, print it */
				sp = ffp->n_name + cpos;
				while (*sp)
					TTputc(*sp++);
				TTflush();
				return(ffp->n_func);
			} else {
/* << << << << << << << << << << << << << << << << << */
	/* try for a partial match against the list */

	/* first scan down until we no longer match the current input */
	lffp = (ffp + 1);
	while ((lffp+1)->n_func != NULL) {
		if (Cstrncmp(buf, (lffp+1)->n_name, Cstrlen(buf)) != 0)
			break;
		++lffp;
	}

	/* and now, attempt to partial complete the string, char at a time */
	while (TRUE) {
		/* add the next char in */
		buf[cpos] = ffp->n_name[cpos];

		/* scan through the candidates */
		cffp = ffp + 1;
		while (cffp <= lffp) {
			if (cffp->n_name[cpos] != buf[cpos])
				goto onward;
			++cffp;
		}

		/* add the character */
		TTputc(buf[cpos++]);
	}
/* << << << << << << << << << << << << << << << << << */
			}
		}
		++ffp;
	}

	/* no match.....beep and onward */
	TTbeep();
onward:;
	TTflush();
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
		} else {
			if (cpos < NSTRING-1 && c > ' ')
				ttcol += prntchar(buf[cpos++] = c);
			TTflush();
		}
	}
}

/*	tgetc:	Get a key from the terminal driver, resolve any keyboard
		macro action					*/

Char
tgetc()
{
	int c;	/* fetched character */

	/* if we are playing a keyboard macro back, */
	if (kbdmode == PLAY) {

		/* if there is some left... */
		if (kbdptr < kbdend)
			return(*kbdptr++);

		/* at the end of last repitition? */
		if (--kbdrep < 1) {
			kbdmode = STOP;
#if	VISMAC == 0
			/* force a screen update after all is done */
			update(FALSE);
#endif
		} else {

			/* reset the macro to the begining for the next rep */
			kbdptr = &kbdm[0];
			return(*kbdptr++);
		}
	}

	/* fetch a character from the terminal driver */
	c = TTgetc();

	/* record it for $lastkey */
	lastkey = c;

	/* save it if we need to */
	if (kbdmode == RECORD) {
		*kbdptr++ = c;
		kbdend = kbdptr;

		/* don't overrun the buffer */
		if (kbdptr == &kbdm[NKBDM - 1]) {
			kbdmode = STOP;
			TTbeep();
		}
	}

	/* and finally give the char back */
	return(c);
}

/*
 *	GET1KEY:	Get one keystroke. The only prefix legal here
 *			is the CTLFLG prefix.
 */

Ckey
get1key()
{
	Ckey	c;

	/* get a keystroke */
        c = tgetc();

	if (c < ' ')		     /* C0 control -> C-     */
		c |= CTLFLG | 0x40;
	return (c);
}

/*
 *	GETCMD: Get a command from the keyboard. Process all applicable
 *		prefix keys
 */
Ckey
getcmd()
{
	register Ckey c;		/* fetched keystroke */
	register Ckey m;		/* prefix mask */


	/* get initial character */
	c = get1key();

	/* process META prefix */
	if (c == metac) {
		m = META;
		if ((c = get1key()) == abortc) return(c);
	        if (isLower(c))		/* Force to upper */
        	        c ^= DIFCASE;
		if (c=='O') {			/* Escape O */
			if ((c = get1key()) == abortc) return(c);
			m |= SPEC;
		}
		if (c < ' ') {		/* control key */
			c += '@';
			m |= CTLFLG;
		}
		return (m | c);
	}

	/* process CTLX prefix */
	if (c == ctlxc) {
		m = CTLX;
		if ((c = get1key()) == abortc) return(c);
	        if (isLower(c))		/* Force to upper */
        	        c ^= DIFCASE;
		if (c < ' ') {		/* control key */
			c += '@';
			m |= CTLFLG;
		}
		return(m | c);
	}

	/* otherwise, just return it */
	return(c);
}

/*
 *	A more generalized prompt/reply function allowing the caller
 *	to specify the proper terminator. If the terminator is not
 *	a return ('\n') it will echo as "<NL>"
 */
getstring(prompt, buf, nbuf, eolchar)
	Char *prompt;
	Char *buf;
	Ckey eolchar;
{
	register int cpos;	/* current character position in string */
	register Ckey c;
	register int quotef;	/* are we quoting the next char? */

	cpos = 0;
	quotef = FALSE;

	/* prompt the user for the input string */
	mlwrite(CSTR(&Cxstr[1281]), prompt);

	for (;;) {
		/* get a character from the user */
		c = get1key();

		if (!quotef) {
			/* If it is a <ret>, change it to a <NL> */
			if (c == (CTLFLG | 'M'))
				c = CTLFLG | 0x40 | '\n';

			/* if they hit the line terminate, wrap it up */
			if (c == eolchar) {
				buf[cpos++] = 0;
				TTputc('\r');
				TTflush();
				/* if we default the buffer, return FALSE */
				if (buf[0] == 0)
					return(FALSE);
				return(TRUE);
			}

			/* Abort the input? */
			if (c == abortc) {
				(void)ctrlg(FALSE, 0);
				TTflush();
				return(ABORT);
			}

			if (c==0x7F || c== (CTLFLG | 'H')) {
				/* rubout/erase */
				if (cpos > 0) {
					ttcol -= killchar(buf[--cpos]);
					TTflush();
				}
				continue;
			}
			if (c == (CTLFLG | 'U')) {
				/* C-U, kill */
				while (cpos > 0)
					ttcol -= killchar(buf[--cpos]);
				TTflush();
				continue;
			}
			if (c == ctoec(quotec)) {
				quotef = TRUE;
				continue;
			}
		}
		quotef = FALSE;
		c = (Ckey)ectoc(c);
		if (cpos < nbuf-1) {
			ttcol += prntchar(buf[cpos++] = (Char)c);
			TTflush();
		}
	}
}

/*
 * PRNTCHAR: print one character.
 * control characters are changed to printable form.
 * ( <NL> for newline, ^x for other control characters)
 * returns the width of the character.
 */
static int
prntchar(c)
	register Char c;
{
  if (!disinp) return 0;
  if (c == '\n') {
    outstring(CSTR(&Cxstr[3099]));
    return 4;
  }
  if (iscntrl(c) && c != ' ') {
    TTputc('^');
    TTputc(c ^ 0x40);
    return 2;
  }
  TTputc(c);
  return
#if KANJI
    iskanji(c)? 2:
#endif
		1;
}

/*
 * kill one character C. returns the width of the character killed.
 */
static int
killchar(c)
	register Char c;
{
	outstring(CSTR(&Cxstr[3104]));
	if (c == '\n') {
		outstring(CSTR(&Cxstr[3108]));
		return 4;
	}
	if (iscntrl(c) && c != ' ') {
		outstring(CSTR(&Cxstr[3104]));
		return 2;
	}
#if KANJI
	if (iskanji(c)) {
		outstring(CSTR(&Cxstr[3104]));
		return 2;
	}
#endif
	return 1;
}

outstring(s)	/* output a string of characters */
	Char *s;	/* string to output */
{
  if (disinp)
    while (*s)
      TTputc(*s++);
}
