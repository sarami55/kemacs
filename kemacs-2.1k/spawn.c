#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*
 *	Spawn:	various OS access commands
 *		for MicroEMACS
 */

#include	"ecomm.h"
#include	"estruct.h"
#include	"edef.h"

#include	<signal.h>
#include	<pwd.h>
#include	<errno.h>
extern int vttidy();
static int System();

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
/*ARGSUSED*/
spawncli(f, n)
{
	register char *cp;
	char	*getenv();

#if	RESTRICT
	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());
#endif

	movecursor(term.t_nrow, 0);		/* Seek to last line.	*/
	TTflush();

	if((cp = getenv("SHELL")) == NULL || !*cp){
		struct passwd *ent = getpwuid(geteuid());

		cp = (ent == NULL ? "/bin/sh" : ent->pw_shell);
	}
	(void)System(cp, FALSE, NULL, NULL);

	sgarbf = TRUE;
	millisleep(TRUE, 1000);
	 /* originally sleep(2), but maybe 1sec is enough (Nide) */
	return(TRUE);
}

#ifdef SIGTSTP
bktoshell()		/* suspend MicroEMACS and wait to wake up */
{
	movecursor(term.t_nrow, 0);		/* Seek to last line.	*/
	TTflush();

	vttidy();
	(void)kill(0, SIGTSTP); /* suspend all processes within a group */
	TTkopen();
}

/* ARGSUSED */
SIGRET_T
rtfrmshell(SIGARG_T(dummy))
{
	ttopen();
	curwp->w_flag = WFHARD;
	sgarbf = TRUE;
	(void)signal(SIGCONT, rtfrmshell);
}
#endif

static mktmpdir(s)
    char *s; /* like "/tmp/kmXXXXXX" */
{
	char	*p;
	long	l, lbase;
	static	int i = 0;

	p = s + strlen(s) - 6;
	l = getpid() + (getpid() << 15) + time(NULL) + i++;
	lbase = (l &= 0xffffffL);
	do {
		sprintf(p, "%06lX", l);
		if(mkdir(s, 0700) == 0) return 0;
		if(errno != EEXIST) return -1;
	} while(l++, l &= 0xffffffL, l != lbase);
	return -1;
}

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
/*ARGSUSED*/
spawn(f, n)
{
	register int	s;
	Char		line[NLINE];

#if	RESTRICT
	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());
#endif

	if ((s=mlreply(CSTR(&Cxstr[576]),
				  line, NLINE)) != TRUE)
		return (s);
	TTputc('\n');                /* Already have '\r'    */
	TTflush();
	(void)System(cfromC(line), TRUE, NULL, NULL);
	mlputs(
#if KMSGS
	    kterminal? CSTR(&Cxstr[1832]):
#endif
	    CSTR(&Cxstr[7472]));
	TTflush();
	/* Pause. */
	while ((s = tgetc()) != '\r' && s != '\n' && s != ' ')
		;
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
/*ARGSUSED*/
pipecmd(f, n)
{
	register int	s;	/* return status from CLI */
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
	Char	line[NLINE];	/* command line send to shell */
	static Char *bname = CSTR(&Cxstr[7478]);
	bool	retval;

	static char tmpdirtmpl[] = "/tmp/.kmXXXXXX";
	static char tmpfilbas[] = ",command";
	char tmpdirname[sizeof(tmpdirtmpl)];
	char tmpfilnam[sizeof(tmpdirtmpl) + sizeof(tmpfilbas)];
	Char *filnam;

#if	RESTRICT
	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());
#endif

	/* get the command to pipe in */
	if ((s=mlreply(CSTR(&Cxstr[7488]), line, NLINE)) != TRUE)
		return(s);

	/* get rid of the command output buffer if it exists */
	if ((bp=bfind(bname, FALSE, 0)) != FALSE) {
		/* try to make sure we are off screen */
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_bufp == bp) {
				(void)onlywind(FALSE, 1);
				break;
			}
			wp = wp->w_wndp;
		}
		if (zotbuf(bp) != TRUE)
			return(FALSE);
	}

	/* make temporal dir */
	strcpy(tmpdirname, tmpdirtmpl);
	if(mktmpdir(tmpdirname) < 0) return(FALSE);
	chmod(tmpdirname, 0700); /* in case umask is like 0266... :-) */
	sprintf(tmpfilnam, "%s/%s", tmpdirname, tmpfilbas);
	filnam = SafeCfromc(tmpfilnam);

	TTflush();
	(void)System(cfromC(line), TRUE, NULL, tmpfilnam);
	TTflush();
	sgarbf = TRUE;

	/* s = TRUE;
	   if (s != TRUE)
		return(s); */

	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE){
		retval = FALSE;
		goto return_gracefully;
	}

	/* and read the stuff in */
	if (getfile(filnam, FALSE) == FALSE){
		retval = FALSE;
		goto return_gracefully; 
	}

	/* set name and make this window in VIEW mode, update all mode lines */
	(void)copystr(&curwp->w_bufp->b_bname, bname);
	(void)copystr(&curwp->w_bufp->b_fname, CSTR(&Cxstr[6]));
	curwp->w_bufp->b_mode |= MDVIEW;
	upmode();
	retval = TRUE;

	/* and get rid of the temporary file */
    return_gracefully:
	Free(filnam);
	(void)unlink(tmpfilnam);
	rmdir(tmpdirname);
	return(retval);
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
/*ARGSUSED*/
filter(f, n)
{
	register int	s;	/* return status from CLI */
	register BUFFER *bp;	/* pointer to buffer to zot */
	Char line[NLINE];	/* command line send to shell */
	Char *tmpnam;		/* place to store real file name */
	bool retval;

	static char tmpdirtmpl[] = "/tmp/.kmXXXXXX";
	static char tmpfilbas1[] = ",fltinp";
	static char tmpfilbas2[] = ",fltout";
	char tmpdirname[sizeof(tmpdirtmpl)];
	char tmpfilnam1[sizeof(tmpdirtmpl) + sizeof(tmpfilbas1)];
	char tmpfilnam2[sizeof(tmpdirtmpl) + sizeof(tmpfilbas2)];
	Char *filnam1, *filnam2;

#if	RESTRICT
	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());
#endif

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* get the filter name and its args */
	if ((s=mlreply(CSTR(&Cxstr[7490]), line, NLINE)) != TRUE)
		return(s);

	/* make temporal dir */
	strcpy(tmpdirname, tmpdirtmpl);
	if(mktmpdir(tmpdirname) < 0) return(FALSE);
	chmod(tmpdirname, 0700); /* in case umask is like 0266... :-) */
	sprintf(tmpfilnam1, "%s/%s", tmpdirname, tmpfilbas1);
	sprintf(tmpfilnam2, "%s/%s", tmpdirname, tmpfilbas2);
	filnam1 = SafeCfromc(tmpfilnam1);
	filnam2 = SafeCfromc(tmpfilnam2);

	/* setup the proper file names */
	bp = curbp;
	tmpnam = bp->b_fname;	/* save the original name */
	bp->b_fname = NULL;

	/* write it out, checking for errors */
	if (writeout(filnam1) != TRUE) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[7492]):
#endif
		    CSTR(&Cxstr[7512]));
		retval = FALSE;
		goto return_gracefully;
	}

	TTflush();
	(void)System(cfromC(line), TRUE, tmpfilnam1, tmpfilnam2);
	TTflush();
	sgarbf = TRUE;

	/* on failure, escape gracefully */
	if (readin(filnam2, FALSE) != TRUE) {
		mlwrite(
#if KMSGS
		    kterminal? CSTR(&Cxstr[7539]):
#endif
		    CSTR(&Cxstr[7554]));
		retval = FALSE; /* not s, I think. Nide */ 
		goto return_gracefully;
	}

	retval = TRUE;
	bp->b_flag |= BFCHG;		/* flag it as changed */

  return_gracefully:
	/* reset file name */
	free((char *)bp->b_fname);
	bp->b_fname =  tmpnam;	/* restore name */

	/* and get rid of the temporary file */
	Free(filnam1);
	Free(filnam2);
	(void)unlink(tmpfilnam1);
	(void)unlink(tmpfilnam2);
	rmdir(tmpdirname);
	return(retval);
}

/*
 * execute shell command.
 */
static int
System(cmd, via_shell_p, inf, outf)
    char *cmd;
    bool via_shell_p;
    char *inf, *outf;
{
    int s;
#if	CLOCK
    SIGRET_T (*f)();
    unsigned t;
#endif

#if	CLOCK
    f = signal(SIGALRM, SIG_IGN);
    t = alarm((unsigned)0);
#endif
    TTclose();

    switch(fork()){
    case -1:
	s = -1;
	break;
    case 0:
	if(inf != NULL){
	    if(freopen(inf, "r", stdin) == NULL) exit(-1);
	}
	if(outf != NULL){
	    if(freopen(outf, "w", stdout) == NULL) exit(-1);
	}
	
	if(via_shell_p){
	    execlp("/bin/sh", "/bin/sh", "-c", cmd, NULL);
	} else {
	    execlp(cmd, cmd, NULL);
	}
	exit(-1);
    default:
	for(;;){
	    int child_stat;

	    if(0 >= wait(&child_stat)){s = -1; break;}
	    switch(child_stat & 0xff){
	    case 0: /* exitted */
		s = (child_stat >> 8) & 0xff; break;
	    case 0x7f: /* stopped */
		continue;
	    default: /* terminated by signal */
		s = -1; break;
	    }
	    break;
	}
    }

    ttopen();
#if	CLOCK
    (void)signal(SIGALRM, f);
    (void)alarm(t);
#endif
    return s;
}
