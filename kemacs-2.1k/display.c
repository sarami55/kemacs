#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*
 * The functions in this file handle redisplay. There are two halves, the
 * ones that update the virtual display screen, and the ones that make the
 * physical display screen the same as the virtual display screen. These
 * functions use hints that are left in the windows by the commands.
 *
 */

#include	"ecomm.h"
#include	"estruct.h"
#include	"edef.h"

#if	HAVE_STDARG
 /* 2005/1/30 by NIDE, N.  Very old cc does not know #elif (??) */
# define HANDLE_VARIABLE_ARGS
#else
#if	HAVE_VARARGS
# define HANDLE_VARIABLE_ARGS
#endif
#endif

#if	CLOCK
# include	<signal.h>
#endif
#ifdef	HANDLE_VARIABLE_ARGS
# if	HAVE_STDARG
#  include	<stdarg.h>
# else
#  include	<varargs.h>
# endif
#endif

static Char *dummy = CSTR(&Cxstr[6]);
static updateline();

typedef struct	VIDEO {
	int	v_flag;			/* Flags */
#if	COLOR
	int	v_fcolor;		/* current forground color */
	int	v_bcolor;		/* current background color */
	int	v_rfcolor;		/* requested forground color */
	int	v_rbcolor;		/* requested background color */
#endif
	Char	v_text[1];		/* Screen data. */
}	VIDEO;

#define VFCHG	0x0001			/* Changed flag			*/
#define VFEXT	0x0002			/* extended (beyond column 80)	*/
#define VFREV	0x0004			/* reverse video status		*/
#define VFREQ	0x0008			/* reverse video request	*/
#define VFCOL	0x0010			/* color change requested	*/

VIDEO	**vscreen;			/* Virtual screen. */
VIDEO	**pscreen;			/* Physical screen. */

#if KANJI
/*
 * trailing part of multi-column character
 * this should be distinguishable from other Character code.
 */
# define TRAILER	(-1)
#endif

/*
 * Initialize the data structures used by the display code. The edge vectors
 * used to access the screens are set up. The operating system's terminal I/O
 * channel is set up. All the other things get initialized at compile time.
 * The original window has "WFCHG" set, so that it will get completely
 * redrawn on the first call to "update".
 */
vtinit()
{
    register int i;
    register VIDEO *vp;

    TTopen();		/* open the screen */
    TTkopen();		/* open the keyboard */
    TTrev(FALSE);
    vscreen = (VIDEO **) malloc((unsigned)MAX_SCREEN_LINES*sizeof(VIDEO *));

    if (vscreen == NULL)
	exit(1);

    pscreen = (VIDEO **) malloc((unsigned)MAX_SCREEN_LINES*sizeof(VIDEO *));

    if (pscreen == NULL)
	exit(1);

    for (i = 0; i < MAX_SCREEN_LINES; ++i) {
	vp = (VIDEO *)malloc((unsigned)(sizeof(VIDEO)+sizeof(Char)*MAX_SCREEN_COLUMNS));
	if (vp == NULL) exit(1);

	vp->v_flag = 0;
#if	COLOR
	vp->v_rfcolor = 7;
	vp->v_rbcolor = 0;
#endif
	vscreen[i] = vp;

	vp = (VIDEO *)malloc((unsigned)(sizeof(VIDEO)+sizeof(Char)*MAX_SCREEN_COLUMNS));
	if (vp == NULL) exit(1);

	vp->v_flag = 0;
	pscreen[i] = vp;
    }
}

/*
 * Clean up the virtual terminal system, in anticipation for a return to the
 * operating system. Move down to the last line and clear it out (the next
 * system prompt will be written in the line). Shut down the channel to the
 * terminal.
 */
vttidy()
{
    mlerase();
    movecursor(term.t_nrow, 0);
    TTflush();
    TTkclose(); /* moved from above mlerase() by N.Nide. */
    TTclose();
}

/*
 * Set the virtual cursor to the specified row and column on the virtual
 * screen. There is no checking for nonsense values; this might be a good
 * idea during the early stages.
 */
vtmove(row, col)
{
    vtrow = row;
    vtcol = col;
}

/*
 * Write a character to the virtual screen. The virtual row and
 * column are updated. If we are not yet on left edge, don't print
 * it yet. If the line is too long put a "$" in the last column.
 * This routine only puts printing characters into the virtual
 * terminal buffers. Only column overflow is checked.
 */

vtputc(c)
    Char c;
{
    register VIDEO *vp;	/* ptr to line being updated */

    vp = vscreen[vtrow];

    if (vtcol >= term.t_ncol) {
#if KANJI
	register Char	*tp;
#endif
	++vtcol;
#if KANJI
	for (tp = &vp->v_text[term.t_ncol-1]; *tp == TRAILER; tp--) ;
	for ( ; tp < &vp->v_text[term.t_ncol-1]; *tp++ = ' ') ;
#endif
	vp->v_text[term.t_ncol - 1] = '$';
    } else if (c == '\t') {
	do {
	    vtputc(' ');
	} while (((vtcol + taboff)&0x07) != 0);
    } else if (c < 0x20 || c == 0x7F) {
      vtputc('^');
      vtputc(c ^ 0x40);
    } else if (c >= 0x80  && c < 0x100) {
      vtputc('\\');
      vtputc('0'+((c & 0700) >> 6));
      vtputc('0'+((c & 0070) >> 3));
      vtputc('0'+((c & 0007) >> 0));
#if KANJI
    } else if (iskanji(c)) {
	/* two columns are needed to display */
	if (vtcol >= term.t_ncol-1 || vtcol < 0) {
	    /* abandon to display */
	    vtputc(' ');
	    vtputc(' ');
	} else {
	    if (vtcol >= 0) {
		/* first col */
		vp->v_text[vtcol] = c;
		/* second col */
		vp->v_text[vtcol+1] = TRAILER;
	    }
	    vtcol += 2;
	}
#endif /*KANJI*/
    } else {
	if (vtcol >= 0)
	    vp->v_text[vtcol] = c;
	++vtcol;
    }
}

/*
 * Erase from the end of the software cursor to the end of the line on which
 * the software cursor is located.
 */
vteeol()
{
    register VIDEO	*vp;

    vp = vscreen[vtrow];
    while (vtcol < term.t_ncol) {
	vp->v_text[vtcol++] = ' ';
    }
}

/* upscreen:	user routine to force a screen update
		always finishes complete update		*/
/*ARGSUSED*/
upscreen(f, n)
{
	update(TRUE);
	return(TRUE);
}

/* updmode:	user routine to force a modeline update */
/*ARGSUSED*/
updmode(f, n)
{
	upmode();
	update(f);
	return(TRUE);
}

/*
 * Make sure that the display is right. This is a three part process. First,
 * scan through all of the windows looking for dirty ones. Check the framing,
 * and refresh the screen. Second, make sure that "currow" and "curcol" are
 * correct for the current window. Third, make the virtual and physical
 * screens the same.
 */
update(force)
    int force;	/* force update past type ahead? */
{
#if	CLOCK
	SIGRET_T (*f)();
	unsigned t;

	/* block ALRM while updating screen */
	f = signal(SIGALRM, SIG_IGN);
	t = alarm((unsigned)0);
	do_update(force);
	(void)signal(SIGALRM, f);
	(void)alarm(t);
}

do_update(force)
    int force;
{
#endif	/*CLOCK*/
	register WINDOW *wp;

#if	TYPEAH
	if (force == FALSE && typahead())
		return;
#endif
#if	VISMAC == 0
	if (force == FALSE && kbdmode == PLAY)
		return;
#endif

	/* update any windows that need refreshing */
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_flag) {
			/* if the window has changed, service it */
			reframe(wp);	/* check the framing */
			if ((wp->w_flag & ~WFMODE) == WFEDIT)
				updone(wp);	/* update EDITed line */
			else if (wp->w_flag & ~WFMOVE)
				updall(wp);	/* update all lines */
			if (wp->w_flag & WFMODE)
				modeline(wp);	/* update modeline */
			wp->w_flag = 0;
			wp->w_force = 0;
		}
		/* on to the next window */
		wp = wp->w_wndp;
	}

	/* recalc the current hardware cursor location */
	updpos();

	/* update the cursor and flush the buffers */
	movecursor(currow, curcol - lbound);

	/* check for lines to de-extend */
	upddex();

	/* if screen is garbage, re-plot it */
	if (sgarbf != FALSE)
		updgar();

	/* update the virtual screen to the physical screen */
	updupd(force);

	/* update the cursor and flush the buffers */
	movecursor(currow, curcol - lbound);
	TTflush();
}

/*	reframe:	check to see if the cursor is on in the window
			and re-frame it if needed or wanted		*/

reframe(wp)
	WINDOW *wp;
{
	register LINE *lp;
	register int i;

	/* if not a requested reframe, check for a needed one */
	if ((wp->w_flag & WFFORCE) == 0) {
		lp = wp->w_linep;
		for (i = 0; i < wp->w_ntrows; i++) {

			/* if the line is in the window, no reframe */
			if (lp == wp->w_dotp)
				return;

			/* if we are at the end of the file, reframe */
			if (lp == wp->w_bufp->b_linep)
				break;

			/* on to the next line */
			lp = lforw(lp);
		}
	}

	/* reaching here, we need a window refresh */
	i = wp->w_force;

	/* how far back to reframe? */
	if (i > 0) {		/* only one screen worth of lines max */
		if (--i >= wp->w_ntrows)
			i = wp->w_ntrows - 1;
	} else if (i < 0) {	/* negative update???? */
		i += wp->w_ntrows;
		if (i < 0)
			i = 0;
	} else
		i = wp->w_ntrows / 2;

	/* backup to new line at top of window */
	lp = wp->w_dotp;
	while (i != 0 && lback(lp) != wp->w_bufp->b_linep) {
		--i;
		lp = lback(lp);
	}

	/* and reset the current line at top of window */
	wp->w_linep = lp;
	wp->w_flag |= WFHARD;
	wp->w_flag &= ~WFFORCE;
}

/*	updone: update the current line to the virtual screen		*/
updone(wp)
    WINDOW *wp;	/* window to update current line in */
{
	register LINE *lp;	/* line to update */
	register int sline;	/* physical screen line to update */
	register int i;

	/* search down the line we want */
	lp = wp->w_linep;
	sline = wp->w_toprow;
	while (lp != wp->w_dotp) {
		++sline;
		lp = lforw(lp);
	}

	/* and update the virtual line */
	vscreen[sline]->v_flag |= VFCHG;
	vscreen[sline]->v_flag &= ~VFREQ;
	vtmove(sline, 0);
	for (i=0; i < llength(lp); ++i)
		vtputc(lgetc(lp, i));
#if	COLOR
	vscreen[sline]->v_rfcolor = wp->w_fcolor;
	vscreen[sline]->v_rbcolor = wp->w_bcolor;
#endif
	vteeol();
}

/*	updall: update all the lines in a window on the virtual screen */
updall(wp)
    WINDOW *wp;	/* window to update lines in */
{
	register LINE *lp;	/* line to update */
	register int sline;	/* physical screen line to update */
	register int i;

	/* search down the lines, updating them */
	lp = wp->w_linep;
	sline = wp->w_toprow;
	while (sline < wp->w_toprow + wp->w_ntrows) {

		/* and update the virtual line */
		vscreen[sline]->v_flag |= VFCHG;
		vscreen[sline]->v_flag &= ~VFREQ;
		vtmove(sline, 0);
		if (lp != wp->w_bufp->b_linep) {
			/* if we are not at the end */
			for (i=0; i < llength(lp); ++i)
				vtputc(lgetc(lp, i));
			lp = lforw(lp);
		}

		/* on to the next one */
#if	COLOR
		vscreen[sline]->v_rfcolor = wp->w_fcolor;
		vscreen[sline]->v_rbcolor = wp->w_bcolor;
#endif
		vteeol();
		++sline;
	}
}

/*	updpos: update the position of the hardware cursor and handle extended
		lines. This is the only update for simple moves.	*/
updpos()
{
	register LINE *lp;
	register Char c;
	register int i;

	/* find the current row */
	lp = curwp->w_linep;
	currow = curwp->w_toprow;
	while (lp != curwp->w_dotp) {
		++currow;
		lp = lforw(lp);
	}

	/* find the current column */
	curcol = 0;
	i = 0;
	while (i < curwp->w_doto) {
		c = lgetc(lp, i++);
		if (c == '\t')
			curcol |= 0x07;
		else if (c < 0x20 || c == 0x7f)
			++curcol;
		else if (c >= 0x80 && c < 0x100)
		  curcol += 3;
#if KANJI
		else if (iskanji(c))
			++curcol;
#endif

		++curcol;
	}

	/* if extended, flag so and update the virtual line image */
	if (curcol >=  term.t_ncol - 1) {
		vscreen[currow]->v_flag |= (VFEXT | VFCHG);
		updext();
	} else
		lbound = 0;
}

/*	upddex: de-extend any line that derserves it		*/
upddex()
{
	register WINDOW *wp;
	register LINE *lp;
	register int i,j;

	wp = wheadp;

	while (wp != NULL) {
		lp = wp->w_linep;
		i = wp->w_toprow;

		while (i < wp->w_toprow + wp->w_ntrows) {
			if (vscreen[i]->v_flag & VFEXT) {
				if ((wp != curwp) || (lp != wp->w_dotp) ||
				   (curcol < term.t_ncol - 1)) {
					vtmove(i, 0);
					for (j = 0; j < llength(lp); ++j)
						vtputc(lgetc(lp, j));
					vteeol();

					/* this line no longer is extended */
					vscreen[i]->v_flag &= ~VFEXT;
					vscreen[i]->v_flag |= VFCHG;
				}
			}
			lp = lforw(lp);
			++i;
		}
		/* and onward to the next window */
		wp = wp->w_wndp;
	}
}

/*	updgar: if the screen is garbage, clear the physical screen and
		the virtual screen and force a full update		*/
updgar()
{
	register Char *txt;
	register int i,j;

	for (i = 0; i < term.t_nrow; ++i) {
		vscreen[i]->v_flag |= VFCHG;
#if	REVSTA
		vscreen[i]->v_flag &= ~VFREV;
#endif
#if	COLOR
		vscreen[i]->v_fcolor = gfcolor;
		vscreen[i]->v_bcolor = gbcolor;
#endif
		txt = pscreen[i]->v_text;
		for (j = 0; j < term.t_ncol; ++j) txt[j] = ' ';
	}

	movecursor(0, 0);		 /* Erase the screen. */
	TTeeop();
	sgarbf = FALSE;			 /* Erase-page clears */
	mpresf = FALSE;			 /* the message area. */
#if	COLOR
	mlerase();			/* needs to be cleared if colored */
#endif
}

/*	updupd: update the physical screen from the virtual screen	*/
updupd(force)
    int force;	/* forced update flag */
{
	register VIDEO *vp1;
	register int i;

	for (i = 0; i < term.t_nrow; ++i) {
		vp1 = vscreen[i];

		/* for each line that needs to be updated*/
		if ((vp1->v_flag & VFCHG) != 0) {
#if	TYPEAH
			if (force == FALSE && typahead())
				return;
#endif
			updateline(i, vp1, pscreen[i]);
		}
	}
}

/*	updext: update the extended line which the cursor is currently
		on at a column greater than the terminal width. The line
		will be scrolled right or left to let the user see where
		the cursor is
								*/
updext()
{
	register int rcursor;	/* real cursor location */
	register LINE *lp;	/* pointer to current line */
	register int j;		/* index into line */

	/* calculate what column the real cursor will end up in */
	rcursor = ((curcol - term.t_ncol) % term.t_scrsiz) + term.t_margin;
	lbound = taboff = curcol - rcursor;
#if KANJI
	/*
	 * if the second column of the screen falles into trailing part of
	 * multi-column character, lbound must be decremented to show whole
	 * the character.
	 */
	{
	    int i, col, lcol;
	    Char c;

	    lp = curwp->w_dotp;		/* line to output */
	    for (i = 0, col = 0; col <= lbound; i++) {
		lcol = col;
		if ((c = lgetc(lp, i)) == '\t') {
		  col |= 7;
		} else if (c >= 0x80 && c < 0x100) {
		  col += 3;
		} else if (c < ' ' || c == 0x7f || iskanji(c)) {
		    col++;
		}
		col++;
	    }
	    if (col > lbound+1 && iskanji(c)) taboff = lbound = lcol-1;
	}
#endif /*KANJI*/

	/* scan through the line outputing characters to the virtual screen */
	/* once we reach the left edge					*/
	vtmove(currow, -lbound);	/* start scanning offscreen */
#if !KANJI
	lp = curwp->w_dotp;		/* line to output */
#endif
	for (j=0; j<llength(lp); ++j)	/* until the end-of-line */
		vtputc(lgetc(lp, j));

	/* truncate the virtual line, restore tab offset */
	vteeol();
	taboff = 0;

	/* and put a '$' in column 1 */
	vscreen[currow]->v_text[0] = '$';
}

/*
 * Update a single line. This does not know how to use insert or delete
 * character sequences; we are using VT52 functionality. Update the physical
 * row and column variables. It does try an exploit erase to end of line.
 */
static updateline(row, vp1, vp2)
    int row;		/* row of screen to update */
    struct VIDEO *vp1;	/* virtual screen image */
    struct VIDEO *vp2;	/* physical screen image */
{
	register Char *cp1;
	register Char *cp2;
	register Char *cp3;
	register Char *cp4;
	register Char *cp5;
	register int nbflag;	/* non-blanks to the right flag? */
	int rev;		/* reverse video flag */
	int req;		/* reverse video request flag */

	/* set up pointers to virtual and physical lines */
	cp1 = &vp1->v_text[0];
	cp2 = &vp2->v_text[0];

#if	COLOR
	TTforg(vp1->v_rfcolor);
	TTbacg(vp1->v_rbcolor);
#endif

#if	REVSTA | COLOR
	/* if we need to change the reverse video status of the
	   current line, we need to re-write the entire line	 */
	rev = (vp1->v_flag & VFREV) == VFREV;
	req = (vp1->v_flag & VFREQ) == VFREQ;
	if ((rev != req)
# if	COLOR
	    || (vp1->v_fcolor != vp1->v_rfcolor)
	    || (vp1->v_bcolor != vp1->v_rbcolor)
# endif
			) {
		movecursor(row, 0);	/* Go to start of line. */
		/* set rev video if needed */
		if (rev != req)
			TTrev(req);

		/* scan through the line and dump it to the screen and
		   the virtual screen array				*/
		cp3 = &vp1->v_text[term.t_ncol];
		while (cp1 < cp3) {
# if KANJI
		    if (*cp1 != TRAILER)
# endif
			TTputc(*cp1);
		    ++ttcol;
		    *cp2++ = *cp1++;
		}
		/* turn rev video off */
		if (rev != req)
			TTrev(FALSE);

		/* update the needed flags */
		vp1->v_flag &= ~VFCHG;
		if (req)
			vp1->v_flag |= VFREV;
		else
			vp1->v_flag &= ~VFREV;
# if	COLOR
		vp1->v_fcolor = vp1->v_rfcolor;
		vp1->v_bcolor = vp1->v_rbcolor;
# endif
		return;
	}

	/* advance past any common chars at the left */
	while (cp1 != &vp1->v_text[term.t_ncol] && cp1[0] == cp2[0]) {
		++cp1;
		++cp2;
	}

/* This can still happen, even though we only call this routine on changed
 * lines. A hard update is always done when a line splits, a massive
 * change is done, or a buffer is displayed twice. This optimizes out most
 * of the excess updating. A lot of computes are used, but these tend to
 * be hard operations that do a lot of update, so I don't really care.
 */
	/* if both lines are the same, no update needs to be done */
	if (cp1 == &vp1->v_text[term.t_ncol]) {
 		vp1->v_flag &= ~VFCHG;		/* flag this line is changed */
		return;
	}

	/* find out if there is a match on the right */
	nbflag = FALSE;
	cp3 = &vp1->v_text[term.t_ncol];
	cp4 = &vp2->v_text[term.t_ncol];

	while (cp3[-1] == cp4[-1]) {
		--cp3;
		--cp4;
		if (
# if KANJI
		    cp3[0] != TRAILER &&
# endif
		    cp3[0] != ' ')		/* Note if any nonblank */
			nbflag = TRUE;		/* in right match. */
	}
# if KANJI
	while (*cp3 == TRAILER) cp3++;
# endif

	cp5 = cp3;

	/* Erase to EOL ? */
	if (nbflag == FALSE && eolexist == TRUE && (req != TRUE)) {
		while (cp5!=cp1 && cp5[-1]==' ')
			--cp5;

		if (cp3-cp5 <= 3)		/* Use only if erase is */
			cp5 = cp3;		/* fewer characters. */
	}

	movecursor(row, cp1 - &vp1->v_text[0]); /* Go to start of line. */
# if	REVSTA
	TTrev(rev);
# endif

	while (cp1 < cp5) {		/* Ordinary. */
# if KANJI
	    if (*cp1 != TRAILER)
# endif
		TTputc(*cp1);
	    ++ttcol;
	    *cp2++ = *cp1++;
	}

	if (cp5 != cp3) {		/* Erase. */
		TTeeol();
		while (cp1 != cp3)
			*cp2++ = *cp1++;
	}
# if	REVSTA
	TTrev(FALSE);
# endif
	vp1->v_flag &= ~VFCHG;		/* flag this line as updated */
#endif	/*REVSTA | COLOR*/
}

/*
 * return dash character for modeline.
 */
static Char
dash(wp)
	WINDOW *wp;
{
	return wp == curwp? '=':
#if	REVSTA
	       revexist? ' ':
#endif
	       '-';
}

/*
 * Redisplay the mode line for the window pointed to by the "wp". This is the
 * only routine that has any idea of how the modeline is formatted. You can
 * change the modeline format by hacking at this routine. Called by "update"
 * any time there is a dirty window.
 */
modeline(wp)
    WINDOW *wp;
{
    register Char *fp, *cp;
    register Char c;
    register int n;		/* cursor position count */
    register BUFFER *bp;
    register int i;		/* loop index */
    register int firstm;	/* is this the first mode? */
    static Char mline[NLINE];	/* buffer for mode line */

    n = wp->w_toprow+wp->w_ntrows;		/* Location. */
    vscreen[n]->v_flag |= VFCHG | VFREQ | VFCOL; /* Redraw next time. */
#if	COLOR
    vscreen[n]->v_rfcolor = 0;			/* black on */
    vscreen[n]->v_rbcolor = 7;			/* white.....*/
#endif
    vtmove(n, 0);				/* Seek to right line. */

    bp = wp->w_bufp;
    fp = bp->b_modeline;
    if (fp == NULL) fp = gmodeline;
    n = 0;
    while (*fp && n < term.t_ncol) {
	if ((c = *fp++) != '%') {
	    /* normal char. just echo. */
	    vtputc(c);
	    n++;
#if	KANJI
	    if (iskanji(c)) n++;
#endif
	    continue;
	}
	/* special code */
	switch (c = *fp++) {
	case '\0':
	    /* unexpected end of string */
	    fp--;
	    continue;
	default:
	    /* itself */
	    vtputc(c);
	    n++;
#if	KANJI
	    if (iskanji(c)) n++;
#endif
	    continue;
	case '*':
	    vtputc((bp->b_mode & MDVIEW)? '%':
		   (bp->b_flag & BFCHG)? '*': dash(wp));
	    n++;
	    continue;
	case '=':
	    vtputc(dash(wp));
	    n++;
	    continue;
	case '-':
	    while (n < term.t_ncol) {
		vtputc(dash(wp));
		n++;
	    }
	    continue;
	case 'b':
	case 'B':
	    cp = bp->b_bname;
	    break;
	case 'f':
	case 'F':
	    cp = bp->b_fname;
	    if (c == 'f') {
		cp = Crindex(cp, '/');
		if (cp != NULL)
		    cp ++;
		else
		    cp = bp->b_fname;
	    }
	    break;
	case 'm':
	case 'M':
	    /* build mode string */
	    firstm = TRUE;
	    cp = mline;
	    for (i = 0; modename[i] != NULL; i++) {
	      /* add in the mode flags */
	      if (bp->b_mode & (1 << i)) {
		if (firstm != TRUE) *cp++ = ' ';
		firstm = FALSE;
		if (c == 'M') {
		  (void)Cstrcpy(cp, modename[i]);
		  cp += Cstrlen(cp);
		} else {
		  *cp++ = modecode[i];
		}
	      }
	    }
	    *cp++ = '\0';
	    cp = mline;
	    break;
	case 'p':
	case 'P':
	    cp = prgname();
	    break;
	case 'v':
	case 'V':
	    cp = version();
	    break;
#if	KANJI
	case 'c':
	case 'C':
	    if (bp->b_mode & MDBINARY)
		continue;
	    cp = code_string(bp->b_code);
	    break;
#endif /* KANJI */
	case 't':
	case 'T':
	    cp = getclock();
	    break;
	case '<':
	case '>':
	    for (i = 1; i < reclevel && n < term.t_ncol; i++) {
		vtputc(c);
		n++;
	    }
	    continue;
	}
	/* put string pointed by cp */
	while (*cp && n < term.t_ncol) {
	    vtputc(*cp);
	    n++;
#if	KANJI
	    if (iskanji(*cp)) n++;
#endif
	    cp++;
	}
    }
    while (n < term.t_ncol) {
	vtputc(' ');
	n++;
    }
}

upmode()	/* update all the mode lines */
{
	register WINDOW *wp;

	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}
}

/*
 * Send a command to the terminal to move the hardware cursor to row "row"
 * and column "col". The row and column arguments are origin 0. Optimize out
 * random calls. Update "ttrow" and "ttcol".
 */
movecursor(row, col)
{
    if (row!=ttrow || col!=ttcol) {
	ttrow = row;
	ttcol = col;
        TTmove(row, col);
    }
}

/*
 * Erase the message line. This is a special routine because the message line
 * is not considered to be part of the virtual screen. It always works
 * immediately; the terminal buffer is flushed via a call to the flusher.
 */
mlerase()
{
    int i;
    
    movecursor(term.t_nrow, 0);
    if (discmd == FALSE) return;
#if	COLOR
     TTforg(7);
     TTbacg(0);
#endif
    if (eolexist == TRUE)
	    TTeeol();
    else {
	for (i = 0; i < term.t_ncol - 1; i++)
            TTputc(' ');
	movecursor(term.t_nrow, 1);	/* force the move! */
	movecursor(term.t_nrow, 0);
    }
    TTflush();
    mpresf = FALSE;
}

/*
 * Write a message into the message line. Keep track of the physical cursor
 * position. A small class of printf like format items is handled. Assumes the
 * stack grows down; this assumption is made by the "++" in the argument scan
 * loop. Set the "message line" flag TRUE.
 */
#ifdef HANDLE_VARIABLE_ARGS
# if	HAVE_STDARG
mlwrite(Char *fmt, ...)
# else
mlwrite(va_alist) va_dcl
# endif
#else
/*VARARGS1*/
mlwrite(fmt, arg)
    Char *fmt;
#endif
{
    register Char c;
#ifdef HANDLE_VARIABLE_ARGS
# if	HAVE_STDARG
    va_list vl;
# else
    va_list vl;
    Char *fmt;
# endif
#else
    register char *ap; /* really a void pointer... */
#endif

    /* if we are not currently echoing on the command line, abort this */
    if (discmd == FALSE) {
      movecursor(term.t_nrow, 0);
      return;
    }
#if	COLOR
    /* set up the proper colors for the command line */
    TTforg(7);
    TTbacg(0);
#endif
    /* if we can not erase to end-of-line, do it manually */
    if (eolexist == FALSE) {
	mlerase();
        TTflush();
    }

    movecursor(term.t_nrow, 0);
#ifdef HANDLE_VARIABLE_ARGS
# if	HAVE_STDARG
    va_start(vl, fmt);
# else
    va_start(vl);
    fmt = va_arg(vl, Char *);
# endif
#else
    ap = (char *)&arg;
#endif
    while ((c = *fmt++) != 0) {
	if (c != '%') {
            TTputc(c);
	    ++ttcol;
#if KANJI
	    if (iskanji(c)) ++ttcol;
#endif
	} else {
	    c = *fmt++;
	    switch (c) {
	    case 'd':
#ifdef HANDLE_VARIABLE_ARGS
		mlputi(va_arg(vl, int), 10);
#else
		mlputi(*(int *)ap, 10);
		ap += sizeof(int);
#endif
		break;

	    case 'o':
#ifdef HANDLE_VARIABLE_ARGS
		mlputi(va_arg(vl, int), 8);
#else
		mlputi(*(int *)ap,	8);
		ap += sizeof(int);
#endif
		break;

	    case 'x':
#ifdef HANDLE_VARIABLE_ARGS
		mlputi(va_arg(vl, int), 16);
#else
		mlputi(*(int *)ap, 16);
		ap += sizeof(int);
#endif
		break;

	    case 'D':
#ifdef HANDLE_VARIABLE_ARGS
		mlputli(va_arg(vl, long), 10);
#else
		mlputli(*(long *)ap, 10);
		ap += sizeof(long);
#endif
		break;

	    case 's':
#ifdef HANDLE_VARIABLE_ARGS
		mlputs(va_arg(vl, Char *));
#else
		mlputs(*(Char **)ap);
		ap += sizeof(Char *);
#endif
		break;

	    case 'f':
#ifdef HANDLE_VARIABLE_ARGS
		mlputf(va_arg(vl, int));
#else
		mlputf(*(int *)ap);
		ap += sizeof(int);
#endif
		break;

	    default:
                    TTputc(c);
		++ttcol;
#if KANJI
		if (iskanji(c)) ++ttcol;
#endif
	    }
	}
    }
#ifdef HANDLE_VARIABLE_ARGS /*  1993/7/6 by sanewo */
    va_end(vl);
#endif
    /* if we can, erase to the end-of-line */
    if (eolexist == TRUE)
        TTeeol();
    TTflush();
    mpresf = TRUE;
}

/*
 * Write out a string. Update the physical cursor position. This assumes that
 * the characters in the string all have width "1"; if this is not the case
 * things will get screwed up a little.
 */
/*
 * kanji characters are treated as of width "2".
 */
mlputs(s)
    Char *s;
{
    register Char c;

    while ((c = *s++) != 0) {
        TTputc(c);
	++ttcol;
#if KANJI
	if (iskanji(c)) ++ttcol;
#endif
    }
}

/*
 * Write out an integer, in the specified radix. Update the physical cursor
 * position.
 */
mlputi(i, r)
{
    register int q;
    static char hexdigits[] = "0123456789ABCDEF";

    if (i < 0) {
	i = -i;
        TTputc('-');
    }

    q = i/r;

    if (q != 0)
	mlputi(q, r);

    TTputc((Char)hexdigits[i%r]);
    ++ttcol;
}

/*
 * do the same except as a long integer.
 */
mlputli(l, r)
    long l;
{
    register long q;

    if (l < 0) {
	l = -l;
        TTputc('-');
    }

    q = l/r;

    if (q != 0)
	mlputli(q, r);

    TTputc((Char)((int)(l%r)+'0'));
    ++ttcol;
}

/*
 *	write out a scaled integer with two decimal places
 */
mlputf(s)
    int s;	/* scaled integer to output */
{
	int i;	/* integer portion of number */
	int f;	/* fractional portion of number */

	/* break it up */
	i = s / 100;
	f = s % 100;

	/* send out the integer portion */
	mlputi(i, 10);
	TTputc('.');
	TTputc((Char)((f / 10) + '0'));
	TTputc((Char)((f % 10) + '0'));
	ttcol += 3;
}	
