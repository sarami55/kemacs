#ifndef _CX_DEF_
extern short Cxstr[];
#define _CX_DEF_
#endif /*_CX_DEF_*/
/*
 * The functions in this file implement commands that search in the forward
 * and backward directions. There are no special characters in the search
 * strings. Probably should have a regular expression search, or something
 * like that.
 *
 * Aug. 1986 John M. Gamble:
 *	Made forward and reverse search use the same scan routine.
 *
 *	Added a limited number of regular expressions - 'any',
 *	'character class', 'closure', 'beginning of line', and
 *	'end of line'.
 *
 *	Replacement metacharacters will have to wait for a re-write of
 *	the replaces function, and a new variation of ldelete().
 *
 *	For those curious as to my references, i made use of
 *	Kernighan & Plauger's "Software Tools."
 *	I deliberately did not look at any published grep or editor
 *	source (aside from this one) for inspiration.  I did make use of
 *	Allen Hollub's bitmap routines as published in Doctor Dobb's Journal,
 *	June, 1985 and modified them for the limited needs of character class
 *	matching.  Any inefficiences, bugs, stupid coding examples, etc.,
 *	are therefore my own responsibility.
 *
 * April 1987: John M. Gamble
 *	Deleted the "if (n == 0) n = 1;" statements in front of the
 *	search/hunt routines.  Since we now use a do loop, these
 *	checks are unnecessary.  Consolidated common code into the
 *	function delins().  Renamed global mclen matchlen,
 *	and added the globals matchline, matchoff, patmatch, and
 *	mlenold.
 *	This gave us the ability to unreplace regular expression searches,
 *	and to put the matched string into an evironment variable.
 *	SOON TO COME: Meta-replacement characters!
 *
 *	25-apr-87	DML
 *	- cleaned up an unneccessary if/else in forwsearch() and
 *	  backsearch()
 *	- savematch() failed to malloc room for the terminating byte
 *	  of the match string (stomp...stomp...). It does now. Also
 *	  it now returns gracefully if malloc fails
 */

#include	"ecomm.h"
#include	"estruct.h"
#include	"edef.h"

static int amatch(), readpattern(), replaces(), nextch(), mcstr(), mceq(),
	   cclmake(), biteq(), setbit();
static Char adjcase();
static BITMAP clearbits();

/*
 * forwsearch -- Search forward.  Get a search string from the user, and
 *	search for the string.  If found, reset the "." to be just after
 *	the match string, and (perhaps) repaint the display.
 */

forwsearch(f, n)
{
	register int status = TRUE;

	/* If n is negative, search backwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return(backsearch(f, -n));

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	if ((status = readpattern(
#if KANJI
				kterminal? CSTR(&Cxstr[6956]):
#endif
				CSTR(&Cxstr[6959]),
				pat, TRUE)) == TRUE) {
		do {
#if	MAGIC
			if (magical && (curwp->w_bufp->b_mode & MDMAGIC))
				status = mcscanner(mcpat, FORWARD, PTEND);
			else
#endif
				status = scanner(pat, FORWARD, PTEND);
		} while ((--n > 0) && status);

		/* Save away the match, or complain
		 * if not there.
		 */
		if (status == TRUE)
		  savematch();
		else
		  mlwrite(
#if KMSGS
			  kterminal? CSTR(&Cxstr[2091]):
#endif
			  CSTR(&Cxstr[6966]));
	}
	return(status);
}

/*
 * forwhunt -- Search forward for a previously acquired search string.
 *	If found, reset the "." to be just after the match string,
 *	and (perhaps) repaint the display.
 */
forwhunt(f, n)
{
	register int status = TRUE;

	/* Resolve the repeat count.
	 */
	if (n < 0)		/* search backwards */
		return(backhunt(f, -n));

	/* Make sure a pattern exists, or that we didn't switch
	 * into MAGIC mode until after we entered the pattern.
	 */
	if (pat[0] == '\0') {
		mlwrite(
#if KMSGS
			kterminal? CSTR(&Cxstr[6976]):
#endif
			CSTR(&Cxstr[6992]));
		return FALSE;
	}
#if	MAGIC
	if (curwp->w_bufp->b_mode & MDMAGIC && mcpat[0].mc_type == MCNIL) {
		if (!mcstr())
			return FALSE;
	}
#endif

	/* Search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	do {
#if	MAGIC
		if ((magical && curwp->w_bufp->b_mode & MDMAGIC) != 0)
			status = mcscanner(mcpat, FORWARD, PTEND);
		else
#endif
			status = scanner(pat, FORWARD, PTEND);
	} while ((--n > 0) && status);

	/* Save away the match, or complain
	 * if not there.
	 */
	if (status == TRUE)
	  savematch();
	else
	  mlwrite(
#if KMSGS
		  kterminal? CSTR(&Cxstr[2091]):
#endif
		  CSTR(&Cxstr[6966]));

	return(status);
}

/*
 * backsearch -- Reverse search.  Get a search string from the user, and
 *	search, starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
backsearch(f, n)
{
	register int status = TRUE;

	/* If n is negative, search forwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return(forwsearch(f, -n));

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	if ((status = readpattern(
#if KMSGS
				  kterminal? CSTR(&Cxstr[7007]):
#endif
				  CSTR(&Cxstr[7013]),
				  pat, TRUE)) == TRUE) {
		do {
#if	MAGIC
			if ((magical && curwp->w_bufp->b_mode & MDMAGIC) != 0)
				status = mcscanner(tapcm, REVERSE, PTBEG);
			else
#endif
				status = scanner(tap, REVERSE, PTBEG);
		} while ((--n > 0) && status);

		/* Save away the match, or complain
		 * if not there.
		 */
		if (status == TRUE)
		  savematch();
		else
		  mlwrite(
#if KMSGS
			  kterminal? CSTR(&Cxstr[2091]):
#endif
			  CSTR(&Cxstr[6966]));
	}
	return(status);
}

/*
 * backhunt -- Reverse search for a previously acquired search string,
 *	starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
backhunt(f, n)
{
	register int	status = TRUE;

	if (n < 0)
		return(forwhunt(f, -n));

	/* Make sure a pattern exists, or that we didn't switch
	 * into MAGIC mode until after we entered the pattern.
	 */
	if (tap[0] == '\0') {
		mlwrite(
#if KMSGS
			kterminal? CSTR(&Cxstr[6976]):
#endif
			CSTR(&Cxstr[6992]));
		return FALSE;
	}
#if	MAGIC
	if (curwp->w_bufp->b_mode & MDMAGIC && tapcm[0].mc_type == MCNIL) {
		if (!mcstr())
			return FALSE;
	}
#endif

	/* Go search for it for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	do {
#if	MAGIC
		if ((magical && curwp->w_bufp->b_mode & MDMAGIC) != 0)
			status = mcscanner(tapcm, REVERSE, PTBEG);
		else
#endif
			status = scanner(tap, REVERSE, PTBEG);
	} while ((--n > 0) && status);

	/* Save away the match, or complain
	 * if not there.
	 */
	if (status == TRUE)
	  savematch();
	else
	  mlwrite(
#if KMSGS
		  kterminal? CSTR(&Cxstr[2091]):
#endif
		  CSTR(&Cxstr[6966]));

	return(status);
}

#if	MAGIC
/*
 * mcscanner -- Search for a meta-pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 */
int
mcscanner(mcpatrn, direct, beg_or_end)
	MC	*mcpatrn;	/* pointer into pattern */
	int	direct;		/* which way to go.*/
	int	beg_or_end;	/* put point at beginning or end of pattern.*/
{
	LINE *curline;			/* current line during scan */
	int curoff;			/* position within current line */

	/* If we are going in reverse, then the 'end' is actually
	 * the beginning of the pattern.  Toggle it.
	 */
	beg_or_end ^= direct;

	/*
	 * Save the old matchlen length, in case it is
	 * horribly different (closure) from the old length.
	 * This is terribly important for query-replace undo
	 * command.
	 */
	mlenold = matchlen;

	/* Setup local scan pointers to global ".".
	 */
	curline = curwp->w_dotp;
	curoff	= curwp->w_doto;

	/* Scan each character until we hit the head link record.
	 */
	while (!boundry(curline, curoff, direct)) {
		/* Save the current position in case we need to
		 * restore it on a match, and initialize matchlen to
		 * zero in case we are doing a search for replacement.
		 */
		matchline = curline;
		matchoff = curoff;
		matchlen = 0;

		if (amatch(mcpatrn, direct, &curline, &curoff)) {
			/* A SUCCESSFULL MATCH!!!
			 * reset the global "." pointers.
			 */
			if (beg_or_end == PTEND) { /* at end of string */
				curwp->w_dotp = curline;
				curwp->w_doto = curoff;
			} else { /* at beginning of string */
				curwp->w_dotp = matchline;
				curwp->w_doto = matchoff;
			}

			curwp->w_flag |= WFMOVE; /* flag that we have moved */
			return TRUE;
		}

		/* Advance the cursor.
		 */
		(void)nextch(&curline, &curoff, direct);
	}

	return FALSE;	/* We could not find a match.*/
}

/*
 * amatch -- Search for a meta-pattern in either direction.  Based on the
 *	recursive routine amatch() (for "anchored match") in
 *	Kernighan & Plauger's "Software Tools".
 */
static int
amatch(mcptr, direct, pcwline, pcwoff)
	register MC	*mcptr; 	/* string to scan for */
	int		direct;		/* which way to go.*/
	LINE		**pcwline;	/* current line during scan */
	int		*pcwoff;	/* position within current line */
{
	register Char c;		/* character at current position */
	LINE *curline;			/* current line during scan */
	int curoff;			/* position within current line */
	int nchars;

	/* Set up local scan pointers to ".", and get
	 * the current character.  Then loop around
	 * the pattern pointer until success or failure.
	 */
	curline = *pcwline;
	curoff = *pcwoff;

	/* The beginning-of-line and end-of-line metacharacters
	 * do not compare against characters, they compare
	 * against positions.
	 * BOL is guaranteed to be at the start of the pattern
	 * for forward searches, and at the end of the pattern
	 * for reverse searches.  The reverse is true for EOL.
	 * So, for a start, we check for them on entry.
	 */
	if (mcptr->mc_type == BOL) {
		if (curoff != 0)
			return FALSE;
		mcptr++;
	}

	if (mcptr->mc_type == EOL) {
		if (curoff != llength(curline))
			return FALSE;
		mcptr++;
	}

	while (mcptr->mc_type != MCNIL) {
		c = nextch(&curline, &curoff, direct);

		if (mcptr->mc_type & CLOSURE) {
			/* Try to match as many characters as possible
			 * against the current meta-character.	A
			 * newline never matches a closure.
			 */
			nchars = 0;
			while (c != '\n' && mceq(c, mcptr)) {
				c = nextch(&curline, &curoff, direct);
				nchars++;
			}

			/* We are now at the character that made us
			 * fail.  Try to match the rest of the pattern.
			 * Shrink the closure by one for each failure.
			 * Since closure matches *zero* or more occurences
			 * of a pattern, a match may start even if the
			 * previous loop matched no characters.
			 */
			mcptr++;

			for (;;) {
				c = nextch(&curline, &curoff, direct ^ REVERSE);

				if (amatch(mcptr, direct, &curline, &curoff)) {
					matchlen += nchars;
					goto success;
				}

				if (nchars-- == 0)
					return FALSE;
			}
		} else { /* Not closure.*/
			/* The only way we'd get a BOL metacharacter
			 * at this point is at the end of the reversed pattern.
			 * The only way we'd get an EOL metacharacter
			 * here is at the end of a regular pattern.
			 * So if we match one or the other, and are at
			 * the appropriate position, we are guaranteed success
			 * (since the next pattern character has to be MCNIL).
			 * Before we report success, however, we back up by
			 * one character, so as to leave the cursor in the
			 * correct position.  For example, a search for ")$"
			 * will leave the cursor at the end of the line, while
			 * a search for ")<NL>" will leave the cursor at the
			 * beginning of the next line.	This follows the
			 * notion that the meta-character '$' (and likewise
			 * '^') match positions, not characters.
			 */
			if (mcptr->mc_type == BOL) {
				if (curoff == llength(curline)) {
					c = nextch(&curline, &curoff,
						   direct ^ REVERSE);
					goto success;
				} else
					return FALSE;
			}

			if (mcptr->mc_type == EOL) {
				if (curoff == 0) {
					c = nextch(&curline, &curoff,
						   direct ^ REVERSE);
					goto success;
				} else
					return FALSE;
			}

			/* Neither BOL nor EOL, so go through
			 * the meta-character equal function.
			 */
			if (!mceq(c, mcptr))
				return FALSE;
		}

		/* Increment the length counter and
		 * advance the pattern pointer.
		 */
		matchlen++;
		mcptr++;
	}			/* End of mcptr loop.*/

	/* A SUCCESSFULL MATCH!!!
	 * Reset the "." pointers.
	 */
    success:
	*pcwline = curline;
	*pcwoff	 = curoff;

	return TRUE;
}
#endif

/*
 * scanner -- Search for a pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 */
int
scanner(patrn, direct, beg_or_end)
	Char	*patrn;		/* string to scan for */
	int	direct;		/* which way to go.*/
	int	beg_or_end;	/* put point at beginning or end of pattern.*/
{
	register Char c;		/* character at current position */
	register Char *patptr;		/* pointer into pattern */
	LINE *curline;			/* current line during scan */
	int curoff;			/* position within current line */
	LINE *scanline;		/* current line during matching */
	int scanoff;			/* position in matching line */

	/* If we are going in reverse, then the 'end' is actually
	 * the beginning of the pattern.  Toggle it.
	 */
	beg_or_end ^= direct;

	/* Set up local pointers to global ".".
	 */
	curline = curwp->w_dotp;
	curoff = curwp->w_doto;

	/* Scan each character until we hit the head link record.
	 */
	while (!boundry(curline, curoff, direct)) {
		/* Save the current position in case we match
		 * the search string at this point.
		 */
		matchline = curline;
		matchoff = curoff;

		/* Get the character resolving newlines, and
		 * test it against first char in pattern.
		 */
		c = nextch(&curline, &curoff, direct);

		if (eq(c, patrn[0])) { /* if we find it..*/
			/* Setup scanning pointers.
			 */
			scanline = curline;
			scanoff = curoff;
			patptr = patrn;

			/* Scan through the pattern for a match.
			 */
			while (*++patptr != '\0') {
				c = nextch(&scanline, &scanoff, direct);

				if (!eq(c, *patptr))
						goto fail;
			}

			/* A SUCCESSFULL MATCH!!!
			 * reset the global "." pointers
			 */
			if (beg_or_end == PTEND) { /* at end of string */
				curwp->w_dotp = scanline;
				curwp->w_doto = scanoff;
			} else { /* at beginning of string */
				curwp->w_dotp = matchline;
				curwp->w_doto = matchoff;
			}

			curwp->w_flag |= WFMOVE; /* Flag that we have moved.*/
			return TRUE;

		}
	    fail:;	/* continue to search */
	}

	return FALSE;	/* We could not find a match */
}

/*
 * eq -- Compare two characters.  The "bc" comes from the buffer, "pc"
 *	from the pattern.  If we are not in EXACT mode, fold out the case.
 */
int
eq(bc, pc)
	register Char	bc;
	register Char	pc;
{
	if (!(curwp->w_bufp->b_mode & MDEXACT)) {
		bc = adjcase(bc);
		pc = adjcase(pc);
	}
	return bc == pc;
}

#if KANJI
/* table for converting characters for non-EXACT comparisons */
/* This table is sorted by s field for binary search method */
struct ctab {
	Char s, d;
} ctab[] = {
	{ '`', '\'' }, /* back quote */
	{ 0xa0, ' ' }, /* KANA Space*/
	{ 0xa1, '.' }, /* kuten */
	{ 0xa2, '[' }, /* open kagi kakko */
	{ 0xa3, ']' }, /* close kagi kakko */
	{ 0xa4, ',' }, /* touten */
	{ 0xa5, '.' }, /* nakaguro */
	{ 0xb0, '-' }, /* Chou-on kigou */
	{ CCHR(0x2121), ' ' }, /* KANJI Space */
	/* SYMBOL */
	{ CCHR(0x2122), ',' }, { CCHR(0x2123), '.' }, { CCHR(0x2124), ',' },
	{ CCHR(0x2125), '.' },
	{ 0x2126, '.' }, { 0x2127, ':' }, { 0x2128, ';' }, { 0x2129, '?' },
	{ 0x212a, '!' }, { 0x212b, 0xde }, { 0x212c, 0xdf }, { 0x212d, '\'' },
	{ 0x212e, '\'' }, { 0x212f, '"' }, { 0x2130, '^' }, { 0x2131, '~' },
	{ 0x2132, '_' }, { 0x213c, '-' }, { 0x213d, '-' }, { 0x213e, '-' },
	{ 0x213f, '/' }, { 0x2140, '\\' }, { 0x2141, '-' }, { 0x2142, '|' },
	{ 0x2143, '|' }, { 0x2144, '.' }, { 0x2145, '.' }, { 0x2146, '\'' },
	{ 0x2147, '\'' }, { 0x2148, '"' }, { 0x2149, '"' }, { 0x214a, '(' },
	{ 0x214b, ')' }, { 0x214c, '[' }, { 0x214d, ']' }, { 0x214e, '[' },
	{ 0x214f, ']' }, { 0x2150, '{' }, { 0x2151, '}' }, { 0x2152, '<' },
	{ 0x2153, '>' }, { 0x2154, '<' }, { 0x2155, '>' }, { 0x2156, '[' },
	{ 0x2157, ']' }, { 0x2158, '[' }, { 0x2159, ']' }, { 0x215a, '[' },
	{ 0x215b, ']' }, { 0x215c, '+' }, { 0x215d, '-' }, { 0x215f, '*' },
	{ 0x2161, '=' }, { 0x2163, '<' }, { 0x2164, '>' }, { 0x2165, '<' },
	{ 0x2166, '>' }, { 0x216b, '\'' }, { 0x216c, '\'' }, { 0x216d, '\'' },
	{ 0x216f, '\\' }, { 0x2170, '$' }, { 0x2171, '$' }, { 0x2172, '$' },
	{ 0x2173, '%' }, { 0x2174, '#' }, { 0x2175, '&' }, { 0x2176, '*' },
	{ 0x2177, '@' },
	/* DIGITS */
	{ 0x2330, '0' }, { 0x2331, '1' }, { 0x2332, '2' }, { 0x2333, '3' },
	{ 0x2334, '4' }, { 0x2335, '5' }, { 0x2336, '6' }, { 0x2337, '7' },
	{ 0x2338, '8' }, { 0x2339, '9' },
	/* ALPHA */
	{ 0x2341, 'A' }, { 0x2342, 'B' }, { 0x2343, 'C' }, { 0x2344, 'D' },
	{ 0x2345, 'E' }, { 0x2346, 'F' }, { 0x2347, 'G' }, { 0x2348, 'H' },
	{ 0x2349, 'I' }, { 0x234a, 'J' }, { 0x234b, 'K' }, { 0x234c, 'L' },
	{ 0x234d, 'M' }, { 0x234e, 'N' }, { 0x234f, 'O' }, { 0x2350, 'P' },
	{ 0x2351, 'Q' }, { 0x2352, 'R' }, { 0x2353, 'S' }, { 0x2354, 'T' },
	{ 0x2355, 'U' }, { 0x2356, 'V' }, { 0x2357, 'W' }, { 0x2358, 'X' },
	{ 0x2359, 'Y' }, { 0x235a, 'Z' },
	{ 0x2361, 'A' }, { 0x2362, 'B' }, { 0x2363, 'C' }, { 0x2364, 'D' },
	{ 0x2365, 'E' }, { 0x2366, 'F' }, { 0x2367, 'G' }, { 0x2368, 'H' },
	{ 0x2369, 'I' }, { 0x236a, 'J' }, { 0x236b, 'K' }, { 0x236c, 'L' },
	{ 0x236d, 'M' }, { 0x236e, 'N' }, { 0x236f, 'O' }, { 0x2370, 'P' },
	{ 0x2371, 'Q' }, { 0x2372, 'R' }, { 0x2373, 'S' }, { 0x2374, 'T' },
	{ 0x2375, 'U' }, { 0x2376, 'V' }, { 0x2377, 'W' }, { 0x2378, 'X' },
	{ 0x2379, 'Y' }, { 0x237a, 'Z' },
	/* HIRAGANA */
	{ 0x2421, 0xa7 }, { 0x2422, 0xb1 }, { 0x2423, 0xa8 }, { 0x2424, 0xb2 },
	{ 0x2425, 0xa9 }, { 0x2426, 0xb3 }, { 0x2427, 0xaa }, { 0x2428, 0xb4 },
	{ 0x2429, 0xab }, { 0x242a, 0xb5 }, { 0x242b, 0xb6 }, { 0x242c, 0xb6 },
	{ 0x242d, 0xb7 }, { 0x242e, 0xb7 }, { 0x242f, 0xb8 }, { 0x2430, 0xb8 },
	{ 0x2431, 0xb9 }, { 0x2432, 0xb9 }, { 0x2433, 0xba }, { 0x2434, 0xba },
	{ 0x2435, 0xbb }, { 0x2436, 0xbb }, { 0x2437, 0xbc }, { 0x2438, 0xbc },
	{ 0x2439, 0xbd }, { 0x243a, 0xbd }, { 0x243b, 0xbe }, { 0x243c, 0xbe },
	{ 0x243d, 0xbf }, { 0x243e, 0xbf }, { 0x243f, 0xc0 }, { 0x2440, 0xc0 },
	{ 0x2441, 0xc1 }, { 0x2442, 0xc1 }, { 0x2443, 0xaf }, { 0x2444, 0xc2 },
	{ 0x2445, 0xc2 }, { 0x2446, 0xc3 }, { 0x2447, 0xc3 }, { 0x2448, 0xc4 },
	{ 0x2449, 0xc4 }, { 0x244a, 0xc5 }, { 0x244b, 0xc6 }, { 0x244c, 0xc7 },
	{ 0x244d, 0xc8 }, { 0x244e, 0xc9 }, { 0x244f, 0xca }, { 0x2450, 0xca },
	{ 0x2451, 0xca }, { 0x2452, 0xcb }, { 0x2453, 0xcb }, { 0x2454, 0xcb },
	{ 0x2455, 0xcc }, { 0x2456, 0xcc }, { 0x2457, 0xcc }, { 0x2458, 0xcd },
	{ 0x2459, 0xcd }, { 0x245a, 0xcd }, { 0x245b, 0xce }, { 0x245c, 0xce },
	{ 0x245d, 0xce }, { 0x245e, 0xcf }, { 0x245f, 0xd0 }, { 0x2460, 0xd1 },
	{ 0x2461, 0xd2 }, { 0x2462, 0xd3 }, { 0x2463, 0xac }, { 0x2464, 0xd4 },
	{ 0x2465, 0xad }, { 0x2466, 0xd5 }, { 0x2467, 0xae }, { 0x2468, 0xd6 },
	{ 0x2469, 0xd7 }, { 0x246a, 0xd8 }, { 0x246b, 0xd9 }, { 0x246c, 0xda },
	{ 0x246d, 0xdb }, { 0x246e, 0xdc }, { 0x246f, 0xdc }, { 0x2470, 0xb2 },
	{ 0x2471, 0xb4 }, { 0x2472, 0xa6 }, { 0x2473, 0xdd },
	/* KATAKANA */
	{ 0x2521, 0xa7 }, { 0x2522, 0xb1 }, { 0x2523, 0xa8 }, { 0x2524, 0xb2 },
	{ 0x2525, 0xa9 }, { 0x2526, 0xb3 }, { 0x2527, 0xaa }, { 0x2528, 0xb4 },
	{ 0x2529, 0xab }, { 0x252a, 0xb5 }, { 0x252b, 0xb6 }, { 0x252c, 0xb6 },
	{ 0x252d, 0xb7 }, { 0x252e, 0xb7 }, { 0x252f, 0xb8 }, { 0x2530, 0xb8 },
	{ 0x2531, 0xb9 }, { 0x2532, 0xb9 }, { 0x2533, 0xba }, { 0x2534, 0xba },
	{ 0x2535, 0xbb }, { 0x2536, 0xbb }, { 0x2537, 0xbc }, { 0x2538, 0xbc },
	{ 0x2539, 0xbd }, { 0x253a, 0xbd }, { 0x253b, 0xbe }, { 0x253c, 0xbe },
	{ 0x253d, 0xbf }, { 0x253e, 0xbf }, { 0x253f, 0xc0 }, { 0x2540, 0xc0 },
	{ 0x2541, 0xc1 }, { 0x2542, 0xc1 }, { 0x2543, 0xaf }, { 0x2544, 0xc2 },
	{ 0x2545, 0xc2 }, { 0x2546, 0xc3 }, { 0x2547, 0xc3 }, { 0x2548, 0xc4 },
	{ 0x2549, 0xc4 }, { 0x254a, 0xc5 }, { 0x254b, 0xc6 }, { 0x254c, 0xc7 },
	{ 0x254d, 0xc8 }, { 0x254e, 0xc9 }, { 0x254f, 0xca }, { 0x2550, 0xca },
	{ 0x2551, 0xca }, { 0x2552, 0xcb }, { 0x2553, 0xcb }, { 0x2554, 0xcb },
	{ 0x2555, 0xcc }, { 0x2556, 0xcc }, { 0x2557, 0xcc }, { 0x2558, 0xcd },
	{ 0x2559, 0xcd }, { 0x255a, 0xcd }, { 0x255b, 0xce }, { 0x255c, 0xce },
	{ 0x255d, 0xce }, { 0x255e, 0xcf }, { 0x255f, 0xd0 }, { 0x2560, 0xd1 },
	{ 0x2561, 0xd2 }, { 0x2562, 0xd3 }, { 0x2563, 0xac }, { 0x2564, 0xd4 },
	{ 0x2565, 0xad }, { 0x2566, 0xd5 }, { 0x2567, 0xae }, { 0x2568, 0xd6 },
	{ 0x2569, 0xd7 }, { 0x256a, 0xd8 }, { 0x256b, 0xd9 }, { 0x256c, 0xda },
	{ 0x256d, 0xdb }, { 0x256e, 0xdc }, { 0x256f, 0xdc }, { 0x2570, 0xb2 },
	{ 0x2571, 0xb4 }, { 0x2572, 0xa6 }, { 0x2573, 0xdd }, { 0x2574, 0xb3 },
	{ 0x2575, 0xb6 }, { 0x2576, 0xb9 },
};

# define CTABSIZE (sizeof(ctab)/sizeof(struct ctab))

#endif /*KANJI*/

/*
 * Adjust case of charater.
 */
static Char
adjcase(c)
	register Char c;
{
	register int r, l, m;
	register Char x;

	if (isLower(c))
		return CHCASE(c);
#if KANJI
	for (l = 0, r = CTABSIZE-1; l <= r; ) {
		m = (l+r)/2;
		if (c == (x = ctab[m].s)) {
			/* found */
			return ctab[m].d;
		}
		if (c < x) {
			r = m-1;
			continue;
		}
		l = m+1;
	}
#endif /*KANJI*/
	return c;
}

/*
 * readpattern -- Read a pattern.  Stash it in apat.	If it is the
 *	search string, create the reverse pattern and the magic
 *	pattern, assuming we are in MAGIC mode (and defined that way).
 *	Apat is not updated if the user types in an empty line.	 If
 *	the user typed an empty line, and there is no old pattern, it is
 *	an error.  Display the old pattern, in the style of Jeff Lomicka.
 *	There is some do-it-yourself control expansion.	 Change to using
 *	<META> to delimit the end-of-pattern to allow <NL>s in the search
 *	string. 
 */
static int
readpattern(prompt, apat, srch)
	Char	*prompt;
	Char	apat[];
	int	srch;
{
	int status;
	Char tpat[NPAT+20];

	(void)Cstrcpy(tpat, prompt);	/* copy prompt to output string */
	/* build new prompt string */
	(void)Cstrcat(tpat, CSTR(&Cxstr[3197]));
	(void)expandp(apat, &tpat[Cstrlen(tpat)], NPAT/2); /* add old pattern */
	(void)Cstrcat(tpat, CSTR(&Cxstr[3200]));

	/* Read a pattern.  Either we get one,
	 * or we just get the META charater, and use the previous pattern.
	 * Then, if it's the search string, make a reversed pattern.
	 * *Then*, make the meta-pattern, if we are defined that way.
	 */
	if ((status = mlreplyt(tpat, tpat, NPAT, metac)) == TRUE) {
		(void)Cstrcpy(apat, tpat);
		if (srch) { /* If we are doing the search string.*/
			matchlen = Cstrlen(apat);
			/* Reverse string copy.
			 */
			rvstrcpy(tap, apat);
#if	MAGIC
			/* Only make the meta-pattern if in magic mode,
			 * since the pattern in question might have an
			 * invalid meta combination.
			 */
			if (!(curwp->w_bufp->b_mode & MDMAGIC))
				mcclear();
			else
				status = mcstr();
#endif
		}
	}
	else if (status == FALSE && apat != 0)	/* Old one */
		status = TRUE;

	return(status);
}

/*
 * savematch -- We found the pattern?  Let's save it away.
 */

savematch()
{
	register Char *ptr;	/* ptr into malloced last match string */
	register int j;		/* index */
	LINE *curline;		/* line of last match */
	int curoff;		/* offset "      "    */

	/* free any existing match string */
	if (patmatch != NULL)
		free((char *)patmatch);

	/* attempt to allocate a new one */
	ptr = patmatch = (Char *)malloc(sizeof(Char)* (matchlen + 1));
	if (ptr == NULL)
		return;

	/* save the match! */
	curoff = matchoff;
	curline = matchline;

	for (j = 0; j < matchlen; j++)
		*ptr++ = nextch(&curline, &curoff, FORWARD);

	/* null terminate the match string */
	*ptr = '\0';
}

/*
 * rvstrcpy -- Reverse string copy.
 */
rvstrcpy(rvstr, str)
	register Char *rvstr, *str;
{
	register int i;

	str += (i = Cstrlen(str));

	while (i-- > 0)
		*rvstr++ = *--str;

	*rvstr = '\0';
}

/*
 * sreplace -- Search and replace.
 */
sreplace(f, n)
{
	return(replaces(FALSE, f, n));
}

/*
 * qreplace -- search and replace with query.
 */
qreplace(f, n)
{
	return(replaces(TRUE, f, n));
}

/*
 * replaces -- Search for a string and replace it with another
 *	string.	 Query might be enabled (according to kind).
 */
static int
replaces(kind, f, n)
	int kind;	/* Query enabled flag */
{
	register int status;	/* success flag on pattern inputs */
	register int rlength;	/* length of search and replace strings */
	register int numsub;	/* number of substitutions */
	register int nummatch;	/* number of found matches */
	int nlflag;		/* last char of search string a <NL>? */
	int nlrepl;		/* was a replace done on the last line? */
	Char c;			/* input char for query */
	Char tpat[NPAT];	/* temporary to hold search pattern */
	LINE *origline;		/* original "." position */
	int origoff;		/* and offset (for . query option) */
	LINE *lastline;		/* position of last replace and */
	int lastoff;		/* offset (for 'u' query option) */

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* Check for negative repetitions.
	 */
	if (f && n < 0)
		return(FALSE);

	/* Ask the user for the text of a pattern.
	 */
	if ((status = readpattern(
			(kind == FALSE ? (
#if KMSGS
				kterminal? CSTR(&Cxstr[7028]):
#endif
				CSTR(&Cxstr[7040])): (
#if KMSGS
				kterminal? CSTR(&Cxstr[7048]):
#endif
				CSTR(&Cxstr[7062]))),
			pat, TRUE))
							  != TRUE)
		return(status);

	/* Ask for the replacement string.
	 */
	if ((status = readpattern(
#if KMSGS
			kterminal? CSTR(&Cxstr[7076]):
#endif
			CSTR(&Cxstr[7086]),
				  rpat, FALSE)) == ABORT)
		return(status);

	/* Find the length of the replacement string.
	 */
	rlength = Cstrlen(rpat);

	/* Set up flags so we can make sure not to do a recursive
	 * replace on the last line.
	 */
	nlflag = (pat[matchlen - 1] == '\n');
	nlrepl = FALSE;

	if (kind) {
		/* Build query replace question string.
		 */
		(void)Cstrcpy(tpat,
#if KMSGS
			kterminal? CSTR(&Cxstr[2760]):
#endif
			CSTR(&Cxstr[7091]));
		(void)expandp(pat, &tpat[Cstrlen(tpat)], NPAT/3);
		(void)Cstrcat(tpat,
#if KMSGS
			kterminal? CSTR(&Cxstr[7101]):
#endif
			CSTR(&Cxstr[7107]));
		(void)expandp(rpat, &tpat[Cstrlen(tpat)], NPAT/3);
		(void)Cstrcat(tpat,
#if KMSGS
			kterminal? CSTR(&Cxstr[7116]):
#endif
			CSTR(&Cxstr[7129]));

		/* Initialize last replaced pointers.
		 */
		lastline = NULL;
		lastoff = 0;
	}

	/* Save original . position, init the number of matches and
	 * substitutions, and scan through the file.
	 */
	origline = curwp->w_dotp;
	origoff = curwp->w_doto;
	numsub = 0;
	nummatch = 0;

	while ( (f == FALSE || n > nummatch) &&
		(nlflag == FALSE || nlrepl == FALSE) ) {
		/* Search for the pattern.
		 * If we search with a regular expression,
		 * matchlen is reset to the true length of
		 * the matched string.
		 */
#if	MAGIC
		if (magical && (curwp->w_bufp->b_mode & MDMAGIC)) {
			if (!mcscanner(mcpat, FORWARD, PTBEG))
				break;
		} else
#endif
			if (!scanner(pat, FORWARD, PTBEG))
				break;		/* all done */

		++nummatch;	/* Increment # of matches */

		/* Check if we are on the last line.
		 */
		nlrepl = (lforw(curwp->w_dotp) == curwp->w_bufp->b_linep);
		
		/* Check for query.
		 */
		if (kind) {
			/* Get the query.
			 */
		    pprompt:	
			mlwrite(tpat, &pat[0], &rpat[0]);
		    qprompt:
			update(TRUE);  /* show the proposed place to change */
			c = tgetc();			/* and input */
			mlwrite(CSTR(&Cxstr[6]));		/* and clear */

			/* And respond appropriately.
			 */
			switch (c) {
			case 'y':	/* yes, substitute */
			case ' ':
			  savematch();
			  break;

			case 'n':	/* no, onword */
				(void)forwchar(FALSE, 1);
				continue;

			case '!':	/* yes/stop asking */
				kind = FALSE;
				break;

			case 'u':	/* undo last and re-prompt */
			  /* Restore old position.
			   */
			  if (lastline == NULL) {
			    /* There is nothing to undo.
			     */
			    TTbeep();
			    goto pprompt;
			  }
			  curwp->w_dotp = lastline;
			  curwp->w_doto = lastoff;
			  lastline = NULL;
			  lastoff = 0;

			  /* Delete the new string.
			   */
			  (void)backchar(FALSE, rlength);
			  status = delins(rlength, patmatch);
			  if (status != TRUE)
			    return (status);

			  /* Record one less substitution,
			   * backup, and reprompt.
			   */
			  --numsub;
			  (void)backchar(FALSE, mlenold);
			  goto pprompt;

			case '.':	/* abort! and return */
				/* restore old position */
				curwp->w_dotp = origline;
				curwp->w_doto = origoff;
				curwp->w_flag |= WFMOVE;

			case BELL:	/* abort! and stay */
				mlwrite(
#if KMSGS
					kterminal? CSTR(&Cxstr[7133]):
#endif
					CSTR(&Cxstr[7142]));
				return(FALSE);

			default:	/* bitch and beep */
				TTbeep();

			case '?':	/* help me */
				mlwrite(CSTR(&Cxstr[7151]));
				goto qprompt;

			}
		}

		/* delete the sucker and insert its
		 * replacement.
		 */
		status = delins(matchlen, rpat);
		if (!status)
		  return (status);

		/* Save where we are if we might undo this....
		 */
		if (kind) {
			lastline = curwp->w_dotp;
			lastoff = curwp->w_doto;
		}

		numsub++;	/* increment # of substitutions */
	}

	/* And report the results.
	 */
	mlwrite(
#if KMSGS
		kterminal? CSTR(&Cxstr[7225]):
#endif
		CSTR(&Cxstr[7240]),
		numsub);
	return(TRUE);
}

/*
 * delins -- Delete a specified length from the current
 *	point, then insert the string.
 */
delins(dlength, instr)
     int dlength;
     Char *instr;
{
	int	status;
	Char	tmpc;

	/* Zap what we gotta,
	 * and insert its replacement.
	 */
	if (!(status = ldelete((long) dlength, FALSE))) {
	  mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
		  kterminal? CSTR(&Cxstr[7257]):
#endif
		  CSTR(&Cxstr[7266]));
	  return(FALSE);
	} else
	  while (tmpc = *instr) {
	    status = (tmpc == '\n'? lnewline(): linsert(1, tmpc));

	    /* Insertion error?
	     */
	    if (!status) {
	      mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
		      kterminal? CSTR(&Cxstr[7288]):
#endif
		      CSTR(&Cxstr[7305]));
	      break;
	    }
	    instr++;
	  }
	return (status);
}

/*
 * expandp -- Expand control key sequences for output.
 */
expandp(srcstr, deststr, maxlength)
	Char *srcstr; /* string to expand */
	Char *deststr;	/* destination of expanded string */
	int maxlength;	/* maximum chars in destination */
{
	Char c;		/* current char to translate */

	/* Scan through the string.
	 */
	while ((c = *srcstr++) != 0) {
		if (c == '\n') { /* it's a newline */
			*deststr++ = '<';
			*deststr++ = 'N';
			*deststr++ = 'L';
			*deststr++ = '>';
			maxlength -= 4;
		} else if (c < 0x20 || c == 0x7f) { /* control character */
			*deststr++ = '^';
			*deststr++ = c ^ 0x40;
			maxlength -= 2;
		} else if (c == '%') {
			*deststr++ = '%';
			*deststr++ = '%';
			maxlength -= 2;
		} else { /* any other character */
			*deststr++ = c;
			maxlength--;
		}

		/* check for maxlength */
		if (maxlength < 4) {
			*deststr++ = '$';
			*deststr = '\0';
			return(FALSE);
		}
	}
	*deststr = '\0';
	return(TRUE);
}

/*
 * boundry -- Return information depending on whether we may search no
 *	further.  Beginning of file and end of file are the obvious
 *	cases, but we may want to add further optional boundry restrictions
 *	in future, a' la VMS EDT.  At the moment, just return TRUE or
 *	FALSE depending on if a boundry is hit (ouch).
 */
int
boundry(curline, curoff, dir)
	LINE	*curline;
	int	curoff, dir;
{
	register int	border;

	if (dir == FORWARD) {
		border = (curoff == llength(curline)) &&
			 (lforw(curline) == curbp->b_linep);
	} else {
		border = (curoff == 0) &&
			 (lback(curline) == curbp->b_linep);
	}
	return (border);
}

/*
 * nextch -- retrieve the next/previous character in the buffer,
 *	and advance/retreat the point.
 *	The order in which this is done is significant, and depends
 *	upon the direction of the search.  Forward searches look at
 *	the current character and move, reverse searches move and
 *	look at the character.
 */
static int
nextch(pcurline, pcuroff, dir)
	LINE	**pcurline;
	int	*pcuroff;
	int	dir;
{
	register LINE	*curline;
	register int	curoff;
	register Char	c;

	curline = *pcurline;
	curoff = *pcuroff;

	if (dir == FORWARD) {
		if (curoff == llength(curline)) { /* if at EOL */
			curline = lforw(curline);	/* skip to next line */
			curoff = 0;
			c = '\n';			/* and return a <NL> */
		} else
			c = lgetc(curline, curoff++);	/* get the char */
	} else { /* Reverse.*/
		if (curoff == 0) {
			curline = lback(curline);
			curoff = llength(curline);
			c = '\n';
		} else
			c = lgetc(curline, --curoff);

	}
	*pcurline = curline;
	*pcuroff = curoff;

	return (c);
}

#if	MAGIC
/*
 * mcstr -- Set up the 'magic' array.	 The closure symbol is taken as
 *	a literal character when (1) it is the first character in the
 *	pattern, and (2) when preceded by a symbol that does not allow
 *	closure, such as a newline, beginning of line symbol, or another
 *	closure symbol.
 *
 *	Coding comment (jmg):  yes, i know i have gotos that are, strictly
 *	speaking, unnecessary.	But right now we are so cramped for
 *	code space that i will grab what i can in order to remain
 *	within the 64K limit.  C compilers actually do very little
 *	in the way of optimizing - they expect you to do that.
 */
static int
mcstr()
{
	MC	*mcptr, *rtpcm;
	Char	*patptr;
	int	mj;
	int	pchr;
	int	status = TRUE;
	int	does_closure = FALSE;

	/* If we had metacharacters in the MC array previously,
	 * free up any bitmaps that may have been allocated.
	 */
	if (magical)
		mcclear();

	magical = FALSE;
	mj = 0;
	mcptr = mcpat;
	patptr = pat;

	while ((pchr = *patptr) && status) {
		switch (pchr) {
		case MC_CCL:
			status = cclmake(&patptr, mcptr);
			magical = TRUE;
			does_closure = TRUE;
			break;
		case MC_BOL:
			if (mj != 0)
				goto litcase;

			mcptr->mc_type = BOL;
			magical = TRUE;
			does_closure = FALSE;
			break;
		case MC_EOL:
			if (*(patptr + 1) != '\0')
				goto litcase;

			mcptr->mc_type = EOL;
			magical = TRUE;
			does_closure = FALSE;
			break;
		case MC_ANY:
			mcptr->mc_type = ANY;
			magical = TRUE;
			does_closure = TRUE;
			break;
		case MC_CLOSURE:
			/* Does the closure symbol mean closure here?
			 * If so, back up to the previous element
			 * and indicate it is enclosed.
			 */
			if (!does_closure)
				goto litcase;
			mj--;
			mcptr--;
			mcptr->mc_type |= CLOSURE;
			magical = TRUE;
			does_closure = FALSE;
			break;
		case MC_ESC:
			if (*(patptr + 1) != '\0') {
				pchr = *++patptr;
				magical = TRUE;
			}
			/* Falls into... */
		default:
		      litcase:
			mcptr->mc_type = LITCHAR;
			mcptr->u.lchar = pchr;
			does_closure = (pchr != '\n');
			break;
		}		/* End of switch.*/
		mcptr++;
		patptr++;
		mj++;
	}		/* End of while.*/

	/* Close off the meta-string.
	 */
	mcptr->mc_type = MCNIL;

	/* Set up the reverse array, if the status is good.  Please note the
	 * structure assignment - your compiler may not like that.
	 * If the status is not good, nil out the meta-pattern.
	 * The only way the status would be bad is from the cclmake()
	 * routine, and the bitmap for that member is guarenteed to be
	 * freed.  So we stomp a MCNIL value there, and call mcclear()
	 * to free any other bitmaps.
	 */
	if (status) {
		rtpcm = tapcm;
		while (--mj >= 0) {
			*rtpcm++ = *--mcptr;
		}
		rtpcm->mc_type = MCNIL;
	} else {
		(--mcptr)->mc_type = MCNIL;
		mcclear();
	}

	return(status);
}

/*
 * mcclear -- Free up any CCL bitmaps, and MCNIL the MC arrays.
 */
mcclear()
{
	register MC	*mcptr;

	mcptr = &mcpat[0];

	while (mcptr->mc_type != MCNIL) {
		if ((mcptr->mc_type & MASKCL) == CCL ||
		    (mcptr->mc_type & MASKCL) == NCCL)
			free(mcptr->u.cclmap);
		mcptr++;
	}
	mcpat[0].mc_type = tapcm[0].mc_type = MCNIL;
}

/*
 * mceq -- meta-character equality with a character. In Kernighan & Plauger's
 *	Software Tools, this is the function omatch(), but i felt there
 *	were too many functions with the 'match' name already.
 */
static int
mceq(bc, mt)
	Char	bc;
	MC	*mt;
{
	register int result;

	switch (mt->mc_type & MASKCL) {
	case LITCHAR:
		result = eq(bc, mt->u.lchar);
		break;

	case ANY:
		result = (bc != '\n');
		break;

	case CCL:
		if (!(result = biteq(bc, mt->u.cclmap))) {
			if (!(curwp->w_bufp->b_mode & MDEXACT) &&
			    (isAlpha(bc))) {
				result = biteq(CHCASE(bc), mt->u.cclmap);
			}
		}
		break;

	case NCCL:
		result = !biteq(bc, mt->u.cclmap);

		if (!(curwp->w_bufp->b_mode & MDEXACT) &&
		    (isAlpha(bc))) {
			result &= !biteq(CHCASE(bc), mt->u.cclmap);
		}
		break;

	default:
		mlwrite(CSTR(&Cxstr[7336]), mt->mc_type);
		result = FALSE;
		break;

	}	/* End of switch.*/

	return (result);
}

/*
 * cclmake -- create the bitmap for the character class.
 *	ppatptr is left pointing to the end-of-character-class character,
 *	so that a loop may automatically increment with safety.
 */
static int
cclmake(ppatptr, mcptr)
	Char	**ppatptr;
	MC	*mcptr;
{
	BITMAP		bmap;
	register Char	*patptr;
	register int	pchr, ochr;

	if ((bmap = clearbits()) == NULL) {
		mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
		kterminal? CSTR(&Cxstr[7354]):
#endif
		CSTR(&Cxstr[7363]));
		return FALSE;
	}

	mcptr->u.cclmap = bmap;
	patptr = *ppatptr;

	/*
	 * Test the initial character(s) in ccl for
	 * special cases - negate ccl, or an end ccl
	 * character as a first character.  Anything
	 * else gets set in the bitmap.
	 */
	if (*++patptr == MC_NCCL) {
		patptr++;
		mcptr->mc_type = NCCL;
	} else
		mcptr->mc_type = CCL;

	if ((ochr = *patptr) == MC_ECCL) {
		mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
			kterminal? CSTR(&Cxstr[7378]):
#endif
			CSTR(&Cxstr[7395]));
		return (FALSE);
	} else {
		if (ochr == MC_ESC)
			ochr = *++patptr;

		setbit(ochr, bmap);
		patptr++;
	}

	while (ochr != '\0' && (pchr = *patptr) != MC_ECCL) {
		switch (pchr) {
		/* Range character loses its meaning
		 * if it is the last character in
		 * the class.
		 */
		case MC_RCCL:
			if (*(patptr + 1) == MC_ECCL)
				setbit(pchr, bmap);
			else {
				pchr = *++patptr;
				while (++ochr <= pchr)
					setbit(ochr, bmap);
			}
			break;

		case MC_ESC:
			pchr = *++patptr;
			/* Falls into... */
		default:
			setbit(pchr, bmap);
			break;
		}
		patptr++;
		ochr = pchr;
	}

	*ppatptr = patptr;

	if (ochr == '\0') {
		mlwrite(CSTR(&Cxstr[1281]),
#if KMSGS
			kterminal? CSTR(&Cxstr[7429]):
#endif
			CSTR(&Cxstr[7445]));
		free(bmap);
		return FALSE;
	}
	return TRUE;
}

/*
 * biteq -- is the character in the bitmap?
 */
static int
biteq(bc, cclmap)
	int	bc;
	BITMAP	cclmap;
{
	if (bc >= HICHAR)
		return FALSE;

	return( (*(cclmap + (bc >> 3)) & BIT(bc & 7))? TRUE: FALSE );
}

/*
 * clearbits -- Allocate and zero out a CCL bitmap.
 */
static BITMAP
clearbits()
{
	BITMAP		cclstart, cclmap;
	register int	j;

	if ((cclmap = cclstart = (BITMAP) malloc(HIBYTE)) != NULL)
		for (j = 0; j < HIBYTE; j++)
			*cclmap++ = 0;

	return (cclstart);
}

/*
 * setbit -- Set a bit (ON only) in the bitmap.
 */
static
setbit(bc, cclmap)
	int	bc;
	BITMAP	cclmap;
{
	if (bc < HICHAR)
		*(cclmap + (bc >> 3)) |= BIT(bc & 7);
}
#endif /*MAGIC*/
