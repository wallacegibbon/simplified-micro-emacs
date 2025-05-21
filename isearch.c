#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

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
	matchlen = strlen(pat);
	return TRUE;
}

int risearch(int f, int n)
{
	return fisearch(f, -n);
}

int isearch(int f, int n)
{
	struct line *curline = curwp->w_dotp, *tmpline = NULL;
	int curoff = curwp->w_doto, tmpoff = 0;
	int init_direction = n;
	char pat_save[NPAT];
	int status, col, cpos, expc, c;

	cmd_reexecute = -1;	/* We're not re-executing (yet) */
	cmd_offset = 0;
	cmd_buff[0] = '\0';

	/* `pat` is global, so 0-initialized on startup */
	strncpy(pat_save, pat, NPAT - 1);

start_over:
	col = promptpattern("ISearch:", pat_save);

	/* Restore n for a new loop */
	n = init_direction;

	/* Restore the pat for a new loop */
	strcpy(pat, pat_save);

	status = TRUE;
	cpos = 0;

	c = ectoc(expc = get_char());

	/* When ^S or ^R again, load the pattern and do a search */

	if (pat[0] != '\0' && (c == IS_FORWARD || c == IS_REVERSE)) {
		for (cpos = 0; pat[cpos] != '\0'; ++cpos) {
			movecursor(term.t_nrow, col);
			col += put_c(pat[cpos], TTputc);
		}
		TTflush();
		n = (c == IS_REVERSE) ? -1 : 1;
		status = scanmore(pat, n);
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
		n = (c == IS_REVERSE) ? -1 : 1;
		status = scanmore(pat, n);
		c = ectoc(expc = get_char());
		goto char_loop;
	}

	if (c == '\b' || c == 0x7F) {
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

	if (c < 0x20 && c != '\t') {
		reeat_char = c;
		return TRUE;
	}

	/* Now we are likely to insert c to pattern */

pat_append:
	pat[cpos++] = c;
	if (cpos >= NPAT - 1) {
		mlwrite("? Search string too long");
		return TRUE;
	}
	pat[cpos] = '\0';
	movecursor(term.t_nrow, col);
	col += put_c(c, TTputc);
	TTflush();

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
	tmpline = curwp->w_dotp;
	tmpoff = curwp->w_doto;
	curwp->w_dotp = curline;
	curwp->w_doto = curoff;

	status = scanmore(pat, n);
	if (status == FALSE) {
		/* When search failed, stay on previous success position */
		curwp->w_dotp = tmpline;
		curwp->w_doto = tmpoff;
	}

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
	char tpat[NPAT + 64 /* prompt */ + 5 /* " (", "): " */ + 1];

	strncpy(tpat, prompt, 64);
	strcat(tpat, " (");
	strcat(tpat, pat);
	strcat(tpat, "): ");

	return mlwrite(tpat);
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
