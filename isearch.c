/* isearch.c
 *
 * The functions in this file implement commands that perform incremental
 * searches in the forward and backward directions.  This "ISearch" command
 * is intended to emulate the same command from the original EMACS
 * implementation (ITS).  Contains references to routines internal to
 * SEARCH.C.
 *
 * REVISION HISTORY:
 *
 *	D. R. Banks 9-May-86
 *	- added ITS EMACSlike ISearch
 *
 *	John M. Gamble 5-Oct-86
 *	- Made iterative search use search.c's scanner() routine.
 *	  This allowed the elimination of bakscan().
 *	- Put isearch constants into estruct.h
 *	- Eliminated the passing of 'status' to scanmore() and
 *	  checknext(), since there were no circumstances where
 *	  it ever equalled FALSE.
 *
 *	Modified by Petri Kutvonen
 */

#include <stdio.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

#define IS_REVERSE	0x12	/* Search backward */
#define	IS_FORWARD	0x13	/* Search forward */

static int echo_char(int c, int col);

int fisearch(int f, int n)
{
	struct line *curline;
	int curoff;

	/* remember the initial . on entry: */

	curline = curwp->w_dotp;
	curoff = curwp->w_doto;

	/* do the search */

	if (!(isearch(f, n))) {	/* Call ISearch forwards */
		/* If error in search: */
		curwp->w_dotp = curline;
		curwp->w_doto = curoff;
		curwp->w_flag |= WFMOVE;
		update(FALSE);
		mlwrite("(search failed)");
#if PKCODE
		matchlen = strlen(pat);
#endif
	} else {
		mlerase();	/* If happy, just erase the cmd line */
	}
#if PKCODE
	matchlen = strlen(pat);
#endif
	return TRUE;
}

int risearch(int f, int n)
{
	return fisearch(f, -n);
}

/*
 * Subroutine to do an incremental search.  In general, this works similarly
 * to the older micro-emacs search function, except that the search happens
 * as each character is typed, with the screen and cursor updated with each
 * new search character.
 *
 * While searching forward, each successive character will leave the cursor
 * at the end of the entire matched string.  Typing a Control-S or Control-X
 * will cause the next occurrence of the string to be searched for (where the
 * next occurrence does NOT overlap the current occurrence).  A Control-R will
 * change to a backwards search, META will terminate the search and Control-G
 * will abort the search.  Rubout will back up to the previous match of the
 * string, or if the starting point is reached first, it will delete the
 * last character from the search string.
 *
 * While searching backward, each successive character will leave the cursor
 * at the beginning of the matched string.  Typing a Control-R will search
 * backward for the next occurrence of the string.  Control-S or Control-X
 * will revert the search to the forward direction.  In general, the reverse
 * incremental search is just like the forward incremental search inverted.
 *
 * In all cases, if the search fails, the user will be feeped, and the search
 * will stall until the pattern string is edited back into something that
 * exists (or until the search is aborted).
 */
int isearch(int f, int n)
{
	char pat_save[NPAT];
	int col = 0;
	int cpos = 0;
	int status = TRUE;
	int c, expc;

	/* `pat` is global, so 0-initialized on startup */
	strncpy(pat_save, pat, NPAT - 1);
	col = promptpattern("ISearch: ");

	/*
	 * Get the first character in the pattern.
	 * If we get an initial C-S or C-R,
	 * re-use the old search string and find the first occurrence
	 */

	c = ectoc(expc = get1key());

	/* Reuse old search string? */
	if ((c == IS_FORWARD) || (c == IS_REVERSE)) {
		for (cpos = 0; pat[cpos] != 0; cpos++)
			col = echo_char(pat[cpos], col);

		n = (c == IS_REVERSE) ? -1 : 1;
		status = scanmore(pat, n);
		c = ectoc(expc = get1key());
	}

	/* Top of the per character loop */

	for (;;) {
		/* CR/NEWLINE finish the searching */
		if (expc == enterc)
			return TRUE;
		/* ^G stop the searching and restore previous pattern */
		if (expc == abortc) {
			strcpy(pat, pat_save);
			return FALSE;
		}

		switch (c) {
		case IS_REVERSE:
		case IS_FORWARD:
			n = (c == IS_REVERSE) ? -1 : 1;
			status = scanmore(pat, n);
			update(FALSE);
			c = ectoc(expc = get1key());
			continue;

		default:
			/* Only add visible chars to the pattern buffer */
			if (!isvisible(c)) {
				c = ectoc(expc = get1key());
				continue;
			}
		}

		/* I guess we got something to search for, so search for it */

		pat[cpos++] = c;
		pat[cpos] = 0;
		if (cpos >= NPAT - 1) {
			mlwrite("? Search string too long");
			return TRUE;
		}
		col = echo_char(c, col);
		if (!status) {	/* If we lost last time */
			TTputc(BELL);
			TTflush();
		} else if (!(status = checknext(c, pat, n))) {
			status = scanmore(pat, n);
		}
		update(FALSE);
		c = ectoc(expc = get1key());
	}
}

/*
 * Trivial routine to insure that the next character in the search string is
 * still true to whatever we're pointing to in the buffer.  This routine will
 * not attempt to move the "point" if the match fails, although it will
 * implicitly move the "point" if we're forward searching, and find a match,
 * since that's the way forward isearch works.
 *
 * If the compare fails, we return FALSE and assume the caller will call
 * scanmore or something.
 *
 * char chr;		Next char to look for
 * char *patrn;		The entire search string (incl chr)
 * int dir;		Search direction
 */
int checknext(char chr, char *patrn, int dir)	/* Check next character in search string */
{
	struct line *curline;
	int curoff; /* position within current line */
	int buffchar; /* character at current position */
	int status;


	/* setup the local scan pointer to current "." */

	curline = curwp->w_dotp;
	curoff = curwp->w_doto;

	if (dir > 0) { /* If searching forward */
		if (curoff == llength(curline)) { /* If at end of line */
			curline = lforw(curline);

			/* Abort if at end of buffer */
			if (curline == curbp->b_linep)
				return FALSE;

			curoff = 0; /* Start at the beginning of the line */
			buffchar = '\n'; /* And say the next char is NL */
		} else {
			buffchar = lgetc(curline, curoff++);
		}
		/* Is it what we're looking for? */
		if ((status = eq(buffchar, chr)) != 0) {
			curwp->w_dotp = curline;
			curwp->w_doto = curoff;
			curwp->w_flag |= WFMOVE;
		}
		return status;
	} else { /* Else, if reverse search: */
		return match_pat(patrn); /* See if we're in the right place */
	}
}

/*
 * This hack will search for the next occurrence of <pat> in the buffer, either
 * forward or backward.  It is called with the status of the prior search
 * attempt, so that it knows not to bother if it didn't work last time.  If
 * we can't find any more matches, "point" is left where it was before.  If
 * we do find a match, "point" will be at the end of the matched string for
 * forward searches and at the beginning of the matched string for reverse
 * searches.
 *
 * char *patrn;			string to scan for
 * int dir;			direction to search
 */
int scanmore(char *patrn, int dir)	/* search forward or back for a pattern */
{
	int sts;

	if (dir < 0) {		/* reverse search? */
		rvstrcpy(tap, patrn);	/* Put reversed string in tap */
		sts = scanner(tap, REVERSE, PTBEG);
	} else {
		sts = scanner(patrn, FORWARD, PTEND);	/* Nope. Go forward */
	}

	if (!sts) {
		TTputc(BELL);	/* Feep if search fails */
		TTflush();	/* see that the feep feeps */
	}

	return sts;		/* else, don't even try */
}

/*
 * The following is a worker subroutine used by the reverse search.  It
 * compares the pattern string with the characters at "." for equality. If
 * any characters mismatch, it will return FALSE.
 *
 * This isn't used for forward searches, because forward searches leave "."
 * at the end of the search string (instead of in front), so all that needs to
 * be done is match the last char input.
 */
int match_pat(char *patrn)
{
	struct line *curline;
	int curoff;
	int buffchar, i;

	/* setup the local scan pointer to current "." */

	curline = curwp->w_dotp;
	curoff = curwp->w_doto;

	/* top of per character compare loop: */

	for (i = 0; i < (int)strlen(patrn); i++) {
		if (curoff == llength(curline)) {	/* If at end of line */
			curline = lforw(curline);	/* Skip to the next line */
			curoff = 0;
			if (curline == curbp->b_linep)
				return FALSE;	/* Abort if at end of buffer */
			buffchar = '\n';
		} else {
			buffchar = lgetc(curline, curoff++);	/* Get the next char */
		}
		if (!eq(buffchar, patrn[i]))	/* Is it what we're looking for? */
			return FALSE;	/* Nope, just punt it then */
	}
	return TRUE; /* Everything matched? */
}

/*
 * Routine to prompt for I-Search string.
 */
int promptpattern(char *prompt)
{
	char tpat[NPAT + 20];

	strcpy(tpat, prompt);	/* copy prompt to output string */
	strcat(tpat, " (");	/* build new prompt string */
	expandp(pat, &tpat[strlen(tpat)], NPAT / 2);	/* add old pattern */
	strcat(tpat, ")<CR>: ");

	mlwrite(tpat);
	return strlen(tpat);
}

static int echo_char(int c, int col)
{
	movecursor(term.t_nrow, col);
	if (c == 0x7F) {
		TTputc('^'); TTputc('?'); col++;
	} else if (c < ' ') {
		TTputc('^'); TTputc(c + 0x40); col++;
	} else {
		TTputc(c);
	}
	TTflush();
	return ++col;
}
