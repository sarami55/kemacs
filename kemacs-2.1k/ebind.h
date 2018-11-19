#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*	EBIND:		Initial default key to function bindings for
			MicroEMACS 3.7
*/

#include <signal.h>	/* to see whether SIGTSTP is defined */

/*
 * Command table.
 * This table  is *roughly* in ASCII order, left to right across the
 * characters of the command. This explains the funny location of the
 * control-X commands.
 */
KEYTAB	keytab[NBINDS] = {
	{CTLFLG|'A',		gotobol},
	{CTLFLG|'B',		backchar},
	{CTLFLG|'C',		exitrecedit},
	{CTLFLG|'D',		forwdel},
	{CTLFLG|'E',		gotoeol},
	{CTLFLG|'F',		forwchar},
	{CTLFLG|'H',		backdel},
	{CTLFLG|'I',		tab},
	{CTLFLG|'J',		indent},
	{CTLFLG|'K',		killtext},
	{CTLFLG|'L',		refreshscr},
	{CTLFLG|'M',		newline},
	{CTLFLG|'N',		forwline},
	{CTLFLG|'O',		openline},
	{CTLFLG|'P',		backline},
	{CTLFLG|'Q',		quote},
	{CTLFLG|'R',		backsearch},
	{CTLFLG|'S',		forwsearch},
	{CTLFLG|'T',		twiddle},
	{CTLFLG|'U',		unarg},
	{CTLFLG|'V',		forwpage},
	{CTLFLG|'W',		killregion},
	{CTLFLG|'X',		cex},
	{CTLFLG|'Y',		yank},
	{CTLFLG|'Z',		backpage},
	{CTLFLG|'[',		meta_pfx},
	{CTLX|CTLFLG|'B',	listbuffers},
	{CTLX|CTLFLG|'C',	quit},		/* Hard quit.		*/
#if	AEDIT
	{CTLX|CTLFLG|'D',	detab},
	{CTLX|CTLFLG|'E',	entab},
#endif
	{CTLX|CTLFLG|'F',	filefind},
	{CTLX|CTLFLG|'I',	insfile},
	{CTLX|CTLFLG|'L',	lowerregion},
	{CTLX|CTLFLG|'M',	delmode},
	{CTLX|CTLFLG|'N',	mvdnwind},
	{CTLX|CTLFLG|'O',	deblank},
	{CTLX|CTLFLG|'P',	mvupwind},
	{CTLX|CTLFLG|'R',	fileread},
	{CTLX|CTLFLG|'S',	filesave},
#if	AEDIT
	{CTLX|CTLFLG|'T',	trim},
#endif
	{CTLX|CTLFLG|'U',	upperregion},
	{CTLX|CTLFLG|'V',	viewfile},
	{CTLX|CTLFLG|'W',	filewrite},
	{CTLX|CTLFLG|'X',	swapmark},
	{CTLX|CTLFLG|'Z',	shrinkwind},
	{CTLX|'?',		deskey},
	{CTLX|'!',		spawn},
	{CTLX|'@',		pipecmd},
	{CTLX|'#',		filter},
	{CTLX|'=',		showcpos},
	{CTLX|'(',		ctlxlp},
	{CTLX|')',		ctlxrp},
	{CTLX|'^',		enlargewind},
	{CTLX|'0',		delwind},
	{CTLX|'1',		onlywind},
	{CTLX|'2',		splitwind},
	{CTLX|'A',		setvar},
	{CTLX|'B',		usebuffer},
	{CTLX|'C',		spawncli},
#ifdef SIGTSTP
	{CTLX|'D',		bktoshell},
#endif
	{CTLX|'E',		ctlxe},
	{CTLX|'F',		setfillcol},
	{CTLX|'K',		killbuffer},
	{CTLX|'M',		set_mode},
	{CTLX|'N',		filename},
	{CTLX|'O',		nextwind},
	{CTLX|'P',		prevwind},
#if	ISRCH
	{CTLX|'R',		risearch},
	{CTLX|'S',		fisearch},
#endif
	{CTLX|'W',		resize},
	{CTLX|'X',		nextbuffer},
	{CTLX|'Z',		enlargewind},
#if	WORDPRO
	{META|CTLFLG|'C',	wordcount},
#endif
#if	PROC
	{META|CTLFLG|'E',	execproc},
#endif
#if	CFENCE
	{META|CTLFLG|'F',	getfence},
#endif
	{META|CTLFLG|'H',	delbword},
	{META|CTLFLG|'K',	unbindkey},
	{META|CTLFLG|'L',	reposition},
	{META|CTLFLG|'M',	delgmode},
	{META|CTLFLG|'N',	namebuffer},
	{META|CTLFLG|'R',	qreplace},
	{META|CTLFLG|'S',	newsize},
	{META|CTLFLG|'T',	newwidth},
	{META|CTLFLG|'V',	scrnextup},
#if	WORDPRO
	{META|CTLFLG|'W',	killpara},
#endif
	{META|CTLFLG|'Z',	scrnextdw},
	{META|' ',		setmark},
	{META|'?',		help},
	{META|'!',		reposition},
	{META|'.',		setmark},
	{META|'>',		gotoeob},
	{META|'<',		gotobob},
	{META|'~',		unmark},
#if	APROP
	{META|'A',		apro},
#endif
	{META|'B',		backword},
	{META|'C',		capword},
	{META|'D',		delfword},
#if	CRYPT
	{META|'E',		setcrkey},
#endif
	{META|'F',		forwword},
	{META|'G',		gotoline},
	{META|'K',		bindtokey},
	{META|'L',		lowerword},
	{META|'M',		setgmode},
#if	WORDPRO
	{META|'N',		gotoeop},
	{META|'P',		gotobop},
	{META|'Q',		fillpara},
#endif
	{META|'R',		sreplace},
#ifdef SIGTSTP
	{META|'S',		bktoshell},
#endif
	{META|'U',		upperword},
	{META|'V',		backpage},
	{META|'W',		copyregion},
	{META|'X',		namedcmd},
	{META|'Z',		quickexit},
	{META|0x7F,		delbword},
	{SPEC|META|'A',		backline},
	{SPEC|META|'B',		forwline},
	{SPEC|META|'C',		forwchar},
	{SPEC|META|'D',		backchar},
	{SPEC|META|'P',		nextwind},

	{0x7F,			backdel},

	/* special internal bindings */
	{SPEC|META|'W',		wrapword},	/* called on word wrap */
#if	CLOCK
	{SPEC|META|'T',		updmode},	/* called on timer signal */
#endif

	{0,			NULL}
};
