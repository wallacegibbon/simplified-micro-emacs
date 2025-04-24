/* word.c
 *
 *      The routines in this file implement commands that work word or a
 *      paragraph at a time.  There are all sorts of word mode commands.  If I
 *      do any sentence mode commands, they are likely to be put in this file.
 *
 *	Modified by Petri Kutvonen
 */

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

/* Word wrap on n-spaces. Back-over whatever precedes the point on the current
 * line and stop on the first word-break or the beginning of the line. If we
 * reach the beginning of the line, jump back to the end of the word and start
 * a new line.	Otherwise, break the line at the word-break, eat it, and jump
 * back to the end of the word.
 * Returns TRUE on success, FALSE on errors.
 */
int wrapword(int f, int n)
{
	int cnt;	/* size of word wrapped to next line */
	int c;		/* charector temporary */

	/* backup from the <NL> 1 char */
	if (!backchar(0, 1))
		return FALSE;

	/*
	 * Back up until we aren't in a word, make sure there is a break
	 * in the line.
	 */
	cnt = 0;
	while (((c = lgetc(curwp->w_dotp, curwp->w_doto)) != ' ') &&
			(c != '\t')) {
		cnt++;
		if (!backchar(0, 1))
			return FALSE;
		/* if we make it to the beginning, start a new line */
		if (curwp->w_doto == 0) {
			gotoeol(FALSE, 0);
			return lnewline();
		}
	}

	/* delete the forward white space */
	if (!forwdel(0, 1))
		return FALSE;

	/* put in a end of line */
	if (!lnewline())
		return FALSE;

	/* and past the first word */
	while (cnt-- > 0) {
		if (forwchar(FALSE, 1) == FALSE)
			return FALSE;
	}
	return TRUE;
}

/*
 * Move the cursor backward by "n" words. All of the details of motion are
 * performed by the "backchar" and "forwchar" routines. Error if you try to
 * move beyond the buffers.
 */
int backword(int f, int n)
{
	if (n < 0)
		return forwword(f, -n);
	if (backchar(FALSE, 1) == FALSE)
		return FALSE;
	while (n--) {
		while (inword() == FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return FALSE;
		}
		while (inword() != FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return FALSE;
		}
	}
	return forwchar(FALSE, 1);
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwchar". Error if you try and move beyond the buffer's end.
 */
int forwword(int f, int n)
{
	if (n < 0)
		return backword(f, -n);
	while (n--) {
		while (inword() == TRUE) {
			if (forwchar(FALSE, 1) == FALSE)
				return FALSE;
		}

		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return FALSE;
		}
	}
	return TRUE;
}

/*
 * Move the cursor forward by the specified number of words. As you move,
 * convert any characters to upper case. Error if you try and move beyond the
 * end of the buffer. Bound to "M-U".
 */
int upperword(int f, int n)
{
	int c;

	if (curbp->b_mode & MDVIEW)
		return rdonly();
	if (n < 0)
		return FALSE;

	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return FALSE;
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (islower(c)) {
				c -= 'a' - 'A';
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1) == FALSE)
				return FALSE;
		}
	}
	return TRUE;
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert characters to lower case. Error if you try and move over the end of
 * the buffer. Bound to "M-L".
 */
int lowerword(int f, int n)
{
	int c;

	if (curbp->b_mode & MDVIEW)
		return rdonly();
	if (n < 0)
		return FALSE;

	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return FALSE;
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (isupper(c)) {
				c += 'a' - 'A';
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1) == FALSE)
				return FALSE;
		}
	}
	return TRUE;
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert the first character of the word to upper case, and subsequent
 * characters to lower case. Error if you try and move past the end of the
 * buffer. Bound to "M-C".
 */
int capword(int f, int n)
{
	int c;

	if (curbp->b_mode & MDVIEW)
		return rdonly();
	if (n < 0)
		return FALSE;

	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return FALSE;
		}
		if (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (islower(c)) {
				c -= 'a' - 'A';
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1) == FALSE)
				return FALSE;
			while (inword() != FALSE) {
				c = lgetc(curwp->w_dotp, curwp->w_doto);
				if (isupper(c)) {
					c += 'a' - 'A';
					lputc(curwp->w_dotp, curwp->w_doto, c);
					lchange(WFHARD);
				}
				if (forwchar(FALSE, 1) == FALSE)
					return FALSE;
			}
		}
	}
	return TRUE;
}

/*
 * Kill forward by "n" words. Remember the location of dot. Move forward by
 * the right number of words. Put dot back where it was and issue the kill
 * command for the right number of characters. With a zero argument, just
 * kill one word and no whitespace. Bound to "M-D".
 */
int delfword(int f, int n)
{
	struct line *dotp;	/* original cursor line */
	int doto;		/* and row */
	int c;			/* temp char */
	long size;		/* # of chars to delete */

	if (curbp->b_mode & MDVIEW)
		return rdonly();
	if (n < 0)
		return FALSE;

	/* Clear the kill buffer if last command wasn't a kill */
	if ((lastflag & CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;	/* this command is a kill */

	/* save the current cursor position */
	dotp = curwp->w_dotp;
	doto = curwp->w_doto;

	/* figure out how many characters to give the axe */
	size = 0;

	/* get us into a word.... */
	while (inword() == FALSE) {
		if (forwchar(FALSE, 1) == FALSE)
			return FALSE;
		++size;
	}

	if (n == 0) {
		/* skip one word, no whitespace! */
		while (inword() == TRUE) {
			if (forwchar(FALSE, 1) == FALSE)
				return FALSE;
			++size;
		}
	} else {
		/* skip n words.... */
		while (n--) {

			/* if we are at EOL; skip to the beginning of the next */
			while (curwp->w_doto == llength(curwp->w_dotp)) {
				if (forwchar(FALSE, 1) == FALSE)
					return FALSE;
				++size;
			}

			/* move forward till we are at the end of the word */
			while (inword() == TRUE) {
				if (forwchar(FALSE, 1) == FALSE)
					return FALSE;
				++size;
			}

			/* if there are more words, skip the interword stuff */
			if (n != 0)
				while (inword() == FALSE) {
					if (forwchar(FALSE, 1) == FALSE)
						return FALSE;
					++size;
				}
		}

		/* skip whitespace and newlines */
		while ((curwp->w_doto == llength(curwp->w_dotp)) ||
				((c = lgetc(curwp->w_dotp, curwp->w_doto))
						== ' ') ||
				(c == '\t')) {
			if (forwchar(FALSE, 1) == FALSE)
				break;
			++size;
		}
	}

	/* restore the original position and delete the words */
	curwp->w_dotp = dotp;
	curwp->w_doto = doto;
	return ldelete(size, TRUE);
}

/*
 * Kill backwards by "n" words. Move backwards by the desired number of words,
 * counting the characters. When dot is finally moved to its resting place,
 * fire off the kill command. Bound to "M-Rubout" and to "M-Backspace".
 */
int delbword(int f, int n)
{
	long size;

	if (curbp->b_mode & MDVIEW)
		return rdonly();
	if (n <= 0)
		return FALSE;

	/* Clear the kill buffer if last command wasn't a kill */
	if ((lastflag & CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;	/* this command is a kill */

	if (backchar(FALSE, 1) == FALSE)
		return FALSE;
	size = 0;
	while (n--) {
		while (inword() == FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return FALSE;
			++size;
		}
		while (inword() != FALSE) {
			++size;
			if (backchar(FALSE, 1) == FALSE)
				goto bckdel;
		}
	}
	if (forwchar(FALSE, 1) == FALSE)
		return FALSE;

bckdel:
	return ldelchar(size, TRUE);
}

/*
 * Return TRUE if the character at dot is a character that is considered to be
 * part of a word. The word character list is hard coded. Should be setable.
 */
int inword(void)
{
	int c;

	if (curwp->w_doto == llength(curwp->w_dotp))
		return FALSE;
	c = lgetc(curwp->w_dotp, curwp->w_doto);
	if (isletter(c))
		return TRUE;
	if (c >= '0' && c <= '9')
		return TRUE;
	return FALSE;
}
