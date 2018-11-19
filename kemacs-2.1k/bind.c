#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*	This file is for functions having to do with key bindings,
	descriptions, help commands and startup file.

	written 11-feb-86 by Daniel Lawrence
								*/

#include	"ecomm.h"
#include	"estruct.h"
#include	"edef.h"
#include	"epath.h"

extern int meta_pfx(), cex(), unarg(), ctrlg(); /* dummy prefix binding functions */
static int unbindchar();

/*ARGSUSED*/
deskey(f, n)	/* describe the command for a certain key */
{
	register Ckey c;	/* command character to describe */
	register Char *ptr;	/* string pointer to scan output strings */
	Char outseq[80];	/* output buffer for command sequence */
	Ckey getckey();

	/* prompt the user to type us a key to describe */
	mlwrite(CSTR(&Cxstr[360]));

	/* get the command sequence to describe */
	c = getckey(FALSE);			/* get a command sequence */

	/* change it to something we can print as well */
	cmdstr(c, &outseq[0]);

	/* and dump it out */
	if (discmd) {
	  ptr = &outseq[0];
	  while (*ptr)
	    TTputc(*ptr++);
	  TTputc(' ');		/* space it out */
	}

	if ((ptr = key2fname(c)) == NULL) {
	    /* no function is bound to this key */
		(void)Cstrcpy(outseq,
#if KMSGS
		    kterminal? CSTR(&Cxstr[376]):
#endif
		    CSTR(&Cxstr[382]));
	}
	else
	    (void)Cstrcpy(outseq, ptr);
	if (outseq[0] == '\0') {
	    /* something wrong with the name binding table */
		(void)Cstrcpy(outseq,
#if KMSGS
		    kterminal? CSTR(&Cxstr[392]):
#endif
		    CSTR(&Cxstr[410]));
			}

	/* output the command sequence */
	ptr = &outseq[0];
	while (*ptr)
		TTputc(*ptr++);
}

cmdstr(c, seq)	/* change a key command to a string we can print out */
	Ckey c;		/* sequence to translate */
	Char *seq;	/* destination string for sequence */
{
	Char *ptr;	/* pointer into current position in sequence */

	ptr = seq;

	/* apply meta sequence if needed */
	if (c & META) {
		*ptr++ = 'M';
		*ptr++ = '-';
	}

	/* apply ^X sequence if needed */
	if (c & CTLX) {
		*ptr++ = '^';
		*ptr++ = 'X';
	}

	/* apply SPEC sequence if needed */
	if (c & SPEC) {
		*ptr++ = 'F';
		*ptr++ = 'N';
	}

	/* apply control sequence if needed */
	if (c & CTLFLG) {
		*ptr++ = '^';
	}

	c &= ~CK_FLAGS;	/* strip the prefixes */

	/* and output the final sequence */

	*ptr++ = c;
	*ptr = 0;	/* terminate the string */
}

/*
 * returns help buffer name
 */
Char *
help_buffer_name()
{
	return (
#if KMSGS
		kterminal? CSTR(&Cxstr[424]):
#endif
		CSTR(&Cxstr[430]));
}

/*ARGSUSED*/
help(f, n)	/* give me some help!!!!
		   bring up a fake buffer and read the help file
		   into it with view mode			*/
{
	register int status;	/* status of I/O operations */
	register int i;		/* index into help file names */
	Char fname[NSTRING];	/* buffer to construct file name in */
	int ready = (hlpbp != NULL);

	/* first check if we are already here */
	if (!hlpbp && !(hlpbp = bfind(help_buffer_name(), TRUE, 0))) {
		mlwrite(
#if KMSGS
			kterminal? CSTR(&Cxstr[437]):
#endif
			CSTR(&Cxstr[449]));
		return(FALSE);
	}
	if (!ready) {
		register BUFFER *bp = curbp;

		/* contents not read in yet */
		/* search through the list of help files */
		for (i=2; i < NPNAMES; i++) {
			(void)Cstrcpy(fname, pathname[i]);
			(void)Cstrcat(fname, pathname[1]);
#if	KANJI
			status = ffropen(fname, KS_VALUE(kcode));
#else
			status = ffropen(fname);
#endif
			if (status == FIOSUC)
				break;
		}

		if (status == FIOFNF) {
			mlwrite(
#if KMSGS
			    kterminal? CSTR(&Cxstr[470]):
#endif
			    CSTR(&Cxstr[490]));
			return(FALSE);
		}

		/* close the file to prepare for to read it in */
#if	KANJI
		(void)ffclose((unsigned *)NULL);
#else
		(void)ffclose();
#endif
		/* and read the stuff in */
		curbp = hlpbp;
		if (readin(fname, FALSE) == FALSE) {
			curbp = bp;
			return(FALSE);
		}
		curbp = bp;
	}
	if (make_onscreen(hlpbp) == FALSE)
		return(FALSE);
	(void)copystr(&hlpbp->b_fname, CSTR(&Cxstr[6]));
	hlpbp->b_mode |= MDVIEW;
	upmode();
	return(TRUE);
}

/*
 * dummy function so that distinguishing 'Not found' from 'Aborted'
 */
int
nullfunc()
{}

int (*fncmatch(fname))() /* match fname to a function in the names table
			    and return any match or nullfunc if none	*/
	Char *fname;	/* name to attempt to match */
{
	register NBIND *ffp;	/* pointer to entry in name binding table */

	/* scan through the table, returning any match */
	ffp = &names[0];
	while (ffp->n_func != NULL) {
		if (Cstrcmp(fname, ffp->n_name) == 0)
			return(ffp->n_func);
		++ffp;
	}
	return(nullfunc);
}

/* bindtokey:	add a new key to the key binding table
*/

/*ARGSUSED*/
bindtokey(f, n)
{
	register Ckey c;	/* command key to bind */
	register int (*kfunc)(); /* ptr to the requexted function to bind to */
	register Char *ptr;	/* ptr to dump out input key string */
	register KEYTAB *ktp;	/* pointer into the command table */
	register int found;	/* matched command flag */
	Char outseq[80];	/* output buffer for keystroke sequence */
	int (*getname())();
	Ckey getckey();

	/* prompt the user to type in a key to bind */
	mlwrite(CSTR(&Cxstr[516]));

	/* get the function name to bind it to */
	kfunc = getname();
	if (kfunc == NULL) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[531]):
#endif
		    CSTR(&Cxstr[547]));
		return(FALSE);
	}
	if (discmd) {
	  TTputc(' ');		/* space it out */
	  TTflush();
	}

	/* get the command sequence to bind */
	c = getckey((kfunc == meta_pfx) || (kfunc == cex) ||
	            (kfunc == unarg) || (kfunc == ctrlg));

	/* change it to something we can print as well */
	cmdstr(c, &outseq[0]);

	/* and dump it out */
	if (discmd) {
	  ptr = &outseq[0];
	  while (*ptr)
	    TTputc(*ptr++);
	}

	/* if the function is a prefix key */
	if (kfunc == meta_pfx || kfunc == cex ||
	    kfunc == unarg || kfunc == ctrlg) {

		/* search for an existing binding for the prefix key */
		ktp = &keytab[0];
		found = FALSE;
		while (ktp->k_fp != NULL) {
			if (ktp->k_fp == kfunc)
				(void)unbindchar(ktp->k_code);
			++ktp;
		}

		/* reset the appropriate global prefix variable */
		if (kfunc == meta_pfx)
			metac = c;
		if (kfunc == cex)
			ctlxc = c;
		if (kfunc == unarg)
			reptc = c;
		if (kfunc == ctrlg)
			abortc = c;
		}

		/* search the table to see if it exists */
		ktp = &keytab[0];
		found = FALSE;
		while (ktp->k_fp != NULL) {
			if (ktp->k_code == c) {
				found = TRUE;
				break;
			}
			++ktp;
		}

	if (found) {	/* it exists, just change it then */
		ktp->k_fp = kfunc;
	} else {	/* otherwise we need to add it to the end */
		/* if we run out of binding room, bitch */
		if (ktp >= &keytab[NBINDS]) {
			mlwrite(
#if KMSGS
			    kterminal? CSTR(&Cxstr[566]):
#endif
			CSTR(&Cxstr[578]));
			return(FALSE);
		}

		ktp->k_code = c;	/* add keycode */
		ktp->k_fp = kfunc;	/* and the function pointer */
		++ktp;			/* and make sure the next is null */
		ktp->k_code = 0;
		ktp->k_fp = NULL;
	}
	return(TRUE);
}

/* unbindkey:	delete a key from the key binding table
*/

/*ARGSUSED*/
unbindkey(f, n)
{
	register Ckey c;		/* command key to unbind */
	register Char *ptr;	/* ptr to dump out input key string */
	Char outseq[80];	/* output buffer for keystroke sequence */
	Ckey getckey();

	/* prompt the user to type in a key to unbind */
	mlwrite(CSTR(&Cxstr[598]));

	/* get the command sequence to unbind */
	c = getckey(FALSE);		/* get a command sequence */

	/* change it to something we can print as well */
	cmdstr(c, &outseq[0]);

	/* and dump it out */
	if (discmd) {
	  ptr = &outseq[0];
	  while (*ptr)
	    TTputc(*ptr++);
	}

	/* if it isn't bound, bitch */
	if (unbindchar(c) == FALSE) {
		mlwrite(
#if KMSGS
			kterminal? CSTR(&Cxstr[612]):
#endif
			CSTR(&Cxstr[623]));
		return(FALSE);
	}
	return(TRUE);
}

static int
unbindchar(c)
	Ckey c;		/* command key to unbind */
{
	register KEYTAB *ktp;	/* pointer into the command table */
	register KEYTAB *sktp;	/* saved pointer into the command table */
	register int found;	/* matched command flag */

	/* search the table to see if the key exists */
	ktp = &keytab[0];
	found = FALSE;
	while (ktp->k_fp != NULL) {
		if (ktp->k_code == c) {
			found = TRUE;
			break;
		}
		++ktp;
	}

	/* if it isn't bound, bitch */
	if (!found)
		return(FALSE);

	/* save the pointer and scan to the end of the table */
	sktp = ktp;
	while (ktp->k_fp != NULL)
		++ktp;
	--ktp;		/* backup to the last legit entry */

	/* copy the last entry to the current one */
	sktp->k_code = ktp->k_code;
	sktp->k_fp   = ktp->k_fp;

	/* null out the last one */
	ktp->k_code = 0;
	ktp->k_fp = NULL;
	return(TRUE);
}

/*ARGSUSED*/
desbind(f, n)	/* describe bindings
		   bring up a fake buffer and list the key bindings
		   into it with view mode			*/
#if	APROP
{
	return (buildlist(TRUE, CSTR(&Cxstr[6])));
}

/*ARGSUSED*/
apro(f, n)	/* Apropos (List functions that match a substring) */
{
	Char mstring[NSTRING];	/* string to match cmd names to */
	int status;		/* status return */

	status = mlreply(
# if KMSGS
			 kterminal? CSTR(&Cxstr[639]):
# endif
			 CSTR(&Cxstr[647]),
			 mstring, NSTRING - 1);
	if (status != TRUE)
		return(status);

	return(buildlist(FALSE, mstring));
}

Char *
binding_list_buffer_name()
{
	return (
# if KMSGS
		kterminal? CSTR(&Cxstr[664]):
# endif
		CSTR(&Cxstr[672]));
}

buildlist(type, mstring)  /* build a binding list (limited or full) */
	int type;	/* true = full list,   false = partial list */
	Char *mstring;	/* match string if a partial list */
#endif /* APROP */
{
	register KEYTAB *ktp;	/* pointer into the command table */
	register NBIND *nptr;	/* pointer into the name binding table */
	int cpos;		/* current position to use in outseq */
	Char outseq[256];	/* output buffer for keystroke sequence */

	/* Binding List buffer */
	if (!bdlistp &&
	    !(bdlistp = bfind(binding_list_buffer_name(), TRUE, 0)) ||
	    bclear(bdlistp) == FALSE) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[687]):
#endif
		    CSTR(&Cxstr[701]));
		return(FALSE);
	}

	/* let us know this is in progress */
	mlwrite(
#if KMSGS
		kterminal? CSTR(&Cxstr[730]):
#endif
		CSTR(&Cxstr[741]));

	/* build the contents of this window, inserting it line by line */
	bdlistp->b_flag &= ~BFCHG;
	bdlistp->b_mode |= MDVIEW;
	(void)copystr(&bdlistp->b_fname, CSTR(&Cxstr[6]));
	(void)copystr(&bdlistp->b_bname, binding_list_buffer_name());
	nptr = &names[0];
	while (nptr->n_func != NULL) {

		/* add in the command name */
		(void)Cstrcpy(outseq, nptr->n_name);
		cpos = Cstrlen(outseq);
		
#if	APROP
		/* if we are executing an apropos command..... */
		if (type == FALSE &&
		    /* and current string doesn't include the search string */
		    strinc(outseq, mstring) == FALSE)
			goto fail;
#endif
		/* search down any keys bound to this */
		ktp = &keytab[0];
		while (ktp->k_fp != NULL) {
			if (ktp->k_fp == nptr->n_func) {
				/* padd out some spaces */
				while (cpos < 25)
					outseq[cpos++] = ' ';

				/* add in the command sequence */
				cmdstr(ktp->k_code, &outseq[cpos]);
				while (outseq[cpos] != 0)
					++cpos;

				/* and add it as a line into the buffer */
				if (addline(bdlistp, outseq) == FALSE)
					return(FALSE);
				cpos = 0;	/* and clear the line */
			}
			++ktp;
		}

		/* if no key was bound, we need to dump it anyway */
		if (cpos > 0) {
			outseq[cpos] = 0;
			if (addline(bdlistp, outseq) == FALSE)
				return(FALSE);
		}

fail:		/* and on to the next name */
		++nptr;
	}

	if (make_onscreen(bdlistp) == FALSE)
		return(FALSE);

	/* clear the message line */
	mlwrite(CSTR(&Cxstr[6]));
	return(TRUE);
}

#if	APROP
strinc(source, sub)	/* does source include sub? */
	Char *source;	/* string to search in */
	Char *sub;	/* substring to look for */
{
	Char *sp;	/* ptr into source */
	Char *nxtsp;	/* next ptr into source */
	Char *tp;	/* ptr into substring */

	/* for each character in the source string */
	sp = source;
	while (*sp) {
		tp = sub;
		nxtsp = sp;

		/* is the substring here? */
		while (*tp) {
			if (*nxtsp++ != *tp)
				break;
			else
				tp++;
		}

		/* yes, return a success */
		if (*tp == 0)
			return(TRUE);

		/* no, onward */
		sp++;
	}
	return(FALSE);
}
#endif

/* get a command key sequence from the keyboard */
Ckey
getckey(mflag)
	int mflag;	/* going for a meta sequence? */
{
	register Ckey c;	/* character fetched */
	Char tok[NSTRING];	/* command incoming */
	Ckey get1key();
	Ckey getcmd();

	/* check to see if we are executing a command line */
	if (clexec) {
		(void)macarg(tok);	/* get the next token */
		return strcmd(tok);
		}

	/* or the normal way */
	if (mflag)
		c = get1key();
	else
		c = getcmd();
	return(c);
}

/* execute the startup file */

startup(sfname)
	Char *sfname;	/* name of startup file (null if default) */
{
	register int status;	/* status of I/O operations */
	register int i;		/* index into help file names */
	Char fname[NSTRING];	/* buffer to construct file name in */
	Char *sfroot;		/* root of startup file name */
	Char *homedir;		/* pointer to your home directory */
	char *getenv();

	sfroot = sfname[0]? sfname: pathname[0];

	/* get the HOME from the environment */
	if ((homedir = Cfromc(getenv("HOME"))) != NULL) {
		/* build the file name */
		(void)Cstrcpy(fname, homedir);
		(void)Cstrcat(fname, CSTR(&Cxstr[765]));
		(void)Cstrcat(fname, sfroot);

		/* and test it */
#if	KANJI
		status = ffropen(fname, KS_VALUE(kcode));
#else
		status = ffropen(fname);
#endif
		if (status == FIOSUC) {
#if	KANJI
			(void)ffclose((unsigned *)NULL);
#else
			(void)ffclose();
#endif
			return(dofile(fname));
		}
	}

	/* search through the list of startup files */
	for (i=2; i < NPNAMES; i++) {
		(void)Cstrcpy(fname, pathname[i]);
		(void)Cstrcat(fname, sfroot);
#if	KANJI
		status = ffropen(fname, KS_VALUE(kcode));
#else
		status = ffropen(fname);
#endif
		if (status == FIOSUC)
			break;
	}

	/* if it isn't around, don't sweat it */
	if (status == FIOFNF)
		return(TRUE);

	/* close the file to prepare for to read it in */
#if	KANJI
	(void)ffclose((unsigned *)NULL);
#else
	(void)ffclose();
#endif

	return(dofile(fname));
}

/*
 * Find function bound to command key c, and return pointer to its name.
 * If key is not bouns to any function, return NULL.
 * If key is bound to certain function, but that function is not registered,
 * return pointer to null string.
 */
Char *
key2fname(c)
    Ckey c;
{
    register KEYTAB *ktp;	/* pointer into the command table */
    register NBIND *nptr;	/* pointer into the name binding table */
    register int (*fp)();		/* pointer to function bound to key */

    ktp = &keytab[0];
    while (ktp->k_fp != NULL && ktp->k_code != c)
	ktp++;
    if (ktp->k_fp == NULL)
	return NULL;
    fp = ktp->k_fp;
    nptr = &names[0];
    while (nptr->n_func != NULL && nptr->n_func != fp)
	nptr++;
    return (nptr->n_func == NULL)? CSTR(&Cxstr[6]): nptr->n_name;
}

/*
 * return inner represantation of command key sequence
 *
 */
Ckey
strcmd(s)
    register Char *s;
{
    register Ckey c;

    c = 0;

    /* check (combination of) prefixes	*/
    /* possible combinations are:		*/
    /* 	(M-|^X)?(FN)?(^)?c		*/
    /* e.g.					*/
    /*	A	(simple char)		*/
    /*	^A	(control char)		*/
    /*	FNA	(function key)		*/
    /*	FN^A	(modified f.key)	*/
    /*	M-(same as above) (meta key)	*/
    /*	^X(same as above) (control-x)	*/

    /* first, META or CTLX prefixes */
    if (*s == 'M' && *(s+1) == '-') {
	/* the META prefix */
	c = META;
	s += 2;
    } else if (*s == '^' && *(s+1) == 'X') {
	/* control-x as well... */
	c = CTLX;
	s += 2;
    }

    /* next the function prefix */
    if (*s == 'F' && *(s+1) == 'N') {
	c |= SPEC;
	s += 2;
    }

    /* a control char? */
    if (*s == '^' && *(s+1) != 0) {
	c |= CTLFLG;
	++s;
    }

    /* make sure we are not lower case */
    c |= (c & CTLFLG && isLower(*s))? CHCASE(*s): *s;

    return(c);
}
