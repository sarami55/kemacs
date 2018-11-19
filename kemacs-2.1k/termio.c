#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*
 * The functions in this file negotiate with the operating system for
 * characters, and write characters in a barely buffered fashion on the display.
 * All operating systems.
 */
#include	"ecomm.h"
#include	"estruct.h"
#include	"edef.h"

static Char *dummy = CSTR(&Cxstr[6]);

#include <signal.h>
#if HAVE_TERMIOS
# include <termios.h>
struct	termios	otermio;	/* original terminal characteristics */
struct	termios	ntermio;	/* charactoristics to use inside */
#else
# if	USG			/* System V */
#  include	<termio.h>
#  ifndef TCGETS /* uses TCGETA */
struct	termio	otermio;	/* original terminal characteristics */
struct	termio	ntermio;	/* charactoristics to use inside */
#  else
 /* should I include <termios.h> here? */
struct	termios	otermio;
struct	termios	ntermio;
#  endif
# endif /* USG */

# if	BSD
#  include	<sgtty.h>	 /* for stty/gtty functions */
struct	sgttyb	ostate;		 /* saved tty state */
struct	sgttyb	nstate;		 /* values for editor mode */
struct tchars	otchars;	/* Saved terminal special character set */
struct tchars	ntchars = { (char)0xff, (char)0xff, (char)0xff,
			    (char)0xff, (char)0xff, (char)0xff };
				/* A lot of nothing */
# endif /* BSD */
#endif /* !HAVE_TERMIOS */
#if BSD
# include <sys/ioctl.h>	/* to get at the typeahead */
# define TBUFSIZ 128
char tobuf[TBUFSIZ];		/* terminal output buffer */
#endif /*BSD*/

#ifdef SIGWINCH
extern	SIGRET_T change_win_size();	/* change window size & redisplay */
#endif
#ifdef SIGTSTP
extern	SIGRET_T rtfrmshell();	/* return from suspended shell */
#endif

#if USE_STTY_CMD
#define STTYGMAX 512
char sttyg[STTYGMAX+1];
int sttygsiz;
#endif

/*
 * This function is called once to set up the terminal device streams.
 */
ttopen()
{
#if USE_STTY_CMD
	FILE *f;
#endif

#if HAVE_TERMIOS
	tcgetattr(0, &otermio);
# if STRASSOK
	ntermio = otermio;
# else
	(void)memcpy((void *)&ntermio, (void *)&otermio, sizeof(otermio));
# endif
	cfmakeraw(&ntermio);
	 /* the below 2 lines are for OpenBSD 2.5 BUGS... */
	ntermio.c_cc[VMIN] = 1;
	ntermio.c_cc[VTIME] = 0;
	tcsetattr(0, TCSADRAIN, &ntermio);
#else /* !HAVE_TERMIOS */
# if	USG
#  ifndef TCGETS
	ioctl(0, TCGETA, &otermio);	/* save old settings */
#  else
	ioctl(0, TCGETS, &otermio);
#  endif
					/* setup new settings */
#  if STRASSOK
	ntermio = otermio;
#  else
	(void)memcpy((void *)&ntermio, (void *)&otermio, sizeof(otermio));
#  endif 
	ntermio.c_iflag = 0;		
	ntermio.c_oflag = 0;
	ntermio.c_lflag = 0;
	ntermio.c_cc[VMIN] = 1;
	ntermio.c_cc[VTIME] = 0;
#  ifndef TCGETS
	ioctl(0, TCSETAW, &ntermio);	/* and activate them */
#  else
	ioctl(0, TCSETSW, &ntermio);
#  endif
# endif /* USG */

# if	BSD
	(void)gtty(0, &ostate);			/* save old state */
	(void)gtty(0, &nstate);			/* get base of new state */
	nstate.sg_flags |= RAW;
	nstate.sg_flags &= ~(ECHO|CRMOD);	/* no echo for now... */
	(void)stty(0, &nstate);			/* set mode */
	(void)ioctl(0, (int)TIOCGETC, (char *)&otchars); /* Save old characters */
	(void)ioctl(0, (int)TIOCSETC, (char *)&ntchars); /* Place new character into K */
# endif /* BSD */
#endif /* !HAVE_TERMIOS */
#if BSD
	/* provide a smaller terminal output buffer so that
	   the type ahead detection works better (more often) */
	setbuffer(stdout, &tobuf[0], TBUFSIZ);
#endif	/*BSD*/

#if USE_STTY_CMD /* use stty command; added by N.Nide */
	sttygsiz = 0; 
	if(getenv("UUM_COUNTDOWN") != NULL &&
	   NULL != (f = popen("exec stty -g", "r"))){
		int c;

		while(sttygsiz < STTYGMAX && EOF != (c = getc(f))){
			sttyg[sttygsiz++] = c;
		}
		pclose(f);
		if(sttygsiz > 0 && sttyg[sttygsiz - 1] == '\n')
			sttygsiz--;
		sttyg[sttygsiz] = '\0';
		if(sttygsiz){
			system("exec stty raw -echo -ixon lnext '^-'");
		}
	}
#endif

#ifdef SIGTSTP
	(void)signal(SIGTSTP,SIG_DFL);		/* set signals so that we can */
	(void)signal(SIGCONT,rtfrmshell);	/* suspend & restart emacs */
#endif
#ifdef SIGWINCH
	(void)signal(SIGWINCH,change_win_size);	/* change window size */
#endif
}

/*
 * This function gets called just before we go back home to the command
 * interpreter.
 */
ttclose()
{
#if USE_STTY_CMD
	if(sttygsiz){
		switch(fork()){
		case -1:
			break;
		case 0:
			execlp("stty", "stty", sttyg, NULL);
			exit(-1);
		default:
			wait(NULL);
			break;
		}
	}
#endif

#if HAVE_TERMIOS
	tcsetattr(0, TCSADRAIN, &otermio);
#else
# if	USG
#  ifndef TCGETS
	ioctl(0, TCSETAW, &otermio);	/* restore terminal settings */
#  else
	ioctl(0, TCSETSW, &otermio);
#  endif
# endif /* USG */

# if	BSD
	(void)stty(0, &ostate);
	(void)ioctl(0, (int)TIOCSETC, (char *)&otchars); /* Place old character into K */
# endif /* BSD */
#endif /* HAVE_TERMIOS */
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 */
ttputc(c)
    Char c;
{
#if	KANJI
	kkputc(c, stdout);
#else
	putchar(c);
#endif /*KANJI*/
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
ttflush()
{
#if KANJI
  kkputc(EOF, stdout);
#endif
  (void)fflush(stdout);
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all.
 */
Char
ttgetc()
{
#if KANJI
	Char kkgetc();

	return(kkgetc(stdin));
#else
	return (Char)getchar();
#endif
}

#if	TYPEAH
/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/
typahead()
{
#ifdef FIONREAD
	int x;	/* holds # of pending chars */

	return((ioctl(0, (int)FIONREAD, (char *)&x) < 0) ? 0 : x);
#else
	return 0;
#endif
}
#endif	/*TYPEAH*/
