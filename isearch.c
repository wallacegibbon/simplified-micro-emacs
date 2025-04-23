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

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

static int echo_char(int c, int col);

/* No `static` for easier debugging */
int cmd_buff[CMDBUFLEN];	/* Save the command args here */
int cmd_offset;			/* Current offset into command buff */
int cmd_reexecute = -1;		/* > 0 if re-executing command */

int fisearch(int f, int n)
{
	struct line *curline = curwp->w_dotp;
	int curoff = curwp->w_doto;

	if (!(isearch(f, n))) {
		/* When search failed, restore the original position */
		curwp->w_dotp = curline;
		curwp->w_doto = curoff;
		curwp->w_flag |= WFMOVE;
		update(FALSE);
		mlwrite("(search failed)");
	} else {
		mlerase();
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
 * change to a backwards search, Enter will terminate the search and Control-G
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
	struct line *curline = curwp->w_dotp;
	int curoff = curwp->w_doto;
	int init_direction = n;
	char pat_save[NPAT];
	int status, col, cpos, expc, c;

	cmd_reexecute = -1;	/* We're not re-executing (yet?) */
	cmd_offset = 0;
	cmd_buff[0] = '\0';

	/* `pat` is global, so 0-initialized on startup */
	strncpy(pat_save, pat, NPAT - 1);

	/* Display the prompt before any command */
	promptpattern("ISearch: ", pat);

	/* Fill the search string when necessary (^S or ^R again on start) */

	c = ectoc(expc = get_char());
	cmd_offset = 0;
	if ((c == IS_FORWARD) || (c == IS_REVERSE)) {
		for (cpos = 0; pat[cpos] != '\0'; cpos++)
			cmd_buff[cmd_offset++] = pat[cpos];
	} else {
		cmd_buff[cmd_offset++] = c;
	}
	cmd_buff[cmd_offset] = '\0';

	/* Both rubout and backspace will trigger the re-searching */
start_over:
	/* display prompt and clear rest contents in message line */
	col = promptpattern("ISearch: ", pat_save);
	cmd_reexecute = 0;
	status = TRUE;
	cpos = 0;

char_loop:
	c = ectoc(expc = get_char());

	/* ^M will finish the searching */
	if (expc == enterc)
		return TRUE;

	/* ^G will stop the searching and restore the search pattern */
	if (expc == abortc) {
		strcpy(pat, pat_save);
		return FALSE;
	}

	if (c == IS_REVERSE || c == IS_FORWARD) {
		n = (c == IS_REVERSE) ? -1 : 1;
		status = scanmore(pat, n);
		goto char_loop;
	}

	if (c == IS_BACKSP || c == IS_RUBOUT) {
		if (cmd_offset <= 1) {
			/* We don't want to lose the saved pattern */
			strcpy(pat, pat_save);
			return TRUE;
		}
		--cmd_offset;
		cmd_buff[--cmd_offset] = '\0';	/* Delete last char */
		curwp->w_dotp = curline;
		curwp->w_doto = curoff;
		curwp->w_flag |= WFMOVE;
		n = init_direction;
		goto start_over;
	}

	if (c < ' ') {
		reeat_char = c;
		return TRUE;
	}

	pat[cpos++] = c;
	if (cpos >= NPAT - 1) {
		mlwrite("? Search string too long");
		return TRUE;
	}
	pat[cpos] = '\0';
	col = echo_char(c, col);

	if (!status) {	/* If we lost last time */
		TTputc(BELL);
		TTflush();
	} else if (!(status = checknext(c, pat, n))) {
		if (n == -1)
			forwchar(FALSE, cpos);
		status = scanmore(pat, n);
	}
	goto char_loop;
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
int checknext(char chr, char *patrn, int dir)
{
	struct line *curline = curwp->w_dotp;
	int curoff = curwp->w_doto;
	int buffchar;
	int status;

	/* This tiny optimization only work for forward search */
	if (dir < 0)
		return FALSE;

	if (curoff == llength(curline)) { /* If at end of line */
		curline = lforw(curline);

		/* Abort if at end of buffer */
		if (curline == curbp->b_linep)
			return FALSE;

		curoff = 0;
		buffchar = '\n';
	} else {
		buffchar = lgetc(curline, curoff++);
	}

	/* Is it what we're looking for? */
	if ((status = eq(buffchar, chr))) {
		curwp->w_dotp = curline;
		curwp->w_doto = curoff;
		curwp->w_flag |= WFMOVE;
	}
	return status;
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
int scanmore(char *patrn, int dir)
{
	int sts;

	if (dir < 0) {	/* reverse search? */
		rvstrcpy(tap, patrn);	/* Put reversed string in tap */
		sts = scanner(tap, REVERSE, PTBEG);
	} else {
		sts = scanner(patrn, FORWARD, PTEND);
	}

	if (!sts) {
		TTputc(BELL);	/* Beep if search fails */
		TTflush();
	}

	return sts;	/* else, don't even try */
}

int promptpattern(const char *prompt, const char *pat)
{
	char tpat[NPAT + 20];

	strcpy(tpat, prompt);
	strcat(tpat, " (");
	expandp(pat, &tpat[strlen(tpat)], NPAT / 2);
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

/*
 * Routine to get the next character from the input stream.  If we're reading
 * from the real terminal, force a screen update before we get the char.
 * Otherwise, we must be re-executing the command string, so just return the
 * next character.
 */
int get_char(void)
{
	int c;
	if (cmd_reexecute >= 0)	{
		if ((c = cmd_buff[cmd_reexecute++]) != 0)
			return c;
	}

	/* We're not re-executing (any more).  Try for a real char */

	cmd_reexecute = -1;
	update(FALSE);
	if (cmd_offset >= CMDBUFLEN - 1) {
		mlwrite("? command too long");
		return abortc;
	}
	c = get1key();
	cmd_buff[cmd_offset++] = c;
	cmd_buff[cmd_offset] = '\0';
	return c;
}
