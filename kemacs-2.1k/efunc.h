#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*	EFUNC.H:	MicroEMACS function declarations and names

		This file list all the C code functions used by MicroEMACS
	and the names to use to bind keys to them. To add functions,
	declare it here in both the extern function list and the name
	binding table.

*/

#include <signal.h>	/* to see whether SIGTSTP is defined */

/*	External function declarations		*/

extern	int	ctrlg();		/* Abort out of things		*/
extern	int	quit();			/* Quit				*/
extern	int	ctlxlp();		/* Begin macro			*/
extern	int	ctlxrp();		/* End macro			*/
extern	int	ctlxe();		/* Execute macro		*/
extern	int	fileread();		/* Get a file, read only	*/
extern	int	filefind();		/* Get a file, read write	*/
extern	int	filewrite();		/* Write a file			*/
extern	int	filesave();		/* Save current file		*/
extern	int	filename();		/* Adjust file name		*/
extern	int	gotobol();		/* Move to start of line	*/
extern	int	forwchar();		/* Move forward by characters	*/
extern	int	gotoeol();		/* Move to end of line		*/
extern	int	backchar();		/* Move backward by characters	*/
extern	int	forwline();		/* Move forward by lines	*/
extern	int	backline();		/* Move backward by lines	*/
extern	int	forwpage();		/* Move forward by pages	*/
extern	int	backpage();		/* Move backward by pages	*/
extern	int	gotobob();		/* Move to start of buffer	*/
extern	int	gotoeob();		/* Move to end of buffer	*/
extern	int	setfillcol();		/* Set fill column.		*/
extern	int	setmark();		/* Set mark			*/
extern	int	swapmark();		/* Swap "." and mark		*/
extern	int	forwsearch();		/* Search forward		*/
extern	int	backsearch();		/* Search backwards		*/
extern	int	sreplace();		/* search and replace		*/
extern	int	qreplace();		/* search and replace w/query	*/
extern	int	showcpos();		/* Show the cursor position	*/
extern	int	nextwind();		/* Move to the next window	*/
extern	int	prevwind();		/* Move to the previous window	*/
extern	int	onlywind();		/* Make current window only one */
extern	int	splitwind();		/* Split current window		*/
extern	int	mvdnwind();		/* Move window down		*/
extern	int	mvupwind();		/* Move window up		*/
extern	int	enlargewind();		/* Enlarge display window.	*/
extern	int	shrinkwind();		/* Shrink window.		*/
extern	int	listbuffers();		/* Display list of buffers	*/
extern	int	usebuffer();		/* Switch a window to a buffer	*/
extern	int	killbuffer();		/* Make a buffer go away.	*/
extern	int	reposition();		/* Reposition window		*/
extern	int	refreshscr();		/* Refresh the screen		*/
extern	int	twiddle();		/* Twiddle characters		*/
extern	int	tab();			/* Insert tab			*/
extern	int	newline();		/* Insert CR-LF			*/
extern	int	indent();		/* Insert CR-LF, then indent	*/
extern	int	openline();		/* Open up a blank line		*/
extern	int	deblank();		/* Delete blank lines		*/
extern	int	quote();		/* Insert literal		*/
extern	int	backword();		/* Backup by words		*/
extern	int	forwword();		/* Advance by words		*/
extern	int	forwdel();		/* Forward delete		*/
extern	int	backdel();		/* Backward delete		*/
extern	int	killtext();		/* Kill forward			*/
extern	int	yank();			/* Yank back from killbuffer.	*/
extern	int	upperword();		/* Upper case word.		*/
extern	int	lowerword();		/* Lower case word.		*/
extern	int	upperregion();		/* Upper case region.		*/
extern	int	lowerregion();		/* Lower case region.		*/
extern	int	capword();		/* Initial capitalize word.	*/
extern	int	delfword();		/* Delete forward word.		*/
extern	int	delbword();		/* Delete backward word.	*/
extern	int	killregion();		/* Kill region.			*/
extern	int	copyregion();		/* Copy region to kill buffer.	*/
extern	int	spawncli();		/* Run CLI in a subjob.		*/
extern	int	spawn();		/* Run a command in a subjob.	*/
#ifdef SIGTSTP
extern	int	bktoshell();		/* suspend emacs to parent shell*/
extern SIGRET_T rtfrmshell();		/* return from a suspended state*/
#endif
extern	int	quickexit();		/* low keystroke style exit.	*/
extern	int	set_mode();		/* set an editor mode		*/
extern	int	delmode();		/* delete a mode		*/
extern	int	gotoline();		/* go to a numbered line	*/
extern	int	namebuffer();		/* rename the current buffer	*/
#if	WORDPRO
extern	int	gotobop();		/* go to beginning/paragraph	*/
extern	int	gotoeop();		/* go to end/paragraph		*/
extern	int	fillpara();		/* fill current paragraph	*/
#endif
extern	int	help();			/* get the help file here	*/
extern	int	deskey();		/* describe a key's binding	*/
extern	int	viewfile();		/* find a file in view mode	*/
extern	int	insfile();		/* insert a file		*/
extern	int	scrnextup();		/* scroll next window back	*/
extern	int	scrnextdw();		/* scroll next window down	*/
extern	int	bindtokey();		/* bind a function to a key	*/
extern	int	unbindkey();		/* unbind a key's function	*/
extern	int	namedcmd();		/* execute named command	*/
extern	int	desbind();		/* describe bindings		*/
extern	int	execcmd();		/* execute a command line	*/
extern	int	execbuf();		/* exec commands from a buffer	*/
extern	int	execfile();		/* exec commands from a file	*/
extern	int	nextbuffer();		/* switch to the next buffer	*/
#if	WORDPRO
extern	int	killpara();		/* kill the current paragraph	*/
#endif
extern	int	setgmode();		/* set a global mode		*/
extern	int	delgmode();		/* delete a global mode		*/
extern	int	insspace();		/* insert a space forword	*/
extern	int	forwhunt();		/* hunt forward for next match	*/
extern	int	backhunt();		/* hunt backwards for next match*/
extern	int	pipecmd();		/* pipe command into buffer	*/
extern	int	filter();		/* filter buffer through dos	*/
extern	int	delwind();		/* delete the current window	*/
extern	int	cbufn();
extern	int	cbuf1();		/* execute numbered comd buffer */
extern	int	cbuf2();
extern	int	cbuf3();
extern	int	cbuf4();
extern	int	cbuf5();
extern	int	cbuf6();
extern	int	cbuf7();
extern	int	cbuf8();
extern	int	cbuf9();
extern	int	cbuf10();
extern	int	cbuf11();
extern	int	cbuf12();
extern	int	cbuf13();
extern	int	cbuf14();
extern	int	cbuf15();
extern	int	cbuf16();
extern	int	cbuf17();
extern	int	cbuf18();
extern	int	cbuf19();
extern	int	cbuf20();
extern	int	cbuf21();
extern	int	cbuf22();
extern	int	cbuf23();
extern	int	cbuf24();
extern	int	cbuf25();
extern	int	cbuf26();
extern	int	cbuf27();
extern	int	cbuf28();
extern	int	cbuf29();
extern	int	cbuf30();
extern	int	cbuf31();
extern	int	cbuf32();
extern	int	cbuf33();
extern	int	cbuf34();
extern	int	cbuf35();
extern	int	cbuf36();
extern	int	cbuf37();
extern	int	cbuf38();
extern	int	cbuf39();
extern	int	cbuf40();
extern	int	storemac();		/* store text for macro		*/
extern	int	resize();		/* resize current window	*/
extern	int	clrmes();		/* clear the message line	*/
extern	int	meta_pfx();		/* meta prefix dummy function	*/
extern	int	cex();			/* ^X prefix dummy function	*/
extern	int	unarg();		/* ^U repeat arg dummy function	*/
extern	int	istring();		/* insert string in text	*/
extern	int	unmark();		/* unmark current buffer	*/
#if	ISRCH
extern	int	fisearch();		/* forward incremental search	*/
extern	int	risearch();		/* reverse incremental search	*/
#endif
#if	WORDPRO
extern	int	wordcount();		/* count words in region	*/
#endif
extern	int	savewnd();		/* save current window		*/
extern	int	restwnd();		/* restore current window	*/
extern	int	upscreen();		/* force screen update		*/
extern	int	writemsg();		/* write text on message line	*/
extern	int	clearkill();		/* clear kill buffer		*/
extern	int	noop();			/* do nothing			*/
#if	FLABEL
extern	int	fnclabel();		/* set function key label	*/
#endif
#if	APROP
extern	int	apro();			/* apropos fuction		*/
#endif
#if	CRYPT
extern	int	setcrkey();		/* set encryption key		*/
#endif
extern	int	wrapword();		/* wordwrap function		*/
#if	CFENCE
extern	int	getfence();		/* move cursor to a matching fence */
#endif
extern	int	newsize();		/* change the current screen size */
extern	int	setvar();		/* set a variables value */
extern	int	newwidth();		/* change the current screen width */
extern	int	changewd();		/* change current working directory */
#if AUTOMODE
extern	int	automode();		/* set file pattern and mode */
#endif
extern	int	recedit();		/* enter recursive edit */
extern	int	exitrecedit();		/* exit recursive edit */
#if	AEDIT
extern	int	trim();			/* trim whitespace from end of line */
extern	int	detab();		/* detab rest of line */
extern	int	entab();		/* entab rest of line */
#endif
#if	PROC
extern	int	storeproc();		/* store names procedure */
extern	int	execproc();		/* execute procedure */
#endif
extern	int	updmode();		/* update modeline */
extern	int	millisleep();		/* sleep (added by N.Nide) */

/*	Name to function binding table

		This table gives the names of all the bindable functions
	end their C function address. These are used for the bind-to-key
	function.
*/

NBIND	names[] = {
	{CSTR(&Cxstr[3770]),		ctrlg},
	{CSTR(&Cxstr[3784]),		set_mode},
	{CSTR(&Cxstr[3793]),	setgmode},
#if	APROP
	{CSTR(&Cxstr[3809]),		apro},
#endif
#if AUTOMODE
	{CSTR(&Cxstr[3817]),	automode},
#endif
	{CSTR(&Cxstr[3836]),	backchar},
	{CSTR(&Cxstr[3855]),		ctlxlp},
	{CSTR(&Cxstr[3867]),	gotobob},
	{CSTR(&Cxstr[3885]),	gotobol},
	{CSTR(&Cxstr[3903]),		bindtokey},
	{CSTR(&Cxstr[3915]),	showcpos},
	{CSTR(&Cxstr[3931]),	lowerregion},
	{CSTR(&Cxstr[3949]),	upperregion},
	{CSTR(&Cxstr[3967]),	capword},
	{CSTR(&Cxstr[3988]),	lowerword},
	{CSTR(&Cxstr[4004]),	upperword},
	{CSTR(&Cxstr[4020]),			changewd},
	{CSTR(&Cxstr[4023]),	filename},
	{CSTR(&Cxstr[4040]),	newsize},
	{CSTR(&Cxstr[4059]),	newwidth},
	{CSTR(&Cxstr[4079]),	refreshscr},
	{CSTR(&Cxstr[4096]),	clearkill},
	{CSTR(&Cxstr[4114]),	clrmes},
	{CSTR(&Cxstr[4133]),		copyregion},
#if	WORDPRO
	{CSTR(&Cxstr[4145]),		wordcount},
#endif
	{CSTR(&Cxstr[4157]),		cex},
	{CSTR(&Cxstr[4169]),	deblank},
	{CSTR(&Cxstr[4188]),		killbuffer},
	{CSTR(&Cxstr[4202]),		delmode},
	{CSTR(&Cxstr[4214]),	delgmode},
	{CSTR(&Cxstr[4233]),	forwdel},
	{CSTR(&Cxstr[4255]),	delfword},
	{CSTR(&Cxstr[4272]),	onlywind},
	{CSTR(&Cxstr[4293]),	backdel},
	{CSTR(&Cxstr[4319]),	delbword},
	{CSTR(&Cxstr[4340]),		delwind},
	{CSTR(&Cxstr[4354]),	desbind},
	{CSTR(&Cxstr[4372]),		deskey},
#if	AEDIT
	{CSTR(&Cxstr[4385]),		detab},
#endif
	{CSTR(&Cxstr[4396]),		ctlxrp},
	{CSTR(&Cxstr[4406]),		gotoeob},
	{CSTR(&Cxstr[4418]),		gotoeol},
#if	AEDIT
	{CSTR(&Cxstr[4430]),		entab},
#endif
	{CSTR(&Cxstr[4441]),	swapmark},
	{CSTR(&Cxstr[4465]),	execbuf},
	{CSTR(&Cxstr[4480]),	execcmd},
	{CSTR(&Cxstr[4501]),		execfile},
	{CSTR(&Cxstr[4514]),		ctlxe},
	{CSTR(&Cxstr[4528]),	cbufn},
	{CSTR(&Cxstr[4544]),	cbuf1},
	{CSTR(&Cxstr[4560]),	cbuf2},
	{CSTR(&Cxstr[4576]),	cbuf3},
	{CSTR(&Cxstr[4592]),	cbuf4},
	{CSTR(&Cxstr[4608]),	cbuf5},
	{CSTR(&Cxstr[4624]),	cbuf6},
	{CSTR(&Cxstr[4640]),	cbuf7},
	{CSTR(&Cxstr[4656]),	cbuf8},
	{CSTR(&Cxstr[4672]),	cbuf9},
	{CSTR(&Cxstr[4688]),	cbuf10},
	{CSTR(&Cxstr[4705]),	cbuf11},
	{CSTR(&Cxstr[4722]),	cbuf12},
	{CSTR(&Cxstr[4739]),	cbuf13},
	{CSTR(&Cxstr[4756]),	cbuf14},
	{CSTR(&Cxstr[4773]),	cbuf15},
	{CSTR(&Cxstr[4790]),	cbuf16},
	{CSTR(&Cxstr[4807]),	cbuf17},
	{CSTR(&Cxstr[4824]),	cbuf18},
	{CSTR(&Cxstr[4841]),	cbuf19},
	{CSTR(&Cxstr[4858]),	cbuf20},
	{CSTR(&Cxstr[4875]),	cbuf21},
	{CSTR(&Cxstr[4892]),	cbuf22},
	{CSTR(&Cxstr[4909]),	cbuf23},
	{CSTR(&Cxstr[4926]),	cbuf24},
	{CSTR(&Cxstr[4943]),	cbuf25},
	{CSTR(&Cxstr[4960]),	cbuf26},
	{CSTR(&Cxstr[4977]),	cbuf27},
	{CSTR(&Cxstr[4994]),	cbuf28},
	{CSTR(&Cxstr[5011]),	cbuf29},
	{CSTR(&Cxstr[5028]),	cbuf30},
	{CSTR(&Cxstr[5045]),	cbuf31},
	{CSTR(&Cxstr[5062]),	cbuf32},
	{CSTR(&Cxstr[5079]),	cbuf33},
	{CSTR(&Cxstr[5096]),	cbuf34},
	{CSTR(&Cxstr[5113]),	cbuf35},
	{CSTR(&Cxstr[5130]),	cbuf36},
	{CSTR(&Cxstr[5147]),	cbuf37},
	{CSTR(&Cxstr[5164]),	cbuf38},
	{CSTR(&Cxstr[5181]),	cbuf39},
	{CSTR(&Cxstr[5198]),	cbuf40},
	{CSTR(&Cxstr[5215]),	namedcmd},
#if	PROC
	{CSTR(&Cxstr[5237]),	execproc},
#endif
	{CSTR(&Cxstr[5255]),		quit},
	{CSTR(&Cxstr[5266]),	exitrecedit},
#if	WORDPRO
	{CSTR(&Cxstr[5286]),	fillpara},
#endif
	{CSTR(&Cxstr[5301]),		filter},
	{CSTR(&Cxstr[5315]),		filefind},
	{CSTR(&Cxstr[5325]),	forwchar},
	{CSTR(&Cxstr[5343]),		gotoline},
#if	CFENCE
	{CSTR(&Cxstr[5353]),	getfence},
#endif
	{CSTR(&Cxstr[5373]),		enlargewind},
	{CSTR(&Cxstr[5385]),		tab},
	{CSTR(&Cxstr[5396]),		forwhunt},
	{CSTR(&Cxstr[5409]),		backhunt},
	{CSTR(&Cxstr[5423]),			help},
	{CSTR(&Cxstr[5428]),		spawncli},
#if	ISRCH
	{CSTR(&Cxstr[5436]),	fisearch},
#endif
	{CSTR(&Cxstr[5455]),		insfile},
	{CSTR(&Cxstr[5467]),		insspace},
	{CSTR(&Cxstr[5480]),		istring},
#if	WORDPRO
	{CSTR(&Cxstr[5494]),	killpara},
#endif
	{CSTR(&Cxstr[5509]),		killregion},
	{CSTR(&Cxstr[5521]),	killtext},
#if	FLABEL
	{CSTR(&Cxstr[5541]),	fnclabel},
#endif
	{CSTR(&Cxstr[5560]),		listbuffers},
	{CSTR(&Cxstr[5573]),		meta_pfx},
	{CSTR(&Cxstr[5585]),	mvdnwind},
	{CSTR(&Cxstr[5602]),	mvupwind},
	{CSTR(&Cxstr[5617]),		namebuffer},
	{CSTR(&Cxstr[5629]),		newline},
	{CSTR(&Cxstr[5637]),	indent},
	{CSTR(&Cxstr[5656]),		nextbuffer},
	{CSTR(&Cxstr[5668]),		forwline},
	{CSTR(&Cxstr[5678]),		forwpage},
#if	WORDPRO
	{CSTR(&Cxstr[5688]),	gotoeop},
#endif
	{CSTR(&Cxstr[5703]),		nextwind},
	{CSTR(&Cxstr[4262]),		forwword},
	{CSTR(&Cxstr[5715]),			noop},
	{CSTR(&Cxstr[5721]),		openline},
	{CSTR(&Cxstr[5731]),		pipecmd},
	{CSTR(&Cxstr[5744]),		backline},
	{CSTR(&Cxstr[5758]),		backpage},
#if	WORDPRO
	{CSTR(&Cxstr[5772]),	gotobop},
#endif
	{CSTR(&Cxstr[5791]),	prevwind},
	{CSTR(&Cxstr[4326]),		backword},
	{CSTR(&Cxstr[5807]),	qreplace},
	{CSTR(&Cxstr[5828]),		quickexit},
	{CSTR(&Cxstr[5839]),	quote},
	{CSTR(&Cxstr[5855]),		fileread},
	{CSTR(&Cxstr[5271]),	recedit},
	{CSTR(&Cxstr[5865]),	reposition},
	{CSTR(&Cxstr[5880]),		resize},
	{CSTR(&Cxstr[5894]),	restwnd},
	{CSTR(&Cxstr[5813]),	sreplace},
#if	ISRCH
	{CSTR(&Cxstr[5909]),
					risearch},
#endif
#if	PROC
	{CSTR(&Cxstr[5936]),			execproc},
#endif
	{CSTR(&Cxstr[5940]),		filesave},
	{CSTR(&Cxstr[5950]),		savewnd},
	{CSTR(&Cxstr[5962]),	scrnextdw},
	{CSTR(&Cxstr[5986]),	scrnextup},
 	{CSTR(&Cxstr[6008]),	forwsearch},
	{CSTR(&Cxstr[6023]),	backsearch},
	{CSTR(&Cxstr[6038]),		usebuffer},
	{CSTR(&Cxstr[6052]),			setvar},
#if	CRYPT
	{CSTR(&Cxstr[6056]),	setcrkey},
#endif
	{CSTR(&Cxstr[6075]),	setfillcol},
	{CSTR(&Cxstr[6091]),		setmark},
	{CSTR(&Cxstr[6100]),		spawn},
	{CSTR(&Cxstr[6114]),		shrinkwind},
	{CSTR(&Cxstr[6128]),	millisleep}, /* Added by N.Nide */
	{CSTR(&Cxstr[6144]),	splitwind},
	{CSTR(&Cxstr[6165]),		storemac},
#if	PROC
	{CSTR(&Cxstr[6177]),	storeproc},
#endif
#ifdef SIGTSTP
	{CSTR(&Cxstr[6193]),		bktoshell},
#endif
	{CSTR(&Cxstr[6207]),	twiddle},
#if	AEDIT
	{CSTR(&Cxstr[6228]),		trim},
#endif
	{CSTR(&Cxstr[6238]),		unbindkey},
	{CSTR(&Cxstr[6249]),	unarg},
	{CSTR(&Cxstr[6268]),		unmark},
	{CSTR(&Cxstr[6282]),	updmode},
	{CSTR(&Cxstr[6298]),		upscreen},
	{CSTR(&Cxstr[6312]),		viewfile},
	{CSTR(&Cxstr[6322]),		wrapword},
	{CSTR(&Cxstr[6332]),		filewrite},
	{CSTR(&Cxstr[6343]),		writemsg},
	{CSTR(&Cxstr[6357]),			yank},

	{CSTR(&Cxstr[6]),			NULL}
};
