/* random.c
 *
 *      This file contains the command processing functions for a number of
 *      random commands. There is no functional grouping here, for sure.
 *
 *	Modified by Petri Kutvonen
 */

#include <stdio.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

/*
 * Display the current position of the cursor, in origin 1 X-Y coordinates,
 * the character that is under the cursor (in hex), and the fraction of the
 * text that is before the cursor. The displayed column is not the current
 * column, but the column that would be used on an infinite width display.
 * Normally this is bound to "C-X =".
 */
int showcpos(int f, int n)
{
	struct line *lp;	/* current line */
	long numchars;	/* # of chars in file */
	int numlines;	/* # of lines in file */
	long predchars;	/* # chars preceding point */
	int predlines;	/* # lines preceding point */
	int curchar;	/* character under cursor */
	int ratio;
	int col;
	int savepos;		/* temp save for current offset */
	int ecol;		/* column pos/end of current line */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_linep);

	/* start counting chars and lines */
	numchars = 0;
	numlines = 0;
	predchars = 0;
	predlines = 0;
	curchar = 0;
	while (lp != curbp->b_linep) {
		/* if we are on the current line, record it */
		if (lp == curwp->w_dotp) {
			predlines = numlines;
			predchars = numchars + curwp->w_doto;
			if ((curwp->w_doto) == llength(lp))
				curchar = '\n';
			else
				curchar = lgetc(lp, curwp->w_doto);
		}
		/* on to the next line */
		++numlines;
		numchars += llength(lp) + 1;
		lp = lforw(lp);
	}

	/* if at end of file, record it */
	if (curwp->w_dotp == curbp->b_linep) {
		predlines = numlines;
		predchars = numchars;
#if PKCODE
		curchar = 0;
#endif
	}

	/* Get real column and end-of-line column. */
	col = getccol(FALSE);
	savepos = curwp->w_doto;
	curwp->w_doto = llength(curwp->w_dotp);
	ecol = getccol(FALSE);
	curwp->w_doto = savepos;

	ratio = 0;		/* Ratio before dot. */
	if (numchars != 0)
		ratio = (100L * predchars) / numchars;

	/* summarize and report the info */
	mlwrite("Line %d/%d Col %d/%d Char %D/%D (%d%%) char = 0x%x",
		predlines + 1, numlines + 1, col, ecol,
		predchars, numchars, ratio, curchar);
	return TRUE;
}

int getcline(void)
{				/* get the current line number */
	struct line *lp;	/* current line */
	int numlines;	/* # of lines before point */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_linep);

	/* start counting lines */
	numlines = 0;
	while (lp != curbp->b_linep) {
		/* if we are on the current line, record it */
		if (lp == curwp->w_dotp)
			break;
		++numlines;
		lp = lforw(lp);
	}

	/* and return the resulting count */
	return numlines + 1;
}

/*
 * Return current column.  Stop at first non-blank given TRUE argument.
 */
int getccol(int bflg)
{
	int i, col;
	struct line *dlp = curwp->w_dotp;
	int byte_offset = curwp->w_doto;
	int len = llength(dlp);

	col = i = 0;
	while (i < byte_offset) {
		unicode_t c;

		i += utf8_to_unicode(dlp->l_text, i, len, &c);
		if (c != ' ' && c != '\t' && bflg)
			break;
		if (c == '\t')
			col |= tabmask;
		else if (c < 0x20 || c == 0x7F)
			++col;
		else if (c >= 0xc0 && c <= 0xa0)
			col += 2;
		++col;
	}
	return col;
}

/*
 * Set current column.
 *
 * int pos;		position to set cursor
 */
int setccol(int pos)
{
	int c;		/* character being scanned */
	int i;		/* index into current line */
	int col;	/* current cursor column */
	int llen;	/* length of line in bytes */

	col = 0;
	llen = llength(curwp->w_dotp);

	/* scan the line until we are at or past the target column */
	for (i = 0; i < llen; ++i) {
		/* upon reaching the target, drop out */
		if (col >= pos)
			break;

		/* advance one character */
		c = lgetc(curwp->w_dotp, i);
		if (c == '\t')
			col |= tabmask;
		else if (c < 0x20 || c == 0x7F)
			++col;
		++col;
	}

	/* set us at the new position */
	curwp->w_doto = i;

	/* and tell weather we made it */
	return col >= pos;
}

/*
 * Twiddle the two characters on either side of dot. If dot is at the end of
 * the line twiddle the two characters before it. Return with an error if dot
 * is at the beginning of line; it seems to be a bit pointless to make this
 * work. This fixes up a very common typo with a single stroke. Normally bound
 * to "C-T". This always works within a line, so "WFEDIT" is good enough.
 */
int twiddle(int f, int n)
{
	struct line *dotp;
	int doto;
	int cl;
	int cr;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */
	dotp = curwp->w_dotp;
	doto = curwp->w_doto;
	if (doto == llength(dotp) && --doto < 0)
		return FALSE;
	cr = lgetc(dotp, doto);
	if (--doto < 0)
		return FALSE;
	cl = lgetc(dotp, doto);
	lputc(dotp, doto + 0, cr);
	lputc(dotp, doto + 1, cl);
	lchange(WFEDIT);
	return TRUE;
}

/*
 * Quote the next character, and insert it into the buffer. All the characters
 * are taken literally, with the exception of the newline, which always has
 * its line splitting meaning. The character is always read, even if it is
 * inserted 0 times, for regularity. Bound to "C-Q"
 */
int quote(int f, int n)
{
	int s, c;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */
	c = tgetc();
	if (n < 0)
		return FALSE;
	if (n == 0)
		return TRUE;
	if (c == '\n') {
		do {
			s = lnewline();
		} while (s == TRUE && --n);
		return s;
	}
	return linsert(n, c);
}

/*
 * trim trailing whitespace from the point to eol
 */
int trim(int f, int n)
{
	struct line *lp;	/* current line pointer */
	int offset;		/* original line offset position */
	int length;		/* current length */
	int inc;		/* increment to next line [sgn(n)] */

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */

	if (f == FALSE)
		n = 1;

	/* loop thru trimming n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		lp = curwp->w_dotp;	/* find current line text */
		offset = curwp->w_doto;	/* save original offset */
		length = lp->l_used;	/* find current length */

		/* trim the current line */
		while (length > offset) {
			if (lgetc(lp, length - 1) != ' ' &&
					lgetc(lp, length - 1) != '\t')
				break;
			length--;
		}
		lp->l_used = length;

		/* advance/or back to the next line */
		forwline(TRUE, inc);
		n -= inc;
	}
	lchange(WFEDIT);
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	return TRUE;
}

/*
 * Open up some blank space. The basic plan is to insert a bunch of newlines,
 * and then back up over them. Everything is done by the subcommand
 * procerssors. They even handle the looping. Normally this is bound to "C-O".
 */
int openline(int f, int n)
{
	int i;
	int s;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */
	if (n < 0)
		return FALSE;
	if (n == 0)
		return TRUE;
	i = n;				/* Insert newlines. */
	do {
		s = lnewline();
	} while (s == TRUE && --i);
	if (s == TRUE)			/* Then back up overtop of them all*/
		s = backchar(f, n);
	return s;
}

int insert_newline(int f, int n)
{
	int s;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */
	if (n < 0)
		return FALSE;

	/* insert some lines */
	while (n--) {
		if ((s = lnewline()) != TRUE)
			return s;
#if SCROLLCODE
		curwp->w_flag |= WFINS;
#endif
	}
	return TRUE;
}

/*
 * Delete blank lines around dot. What this command does depends if dot is
 * sitting on a blank line. If dot is sitting on a blank line, this command
 * deletes all the blank lines above and below the current line. If it is
 * sitting on a non blank line then it deletes all of the blank lines after
 * the line. Normally this command is bound to "C-X C-O". Any argument is
 * ignored.
 */
int deblank(int f, int n)
{
	struct line *lp1;
	struct line *lp2;
	long nld;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */
	lp1 = curwp->w_dotp;
	while (llength(lp1) == 0 && (lp2 = lback(lp1)) != curbp->b_linep)
		lp1 = lp2;
	lp2 = lp1;
	nld = 0;
	while ((lp2 = lforw(lp2)) != curbp->b_linep && llength(lp2) == 0)
		++nld;
	if (nld == 0)
		return TRUE;
	curwp->w_dotp = lforw(lp1);
	curwp->w_doto = 0;
	return ldelete(nld, FALSE);
}

/*
 * Insert a newline, then enough tabs and spaces to duplicate the indentation
 * of the previous line. Assumes tabs are every eight characters. Quite simple.
 * Figure out the indentation of the current line. Insert a newline by calling
 * the standard routine. Insert the indentation by inserting the right number
 * of tabs and spaces. Return TRUE if all ok. Return FALSE if one of the
 * subcomands failed. Normally bound to "C-J".
 */
int indent(int f, int n)
{
	int nicol;
	int c;
	int i;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */
	if (n < 0)
		return FALSE;
	while (n--) {
		nicol = 0;
		for (i = 0; i < llength(curwp->w_dotp); ++i) {
			c = lgetc(curwp->w_dotp, i);
			if (c != ' ' && c != '\t')
				break;
			if (c == '\t')
				nicol |= tabmask;
			++nicol;
		}
		if (lnewline() == FALSE ||
				((i = nicol / 8) != 0 &&
						linsert(i, '\t') == FALSE) ||
				((i = nicol % 8) != 0 &&
						linsert(i, ' ') == FALSE))
			return FALSE;
	}
	return TRUE;
}

/*
 * Delete forward. This is real easy, because the basic delete routine does
 * all of the work. Watches for negative arguments, and does the right thing.
 * If any argument is present, it kills rather than deletes, to prevent loss
 * of text if typed with a big argument. Normally bound to "C-D".
 */
int forwdel(int f, int n)
{
	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */
	if (n < 0)
		return backdel(f, -n);
	if (f != FALSE) {	/* Really a kill. */
		if ((lastflag & CFKILL) == 0)
			kdelete();
		thisflag |= CFKILL;
	}
	return ldelchar((long) n, f);
}

/*
 * Delete backwards. This is quite easy too, because it's all done with other
 * functions. Just move the cursor back, and delete forwards. Like delete
 * forward, this actually does a kill if presented with an argument. Bound to
 * both "RUBOUT" and "C-H".
 */
int backdel(int f, int n)
{
	int s;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */
	if (n < 0)
		return forwdel(f, -n);
	if (f != FALSE) {	/* Really a kill. */
		if ((lastflag & CFKILL) == 0)
			kdelete();
		thisflag |= CFKILL;
	}
	if ((s = backchar(f, n)) == TRUE)
		s = ldelchar(n, f);
	return s;
}

/*
 * Kill text. If called without an argument, it kills from dot to the end of
 * the line, unless it is at the end of the line, when it kills the newline.
 * If called with an argument of 0, it kills from the start of the line to dot.
 * If called with a positive argument, it kills from dot forward over that
 * number of newlines. If called with a negative argument it kills backwards
 * that number of newlines. Normally bound to "C-K".
 */
int killtext(int f, int n)
{
	struct line *nextp;
	long chunk;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if */
		return rdonly();	/* we are in read only mode */
	if ((lastflag & CFKILL) == 0)	/* Clear kill buffer if */
		kdelete();	/* last wasn't a kill. */
	thisflag |= CFKILL;
	if (f == FALSE) {
		chunk = llength(curwp->w_dotp) - curwp->w_doto;
		if (chunk == 0)
			chunk = 1;
	} else if (n == 0) {
		chunk = curwp->w_doto;
		curwp->w_doto = 0;
	} else if (n > 0) {
		chunk = llength(curwp->w_dotp) - curwp->w_doto + 1;
		nextp = lforw(curwp->w_dotp);
		while (--n) {
			if (nextp == curbp->b_linep)
				return FALSE;
			chunk += llength(nextp) + 1;
			nextp = lforw(nextp);
		}
	} else {
		mlwrite("neg kill");
		return FALSE;
	}
	return ldelete(chunk, TRUE);
}

int setemode(int f, int n)
{
	return adjustmode(TRUE, FALSE);
}

int delmode(int f, int n)
{
	return adjustmode(FALSE, FALSE);
}

int setgmode(int f, int n)
{
	return adjustmode(TRUE, TRUE);
}

int delgmode(int f, int n)
{
	return adjustmode(FALSE, TRUE);
}

/*
 * change the editor mode status
 *
 * int kind;		true = set,          false = delete
 * int global;		true = global flag,  false = current buffer flag
 */
int adjustmode(int kind, int global)
{
	char *scan;		/* scanning pointer to convert prompt */
	int i;			/* loop index */
	int status;		/* error return on input */
#if COLOR
	int uflag;		/* was modename uppercase? */
#endif
	char prompt[50];	/* string to prompt user with */
	char cbuf[NPAT];	/* buffer to recieve mode name into */

	/* build the proper prompt string */
	if (global)
		strcpy(prompt, "Global mode to ");
	else
		strcpy(prompt, "Mode to ");

	if (kind == TRUE)
		strcat(prompt, "add: ");
	else
		strcat(prompt, "delete: ");

	/* prompt the user and get an answer */

	status = mlreply(prompt, cbuf, NPAT - 1);
	if (status != TRUE)
		return status;

	/* make it uppercase */

	scan = cbuf;
#if COLOR
	uflag = (*scan >= 'A' && *scan <= 'Z');
#endif
	while (*scan != 0) {
		if (*scan >= 'a' && *scan <= 'z')
			*scan = *scan - 32;
		scan++;
	}

	/* test it first against the colors we know */

	for (i = 0; i < NCOLORS; i++) {
		if (strcmp(cbuf, cname[i]) == 0) {
			/* finding the match, we set the color */
#if COLOR
			if (uflag) {
				if (global)
					gfcolor = i;
#if PKCODE == 0
				else
#endif
					curwp->w_fcolor = i;
			} else {
				if (global)
					gbcolor = i;
#if PKCODE == 0
				else
#endif
					curwp->w_bcolor = i;
			}

			curwp->w_flag |= WFCOLR;
#endif
			mlerase();
			return TRUE;
		}
	}

	/* test it against the modes we know */

	for (i = 0; i < NUMMODES; i++) {
		if (strcmp(cbuf, modename[i]) == 0) {
			int val = modevalue[i];
			/* finding a match, we process it */
			if (kind == TRUE) {
				if (global)
					gmode |= val;
				else
					curbp->b_mode |= val;
			} else {
				if (global)
					gmode &= ~val;
				else
					curbp->b_mode &= ~val;
			}
			/* display new mode line */
			if (global == 0)
				upmode();
			mlerase();
			return TRUE;
		}
	}

	mlwrite("No such mode!");
	return FALSE;
}
