#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

static int echo_char(int c, int col);

static int cmd_buff[CMDBUFLEN];		/* Save the command args here */
static int cmd_offset;			/* Current offset into command buff */
static int cmd_reexecute = -1;		/* > 0 if re-executing command */

int fisearch(int f, int n)
{
	struct line *curline = curwp->w_dotp;
	int curoff = curwp->w_doto;

	if (!(isearch(f, n))) {
		/*
		 * When search failed, restore the original position.
		 * (This is necessary when we use ^G to cancel a search)
		 */
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

int isearch(int f, int n)
{
	struct line *curline = curwp->w_dotp;	/* Save curpos to restore */
	int curoff = curwp->w_doto;
	int init_direction = n;
	char pat_save[NPAT];
	int status, col, cpos, expc, c;
#if ISEARCH_FLAVOR == 1
	int was_searching;
#endif

	cmd_reexecute = -1;	/* We're not re-executing (yet) */
	cmd_offset = 0;
	cmd_buff[0] = '\0';

	/* `pat` is global, so 0-initialized on startup */
	strncpy(pat_save, pat, NPAT - 1);

start_over:
	/* Display prompt and clear rest contents in message line */
	col = promptpattern("ISearch: ", pat_save);

	/* Restore n for a new loop */
	n = init_direction;

	/* Restore the pat for a new loop */
	strcpy(pat, pat_save);

#if ISEARCH_FLAVOR == 1
	was_searching = 0;
#endif
	status = TRUE;
	cpos = 0;

	c = ectoc(expc = get_char());

	/* When ^S or ^R again, load the pattern and do a search */

	if (pat[0] != '\0' && (c == IS_FORWARD || c == IS_REVERSE)) {
		for (cpos = 0; pat[cpos] != '\0'; ++cpos)
			col = echo_char(pat[cpos], col);
		n = (c == IS_REVERSE) ? -1 : 1;
		status = scanmore(pat, n);
#if ISEARCH_FLAVOR == 1
		if (status)
			was_searching = 1;
#endif
		c = ectoc(expc = get_char());
	}

char_loop:
	/* ^M finishs the searching and leave the pat as what it is */
	if (expc == ENTERC)
		return TRUE;

	/* ^G stops the searching and restore the search pattern */
	if (expc == ABORTC) {
		strcpy(pat, pat_save);
		return FALSE;
	}

	if (expc == QUOTEC) {
		c = ectoc(expc = get_char());
		goto pat_append;
	}

	if (c == IS_REVERSE || c == IS_FORWARD) {
#if ISEARCH_FLAVOR == 1
		if (pat[0] != '\0' && status)
			was_searching = 1;
#endif
		n = (c == IS_REVERSE) ? -1 : 1;
		status = scanmore(pat, n);
		c = ectoc(expc = get_char());
		goto char_loop;
	}

	if (c == '\b' || c == 0x7F) {
#if ISEARCH_FLAVOR == 1
		if (was_searching) {
			reeat_char = c;
			return TRUE;
		}
#endif
		if (cmd_offset <= 1) {
			/* We don't want to lose the saved pattern */
			strcpy(pat, pat_save);
			return TRUE;
		}
		--cmd_offset;			/* Ignore the '\b' or 0x7F */
		cmd_buff[--cmd_offset] = '\0';	/* Delete last char */
		cmd_reexecute = 0;
		curwp->w_dotp = curline;
		curwp->w_doto = curoff;
		curwp->w_flag |= WFMOVE;
		goto start_over;
	}

	if (c < ' ') {
		reeat_char = c;
		return TRUE;
	}

	/* Now we are likely to insert c to pattern */

pat_append:

#if ISEARCH_FLAVOR == 1
	if (was_searching) {
		reeat_char = c;
		return TRUE;
	}
#endif

	pat[cpos++] = c;
	if (cpos >= NPAT - 1) {
		mlwrite("? Search string too long");
		return TRUE;
	}
	pat[cpos] = '\0';
	col = echo_char(c, col);

	/* If we lost on last char, no more check is needed */
	if (!status) {
		TTputc(BELL);
		TTflush();
		c = ectoc(expc = get_char());
		goto char_loop;
	}

	/* Still matching so far, keep going */

	/*
	 * The scan during a changing pattern is tricky.  A simple solution
	 * is to restore the "." position before a scan.
	 */
	curwp->w_dotp = curline;
	curwp->w_doto = curoff;

	status = scanmore(pat, n);
	if (!status)
		curwp->w_flag |= WFMOVE;

	c = ectoc(expc = get_char());
	goto char_loop;
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
		TTputc('^'); TTputc('?'); ++col;
	} else if (c < ' ') {
		TTputc('^'); TTputc(c + 0x40); ++col;
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
		return ABORTC;
	}
	c = get1key();
	cmd_buff[cmd_offset++] = c;
	cmd_buff[cmd_offset] = '\0';
	return c;
}
