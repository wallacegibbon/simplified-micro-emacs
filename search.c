#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"
#include <unistd.h>

static int readpattern(char *prompt, char *apat, int srch);
static int replaces(int kind, int f, int n);
static int nextch(struct line **pcurline, int *pcuroff, int dir);

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
int filter_buffer(int f, int n)
{
	int s;			/* return status from CLI */
	struct buffer *bp;	/* pointer to buffer to zot */
	char line[NLINE];	/* command line send to shell */
	char tmpnam[NFILEN];	/* place to store real file name */

	static char bname1[] = "fltinp";
	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";

	if (curbp->b_mode & MDVIEW)
		return rdonly();

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
 * reset the "." to be at the start or just after the match string,
 * and (perhaps) repaint the display.
 *
 * unsigned char *patrn;	string to scan for
 * int direct;			which way to go.
 * int beg_or_end;		put point at beginning or end of pattern.
 */
int scanner(const char *patrn, int direct, int beg_or_end)
{
	struct line *curline = curwp->w_dotp;
	int curoff = curwp->w_doto;
	struct line *scanline;
	int scanoff;
	const char *patptr;	/* pointer into pattern */
	int c;

	/*
	 * If we are going in reverse, then the 'end' is actually the
	 * beginning of the pattern.  Toggle it.
	 */
	beg_or_end ^= direct;

	while (!boundry(curline, curoff, direct)) {
		/*
		 * Save the current position in case we match the search
		 * string at this point.
		 */
		matchline = curline;
		matchoff = curoff;

		/*
		 * Get the character resolving newlines, and test it against
		 * first char in pattern.
		 */
		c = nextch(&curline, &curoff, direct);

		if (eq(c, patrn[0])) {	/* if we find it.. */
			/* Setup scanning pointers. */
			scanline = curline;
			scanoff = curoff;
			patptr = patrn;

			/* Scan through the pattern for a match. */
			while (*++patptr != '\0') {
				c = nextch(&scanline, &scanoff, direct);
				if (!eq(c, *patptr))
					goto fail;
			}

			/* A SUCCESSFULL MATCH!!! */

			if (beg_or_end == PTEND) {	/* at end of string */
				curwp->w_dotp = scanline;
				curwp->w_doto = scanoff;
			} else {	/* at beginning of string */
				curwp->w_dotp = matchline;
				curwp->w_doto = matchoff;
			}

			curwp->w_flag |= WFMOVE;
			return TRUE;

		}
fail:
		;/* continue to search */
	}
	return FALSE;
}

/*
 * eq -- Compare two characters.  The "bc" comes from the buffer, "pc"
 * from the pattern.  If we are not in EXACT mode, fold out the case.
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
 * Apat is not updated if the user types in an empty line.  If
 * the user typed an empty line, and there is no old pattern, it is
 * an error.  Display the old pattern, in the style of Jeff Lomicka.
 * There is some do-it-yourself control expansion.  Change to using
 * <META> to delimit the end-of-pattern to allow <NL>s in the search
 * string.
 */
static int readpattern(char *prompt, char *apat, int srch)
{
	char tpat[NPAT + 20];
	int status;

	strcpy(tpat, prompt);
	strcat(tpat, " (");
	expandp(apat, &tpat[strlen(tpat)], NPAT / 2);
	strcat(tpat, ")<CR>: ");

	/* Read a pattern.  Either we get one,
	 * or we just get the CR charater, and use the previous pattern.
	 * Then, if it's the search string, make a reversed pattern.
	 * *Then*, make the meta-pattern, if we are defined that way.
	 */
	if ((status = mlreplyt(tpat, tpat, NPAT, enterc)) == TRUE) {
		strcpy(apat, tpat);
		if (srch) {	/* If we are doing the search string. */
			/*
			 * Reverse string copy, and remember the length for
			 * substitution purposes.
			 */
			rvstrcpy(tap, apat);
			mlenold = matchlen = strlen(apat);
		}
	} else if (status == FALSE && apat[0] != 0) {	/* Old one */
		status = TRUE;
	}
	return status;
}

/*
 * savematch -- We found the pattern?  Let's save it away.
 */
void savematch(void)
{
	char *ptr;		/* pointer to last match string */
	unsigned int j;
	struct line *curline;	/* line of last match */
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
 * qreplace -- search and replace with query.
 */
int qreplace(int f, int n)
{
	return replaces(TRUE, f, n);
}

/*
 * delins -- Delete a specified length from the current point
 * then either insert the string directly, or make use of
 * replacement meta-array.
 */
int delins(int dlength, char *instr, int use_meta)
{
       int status;

       /* Zap what we gotta, and insert its replacement. */
       if ((status = ldelete((long)dlength, FALSE)) != TRUE)
               mlwrite("%%ERROR while deleting");
       else
               status = linstr(instr);

       return status;
}

/*
 * replaces -- Search for a string and replace it with another
 * string.  Query might be enabled (according to kind).
 *
 * int kind;		Query enabled flag
 */
static int replaces(int kind, int f, int n)
{
	int nlflag;		/* last char of search string a <NL>? */
	int nlrepl;		/* was a replace done on the last line? */
	char tpat[NPAT];
	int numsub, nummatch, status, c, last_char = 0;

	if (curbp->b_mode & MDVIEW)
		return rdonly();

	if (f && n < 0)
		return FALSE;

	/* Ask the user for the text of a pattern. */
	if ((status = readpattern((kind == FALSE ? "Replace" : "Query replace"),
			pat, TRUE)) != TRUE)
		return status;

	/* Ask for the replacement string. */
	if ((status = readpattern("with", rpat, FALSE)) == ABORT)
		return status;

	/*
	 * Set up flags so we can make sure not to do a recursive
	 * replace on the last line.
	 */
	nlflag = (pat[matchlen - 1] == '\n');
	nlrepl = FALSE;

	if (kind) {
		/* Build query replace question string. */
		strcpy(tpat, "Replace '");
		expandp(pat, &tpat[strlen(tpat)], NPAT / 3);
		strcat(tpat, "' with '");
		expandp(rpat, &tpat[strlen(tpat)], NPAT / 3);
		strcat(tpat, "'? ");
	}

	numsub = 0;
	nummatch = 0;

	while ((f == FALSE || n > nummatch) &&
			(nlflag == FALSE || nlrepl == FALSE)) {
		if (!scanner(pat, FORWARD, PTBEG))
			break;	/* all done */

		++nummatch;

		/* Check if we are on the last line. */
		nlrepl = (lforw(curwp->w_dotp) == curwp->w_bufp->b_linep);

		if (kind) {
			mlwrite(tpat, pat, rpat);
qprompt:
			update(TRUE);	/* show the proposed place to change */
			c = tgetc();	/* and input */
			mlwrite("");	/* and clear it */

			/* And respond appropriately. */
			last_char = c;
			switch (c) {
			case 'y':
			case ' ':
				savematch();
				break;

			case 'n':
				forwchar(FALSE, 1);
				continue;

			case '!':
				kind = FALSE;
				break;

			case BELL:
				mlwrite("%d substitutions", numsub);
				return FALSE;

			default:
				TTbeep();
				/* fallthrough */
			case '?':
				mlwrite("(Y)es, (N)o, (!)Do rest, (^G)Abort, (?)Help: ");
				goto qprompt;

			}
		}

		/* Delete the sucker, and insert its replacement. */
		status = delins(matchlen, rpat, TRUE);
		if (status != TRUE)
			return status;

		numsub++;
	}

	if (last_char == 'n')
		backchar(FALSE, 1);

	mlwrite("%d substitutions", numsub);
	return TRUE;
}

/*
 * expandp -- Expand control key sequences for output.
 *
 * char *srcstr;		string to expand
 * char *deststr;		destination of expanded string
 * int maxlength;		maximum chars in destination
 */
int expandp(const char *srcstr, char *deststr, int maxlength)
{
	unsigned char c;

	while ((c = *srcstr++) != 0) {
		if (c < 0x20 || c == 0x7F) {	/* control character */
			*deststr++ = '^';
			*deststr++ = c ^ 0x40;
			maxlength -= 2;
		} else if (c == '%') {
			*deststr++ = '%';
			*deststr++ = '%';
			maxlength -= 2;
		} else {

			*deststr++ = c;
			maxlength--;
		}

		if (maxlength < 3) {
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
	if (dir == FORWARD)
		return (curoff == llength(curline)) &&
			(lforw(curline) == curbp->b_linep);
	else
		return (curoff == 0) &&
			(lback(curline) == curbp->b_linep);
}

/*
 * nextch -- retrieve the next/previous character in the buffer,
 * and advance/retreat the point.
 * The order in which this is done is significant, and depends upon the
 * direction of the search.
 * Forward searches look at the current character and move;
 * Reverse searches move and look at the character.
 */
static int nextch(struct line **pcurline, int *pcuroff, int dir)
{
	struct line *curline = *pcurline;
	int curoff = *pcuroff;
	int c;

	if (dir == FORWARD) {
		if (curoff == llength(curline)) {	/* if at EOL */
			curline = lforw(curline);
			curoff = 0;
			c = '\n';	/* and return a <NL> */
		} else {
			c = lgetc(curline, curoff++);
		}
	} else {
		if (curoff == 0) {
			curline = lback(curline);
			curoff = llength(curline);
			c = '\n';
		} else {
			c = lgetc(curline, --curoff);
		}

	}
	*pcurline = curline;
	*pcuroff = curoff;
	return c;
}
