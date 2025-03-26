/* search.c
 *
 * The functions in this file implement commands that search in the forward
 * and backward directions.  There are no special characters in the search
 * strings.  Probably should have a regular expression search, or something
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
 *
 *	July 1987: John M. Gamble
 *	Set the variables matchlen and matchoff in the 'unreplace'
 *	section of replaces().  The function savematch() would
 *	get confused if you replaced, unreplaced, then replaced
 *	again (serves you right for being so wishy-washy...)
 *
 *	August 1987: John M. Gamble
 *	Put in new function rmcstr() to create the replacement
 *	meta-character array.  Modified delins() so that it knows
 *	whether or not to make use of the array.  And, put in the
 *	appropriate new structures and variables.
 *
 *	Modified by Petri Kutvonen
 */

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"
#include <stdio.h>
#include <unistd.h>

static int readpattern(char *prompt, char *apat, int srch);
static int replaces(int kind, int f, int n);
static int nextch(struct line **pcurline, int *pcuroff, int dir);

/*
 * forwsearch -- Search forward.  Get a search string from the user, and
 *	search for the string.  If found, reset the "." to be just after
 *	the match string, and (perhaps) repaint the display.
 */
int forwsearch(int f, int n)
{
	int status = TRUE;

	/* If n is negative, search backwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return backsearch(f, -n);

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	if ((status = readpattern("Search", &pat[0], TRUE)) == TRUE) {
		do {
			status = scanner(&pat[0], FORWARD, PTEND);
		} while ((--n > 0) && status);

		/* Save away the match, or complain
		 * if not there.
		 */
		if (status == TRUE)
			savematch();
		else
			mlwrite("Not found");
	}
	return status;
}

/*
 * forwhunt -- Search forward for a previously acquired search string.
 *	If found, reset the "." to be just after the match string,
 *	and (perhaps) repaint the display.
 */
int forwhunt(int f, int n)
{
	int status = TRUE;

	if (n < 0)		/* search backwards */
		return backhunt(f, -n);

	/* Make sure a pattern exists */
	if (pat[0] == '\0') {
		mlwrite("No pattern set");
		return FALSE;
	}

	/* Search for the pattern for as long as n is positive
	 * (n == 0 will go through once, which is just fine).
	 */
	do {
		status = scanner(&pat[0], FORWARD, PTEND);
	} while ((--n > 0) && status);

	/* Save away the match, or complain
	 * if not there.
	 */
	if (status == TRUE)
		savematch();
	else
		mlwrite("Not found");

	return status;
}

/*
 * backsearch -- Reverse search.  Get a search string from the user, and
 *	search, starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
int backsearch(int f, int n)
{
	int status = TRUE;

	/* If n is negative, search forwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return forwsearch(f, -n);

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	if ((status = readpattern("Reverse search", &pat[0], TRUE)) == TRUE) {
		do {
			status = scanner(&tap[0], REVERSE, PTBEG);
		} while ((--n > 0) && status);

		/* Save away the match, or complain
		 * if not there.
		 */
		if (status == TRUE)
			savematch();
		else
			mlwrite("Not found");
	}
	return status;
}

/*
 * backhunt -- Reverse search for a previously acquired search string,
 *	starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
int backhunt(int f, int n)
{
	int status = TRUE;

	if (n < 0)
		return forwhunt(f, -n);

	/* Make sure a pattern exists */
	if (tap[0] == '\0') {
		mlwrite("No pattern set");
		return FALSE;
	}

	/* Go search for it for as long as n is positive
	 * (n == 0 will go through once, which is just fine).
	 */
	do {
		status = scanner(&tap[0], REVERSE, PTBEG);
	} while ((--n > 0) && status);

	/* Save away the match, or complain
	 * if not there.
	 */
	if (status == TRUE)
		savematch();
	else
		mlwrite("Not found");

	return status;
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
int filter_buffer(int f, int n)
{
	int s;		/* return status from CLI */
	struct buffer *bp;	/* pointer to buffer to zot */
	char line[NLINE];	/* command line send to shell */
	char tmpnam[NFILEN];	/* place to store real file name */
	static char bname1[] = "fltinp";
	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */

	/* get the filter name and its args */
	if ((s = mlreply("#", line, NLINE)) != TRUE)
		return s;

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);	/* save the original name */
	strcpy(bp->b_fname, bname1);	/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1) != TRUE) {
		mlwrite("(Cannot write filter file)");
		strcpy(bp->b_fname, tmpnam);
		return FALSE;
	}
#if MSDOS
	strcat(line, " <fltinp >fltout");
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
	shellprog(line);
	TTkopen();
	sgarbf = TRUE;
	s = TRUE;
#endif

#if V7 | USG | BSD
	TTputc('\n');		/* Already have '\r' */
	TTflush();
	TTclose();		/* stty to old modes */
	TTkclose();
	strcat(line, " <fltinp >fltout");
	system(line);
	TTopen();
	TTkopen();
	TTflush();
	sgarbf = TRUE;
	s = TRUE;
#endif

	/* on failure, escape gracefully */
	if (s != TRUE || (readin(filnam2, FALSE) == FALSE)) {
		mlwrite("(Execution failed)");
		strcpy(bp->b_fname, tmpnam);
		unlink(filnam1);
		unlink(filnam2);
		return s;
	}

	/* reset file name */
	strcpy(bp->b_fname, tmpnam);	/* restore name */
	bp->b_flag |= BFCHG;	/* flag it as changed */

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return TRUE;
}

/*
 * scanner -- Search for a pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 *
 * unsigned char *patrn;	string to scan for
 * int direct;			which way to go.
 * int beg_or_end;		put point at beginning or end of pattern.
 */
int scanner(const char *patrn, int direct, int beg_or_end)
{
	int c;		/* character at current position */
	const char *patptr;	/* pointer into pattern */
	struct line *curline;		/* current line during scan */
	int curoff;		/* position within current line */
	struct line *scanline;		/* current line during scanning */
	int scanoff;		/* position in scanned line */

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

		if (eq(c, patrn[0])) {	/* if we find it.. */
			/* Setup scanning pointers.
			 */
			scanline = curline;
			scanoff = curoff;
			patptr = &patrn[0];

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
			if (beg_or_end == PTEND) {	/* at end of string */
				curwp->w_dotp = scanline;
				curwp->w_doto = scanoff;
			} else {	/* at beginning of string */

				curwp->w_dotp = matchline;
				curwp->w_doto = matchoff;
			}

			curwp->w_flag |= WFMOVE;	/* Flag that we have moved. */
			return TRUE;

		}
fail:
		;/* continue to search */
	}

	return FALSE;		/* We could not find a match */
}

/*
 * eq -- Compare two characters.  The "bc" comes from the buffer, "pc"
 *	from the pattern.  If we are not in EXACT mode, fold out the case.
 */
int eq(unsigned char bc, unsigned char pc)
{
	if ((curwp->w_bufp->b_mode & MDEXACT) == 0) {
		if (islower(bc))
			bc ^= DIFCASE;

		if (islower(pc))
			pc ^= DIFCASE;
	}

	return bc == pc;
}

/*
 * readpattern -- Read a pattern.  Stash it in apat.
 *	Apat is not updated if the user types in an empty line.  If
 *	the user typed an empty line, and there is no old pattern, it is
 *	an error.  Display the old pattern, in the style of Jeff Lomicka.
 *	There is some do-it-yourself control expansion.  Change to using
 *	<META> to delimit the end-of-pattern to allow <NL>s in the search
 *	string. 
 */
static int readpattern(char *prompt, char *apat, int srch)
{
	int status;
	char tpat[NPAT + 20];

	strcpy(tpat, prompt);	/* copy prompt to output string */
	strcat(tpat, " (");	/* build new prompt string */
	expandp(&apat[0], &tpat[strlen(tpat)], NPAT / 2);	/* add old pattern */
	strcat(tpat, ")<CR>: ");

	/* Read a pattern.  Either we get one,
	 * or we just get the CR charater, and use the previous pattern.
	 * Then, if it's the search string, make a reversed pattern.
	 * *Then*, make the meta-pattern, if we are defined that way.
	 */
	if ((status = mlreplyt(tpat, tpat, NPAT, enterc)) == TRUE) {
		strcpy(apat, tpat);
		if (srch) {	/* If we are doing the search string. */
			/* Reverse string copy, and remember
			 * the length for substitution purposes.
			 */
			rvstrcpy(tap, apat);
			mlenold = matchlen = strlen(apat);
		}
	} else if (status == FALSE && apat[0] != 0)	/* Old one */
		status = TRUE;

	return status;
}

/*
 * savematch -- We found the pattern?  Let's save it away.
 */
void savematch(void)
{
	char *ptr;	/* pointer to last match string */
	int j;
	struct line *curline;		/* line of last match */
	int curoff;		/* offset "      " */

	/* Free any existing match string, then
	 * attempt to allocate a new one.
	 */
	if (patmatch != NULL)
		free(patmatch);

	ptr = patmatch = malloc(matchlen + 1);

	if (ptr != NULL) {
		curoff = matchoff;
		curline = matchline;

		for (j = 0; j < matchlen; j++)
			*ptr++ = nextch(&curline, &curoff, FORWARD);

		*ptr = '\0';
	}
}

/*
 * rvstrcpy -- Reverse string copy.
 */
void rvstrcpy(char *rvstr, char *str)
{
	int i;

	str += (i = strlen(str));

	while (i-- > 0)
		*rvstr++ = *--str;

	*rvstr = '\0';
}

/*
 * sreplace -- Search and replace.
 */
int sreplace(int f, int n)
{
	return replaces(FALSE, f, n);
}

/*
 * qreplace -- search and replace with query.
 */
int qreplace(int f, int n)
{
	return replaces(TRUE, f, n);
}

/*
 * replaces -- Search for a string and replace it with another
 *	string.  Query might be enabled (according to kind).
 *
 * int kind;		Query enabled flag
 */
static int replaces(int kind, int f, int n)
{
	int status;		/* success flag on pattern inputs */
	int rlength;		/* length of replacement string */
	int numsub;		/* number of substitutions */
	int nummatch;		/* number of found matches */
	int nlflag;		/* last char of search string a <NL>? */
	int nlrepl;		/* was a replace done on the last line? */
	char c;			/* input char for query */
	char tpat[NPAT];	/* temporary to hold search pattern */
	struct line *origline;	/* original "." position */
	int origoff;		/* and offset (for . query option) */
	struct line *lastline;	/* position of last replace and */
	int lastoff;		/* offset (for 'u' query option) */

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */

	if (f && n < 0)
		return FALSE;

	/* Ask the user for the text of a pattern. */
	if ((status = readpattern((kind == FALSE ? "Replace" : "Query replace"),
			&pat[0], TRUE)) != TRUE)
		return status;

	/* Ask for the replacement string. */
	if ((status = readpattern("with", &rpat[0], FALSE)) == ABORT)
		return status;

	/* Find the length of the replacement string. */
	rlength = strlen(&rpat[0]);

	/*
	 * Set up flags so we can make sure not to do a recursive
	 * replace on the last line.
	 */
	nlflag = (pat[matchlen - 1] == '\n');
	nlrepl = FALSE;

	if (kind) {
		/* Build query replace question string. */
		strcpy(tpat, "Replace '");
		expandp(&pat[0], &tpat[strlen(tpat)], NPAT / 3);
		strcat(tpat, "' with '");
		expandp(&rpat[0], &tpat[strlen(tpat)], NPAT / 3);
		strcat(tpat, "'? ");

		/* Initialize last replaced pointers. */
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

	while ((f == FALSE || n > nummatch) &&
	       (nlflag == FALSE || nlrepl == FALSE)) {
		/* Search for the pattern.
		 * If we search with a regular expression,
		 * matchlen is reset to the true length of the matched string.
		 */
		if (!scanner(&pat[0], FORWARD, PTBEG))
			break;	/* all done */

		++nummatch;	/* Increment # of matches */

		/* Check if we are on the last line. */
		nlrepl = (lforw(curwp->w_dotp) == curwp->w_bufp->b_linep);

		/* Check for query. */
		if (kind) {
			/* Get the query. */
pprompt:
			mlwrite(&tpat[0], &pat[0], &rpat[0]);
qprompt:
			update(TRUE);	/* show the proposed place to change */
			c = tgetc();	/* and input */
			mlwrite("");	/* and clear it */

			/* And respond appropriately. */
			switch (c) {
#if PKCODE
			case 'Y':
#endif
			case 'y':	/* yes, substitute */
			case ' ':
				savematch();
				break;

#if PKCODE
			case 'N':
#endif
			case 'n':	/* no, onword */
				forwchar(FALSE, 1);
				continue;

			case '!':	/* yes/stop asking */
				kind = FALSE;
				break;

#if PKCODE
			case 'U':
#endif
			case 'u':	/* undo last and re-prompt */
				/* Restore old position. */
				if (lastline == NULL) {
					/* There is nothing to undo. */
					TTbeep();
					goto pprompt;
				}
				curwp->w_dotp = lastline;
				curwp->w_doto = lastoff;
				lastline = NULL;
				lastoff = 0;

				/* Delete the new string. */
				backchar(FALSE, rlength);
#if PKCODE
				matchline = curwp->w_dotp;
				matchoff = curwp->w_doto;
#endif
				status = delins(rlength, patmatch, FALSE);
				if (status != TRUE)
					return status;

				/* Record one less substitution,
				 * backup, save our place, and
				 * reprompt.
				 */
				--numsub;
				backchar(FALSE, mlenold);
				matchline = curwp->w_dotp;
				matchoff = curwp->w_doto;
				goto pprompt;

			case '.':	/* abort! and return */
				/* restore old position */
				curwp->w_dotp = origline;
				curwp->w_doto = origoff;
				curwp->w_flag |= WFMOVE;

			case BELL:	/* abort! and stay */
				mlwrite("Aborted!");
				return FALSE;

			default:	/* bitch and beep */
				TTbeep();

			case '?':	/* help me */
				mlwrite("(Y)es, (N)o, (!)Do rest, (U)ndo last, (^G)Abort, (.)Abort back, (?)Help: ");
				goto qprompt;

			}	/* end of switch */
		}

		/* end of "if kind" */
		/*
		 * Delete the sucker, and insert its
		 * replacement.
		 */
		status = delins(matchlen, &rpat[0], TRUE);
		if (status != TRUE)
			return status;

		/* Save our position, since we may
		 * undo this.
		 */
		if (kind) {
			lastline = curwp->w_dotp;
			lastoff = curwp->w_doto;
		}

		numsub++;	/* increment # of substitutions */
	}

	mlwrite("%d substitutions", numsub);
	return TRUE;
}

/*
 * delins -- Delete a specified length from the current point
 *	then either insert the string directly, or make use of
 *	replacement meta-array.
 */
int delins(int dlength, char *instr, int use_meta)
{
	int status;

	/* Zap what we gotta,
	 * and insert its replacement.
	 */
	if ((status = ldelete((long) dlength, FALSE)) != TRUE)
		mlwrite("%%ERROR while deleting");
	else
		status = linstr(instr);

	return status;
}

/*
 * expandp -- Expand control key sequences for output.
 *
 * char *srcstr;		string to expand
 * char *deststr;		destination of expanded string
 * int maxlength;		maximum chars in destination
 */
int expandp(char *srcstr, char *deststr, int maxlength)
{
	unsigned char c;	/* current char to translate */

	/* Scan through the string. */
	while ((c = *srcstr++) != 0) {
		if (c == '\n') {	/* it's a newline */
			*deststr++ = '<';
			*deststr++ = 'N';
			*deststr++ = 'L';
			*deststr++ = '>';
			maxlength -= 4;
		}
#if PKCODE
		else if ((c > 0 && c < 0x20) || c == 0x7f)	/* control character */
#else
		else if (c < 0x20 || c == 0x7f)	/* control character */
#endif
		{
			*deststr++ = '^';
			*deststr++ = c ^ 0x40;
			maxlength -= 2;
		} else if (c == '%') {
			*deststr++ = '%';
			*deststr++ = '%';
			maxlength -= 2;
		} else {	/* any other character */

			*deststr++ = c;
			maxlength--;
		}

		/* check for maxlength */
		if (maxlength < 4) {
			*deststr++ = '$';
			*deststr = '\0';
			return FALSE;
		}
	}
	*deststr = '\0';
	return TRUE;
}

/*
 * boundry
 *
 * Return information depending on whether we may search no further.
 * Beginning of file and end of file are the obvious cases.
 */
int boundry(struct line *curline, int curoff, int dir)
{
	int border;

	if (dir == FORWARD) {
		border = (curoff == llength(curline)) &&
			(lforw(curline) == curbp->b_linep);
	} else {
		border = (curoff == 0) &&
			(lback(curline) == curbp->b_linep);
	}
	return border;
}

/*
 * nextch -- retrieve the next/previous character in the buffer,
 *	and advance/retreat the point.
 *	The order in which this is done is significant, and depends
 *	upon the direction of the search.  Forward searches look at
 *	the current character and move, reverse searches move and
 *	look at the character.
 */
static int nextch(struct line **pcurline, int *pcuroff, int dir)
{
	struct line *curline;
	int curoff;
	int c;

	curline = *pcurline;
	curoff = *pcuroff;

	if (dir == FORWARD) {
		if (curoff == llength(curline)) {	/* if at EOL */
			curline = lforw(curline);	/* skip to next line */
			curoff = 0;
			c = '\n';	/* and return a <NL> */
		} else
			c = lgetc(curline, curoff++);	/* get the char */
	} else {		/* Reverse. */

		if (curoff == 0) {
			curline = lback(curline);
			curoff = llength(curline);
			c = '\n';
		} else
			c = lgetc(curline, --curoff);

	}
	*pcurline = curline;
	*pcuroff = curoff;

	return c;
}

