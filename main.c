#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>

#if UNIX
#include <signal.h>
static void emergencyexit(int);
#ifdef SIGWINCH
void sizesignal(int);
#endif
#endif

void usage(int status)
{
	printf("Usage: %s [options] [filenames]\n\n", PROGRAM_NAME);
	fputs("      -v[V]<n>   View only <n>\n", stdout);
	fputs("      -g[G]<n>   Go to line <n>\n", stdout);
	fputs("      --help     Display this help and exit\n", stdout);
	fputs("      --version  Output version information and exit\n", stdout);
	exit(status);
}

int main(int argc, char **argv)
{
	struct buffer *firstbp = NULL;	/* ptr to first buffer in cmd line */
	struct buffer *bp;		/* temp buffer pointer */
	char bname[NBUFN];		/* buffer name of file to read */
	int saveflag;			/* temp store for lastflag */
	int carg;			/* current arg to scan */
	int firstfile = TRUE;		/* first file flag */
	int viewflag = FALSE;
	int gotoflag = FALSE;
	int gline = 0;
	fn_t execfunc;
	int mflag;			/* negative flag on repeat */
	int c = 0, c1;
	int f, n;

#if PKCODE & BSD
	sleep(1);			/* Time for window manager. */
#endif

#if UNIX
#ifdef SIGWINCH
	signal(SIGWINCH, sizesignal);
#endif
#endif
	if (argc == 2) {
		if (strcmp(argv[1], "--help") == 0) {
			usage(EXIT_FAILURE);
		}
		if (strcmp(argv[1], "--version") == 0) {
			version();
			exit(EXIT_SUCCESS);
		}
	}

	vtinit();		/* Display */
	edinit("main");		/* Buffers, windows */

	for (carg = 1; carg < argc; ++carg) {
		if (argv[carg][0] == '-') {
			switch (argv[carg][1]) {
			case 'v':
			case 'V':
				viewflag = TRUE;
				break;
			case 'g':
			case 'G':
				gotoflag = TRUE;
				gline = atoi(&argv[carg][2]);
				break;
			default:
				break;
			}
		} else {
			/* Process an input file */
			/* set up a buffer for this file */
			makename(bname, argv[carg]);
			unqname(bname);

			/* set this to inactive */
			bp = bfind(bname, TRUE, 0);
			strncpy(bp->b_fname, argv[carg], NFILEN - 1);
			bp->b_active = FALSE;
			if (firstfile) {
				firstbp = bp;
				firstfile = FALSE;
			}

			/* set the modes appropriatly */
			if (viewflag)
				bp->b_mode |= MDVIEW;
		}
	}

#if UNIX
	signal(SIGHUP, emergencyexit);
	signal(SIGTERM, emergencyexit);
#endif

	/* if there are any files to read, read the first one! */
	bp = bfind("main", FALSE, 0);
	if (firstfile == FALSE && (gflags & GFREAD)) {
		swbuffer(firstbp);
		zotbuf(bp);
	} else
		bp->b_mode |= gmode;

	/* Deal with startup gotos */
	if (gotoflag) {
		if (gotoline(TRUE, gline) == FALSE) {
			update(FALSE);
			mlwrite("(Bogus goto argument)");
		}
	}

loop:
	saveflag = lastflag;
	lastflag = saveflag;

#if TYPEAH && PKCODE
	if (typahead()) {
		c1 = getcmd();
		update(FALSE);
		do {
			if (c == c1 && (execfunc = getbind(c1)) != NULL
					&& execfunc != newline)
				c1 = getcmd();
			else
				break;
		} while (typahead());
		c = c1;
	} else {
		update(FALSE);
		c = getcmd();
	}
#else
	update(FALSE);
	c = getcmd();
#endif
	/* if there is something on the command line, clear it */
	if (mpresf != FALSE) {
		mlerase();
		update(FALSE);
	}

	f = FALSE;
	n = 1;

	/* do META-# processing if needed */

	c1 = c & ~META;
	if ((c & META) && ((c1 >= '0' && c1 <= '9') || c1 == '-')) {
		f = TRUE;
		n = 0;
		mflag = 1;	/* current minus flag */
		c = c1;
		while ((c >= '0' && c <= '9') || (c == '-')) {
			if (c == '-') {
				/* already hit a minus or digit? */
				if ((mflag == -1) || (n != 0))
					break;
				mflag = -1;
			} else {
				n = n * 10 + (c - '0');
			}
			if ((n == 0) && (mflag == -1))
				mlwrite("Arg:");
			else
				mlwrite("Arg: %d", n * mflag);

			c = getcmd();
		}
		n = n * mflag;
	}

	/* do ^U repeat argument processing */

	if (c == REPTC) {
		f = TRUE;
		n = 4;
		mflag = 0;
		mlwrite("Arg: 4");
		while (((c = getcmd()) >= '0' && c <= '9') || c == REPTC ||
				c == '-') {
			if (c == REPTC) {
				if ((n > 0) == ((n * 4) > 0))
					n = n * 4;
				else
					n = 1;
			} else if (c == '-') {
				if (mflag)
					break;
				n = 0;
				mflag = -1;
			} else {
				if (!mflag) {
					n = 0;
					mflag = 1;
				}
				n = 10 * n + c - '0';
			}
			mlwrite("Arg: %d",
				(mflag >= 0) ? n : (n ? -n : -1));
		}
		/*
		 * Make arguments preceded by a minus sign negative and change
		 * the special argument "^U -" to an effective "^U -1".
		 */
		if (mflag == -1) {
			if (n == 0)
				++n;
			n = -n;
		}
	}

	execute(c, f, n);
	goto loop;
}

/*
 * Initialize all of the buffers and windows. The buffer name is passed down
 * as an argument, because the main routine may have been told to read in a
 * file by default, and we want the buffer name to be right.
 */
void edinit(char *bname)
{
	struct buffer *bp;
	struct window *wp;

	bp = bfind(bname, TRUE, 0); /* First buffer */
	blistp = bfind("*List*", TRUE, BFINVS); /* Buffer list buffer */
	wp = malloc(sizeof(struct window)); /* First window */
	if (bp == NULL || wp == NULL || blistp == NULL)
		exit(1);
	curbp = bp;
	prevbp = NULL;
	wheadp = wp;
	curwp = wp;
	wp->w_wndp = NULL;	/* Initialize window */
	wp->w_bufp = bp;
	bp->b_nwnd = 1;		/* Displayed. */
	wp->w_linep = bp->b_linep;
	wp->w_dotp = bp->b_linep;
	wp->w_doto = 0;
	wp->w_markp = NULL;
	wp->w_marko = 0;
	wp->w_toprow = 0;
	wp->w_ntrows = term.t_nrow - 1;	/* "-1" for mode line. */
	wp->w_force = 0;
	wp->w_flag = WFMODE | WFHARD;	/* Full. */
}

/*
 * This function looks a key binding up in the binding table
 */
int (*getbind(int c))(int, int)
{
	struct key_tab *ktp = keytab;

	while (ktp->k_fp != NULL) {
		if (ktp->k_code == c)
			return ktp->k_fp;
		++ktp;
	}
	return NULL;
}

/*
 * This is the general command execution routine. It handles the fake binding
 * of all the keys to "self-insert". It also clears out the "thisflag" word,
 * and arranges to move it to the "lastflag", so that the next command can
 * look at it. Return the status of command.
 */
int execute(int c, int f, int n)
{
	int status;
	fn_t execfunc;

	/* if the keystroke is a bound function...do it */
	execfunc = getbind(c);
	if (execfunc != NULL) {
		thisflag = 0;
		status = execfunc(f, n);
		lastflag = thisflag;
		return status;
	}

	/* No binding found, self inserting. */

	/*
	 * To support unicode or things like it, we need to take care of
	 * unbound prefixed chars (like C-X C-A), they can overlap key codes.
	 */

	/* ASCII is enough for coding, let's keep things simple */

	/* If C-I is not bound, insert it */
	if (c == (CTL | 'I'))
		c = '\t';

	if (isvisible(c)) {
		/* Do not insert when n <= 0 */
		if (n <= 0) {
			lastflag = 0;
			return n < 0 ? FALSE : TRUE;
		}

		thisflag = 0;	/* For the future. */

		/*
		 * if we are in overwrite mode, not at eol,
		 * and next char is not a tab or we are at a tab stop,
		 * delete a char forword
		 */
		if (curwp->w_bufp->b_mode & MDOVER &&
				curwp->w_doto < curwp->w_dotp->l_used &&
				(lgetc(curwp->w_dotp, curwp->w_doto) != '\t' ||
						(curwp->w_doto) % 8 == 7))
			ldelchar(1, FALSE);

		status = linsert(n, c);

		/* check auto-save mode and save the file if needed */
		if (curbp->b_mode & MDASAVE) {
			if (--gacount == 0) {
				update(TRUE);
				filesave(FALSE, 0);
				gacount = gasave;
			}
		}
		lastflag = thisflag;
		return status;
	}
	TTbeep();
	mlwrite("(Key not bound)");
	lastflag = 0;	/* Fake last flags. */
	return FALSE;
}

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit emacs, otherwise simply exit.
 */
int quickexit(int f, int n)
{
	struct buffer *bp;	/* scanning pointer to buffers */
	struct buffer *oldcb;	/* original current buffer */
	int status;

	oldcb = curbp;		/* save in case we fail */

	bp = bheadp;
	while (bp != NULL) {
		if ((bp->b_flag & BFCHG) != 0 /* Changed. */
				&& (bp->b_flag & BFTRUNC) == 0 /* Not truncated P.K. */
				&& (bp->b_flag & BFINVS) == 0) { /* Real. */
			curbp = bp;
			mlwrite("(Saving %s)", bp->b_fname);
			if ((status = filesave(f, n)) != TRUE) {
				curbp = oldcb;	/* restore curbp */
				return status;
			}
		}
		bp = bp->b_bufp;	/* on to the next buffer */
	}
	quit(f, n);		/* conditionally quit */
	return TRUE;
}

static void emergencyexit(int signr)
{
	quickexit(FALSE, 0);
	quit(TRUE, 0);
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out. Normally bound to "C-X C-C".
 */
int quit(int f, int n)
{
	int s;

	if (f != FALSE || anycb() == FALSE /* All buffers clean. */
			|| (s = mlyesno("Modified buffers exist. Quit anyway")) == TRUE) {
#if (FILOCK && BSD) || SVR4
		if (lockrel() != TRUE) {
			TTputc('\n');
			TTputc('\r');
			TTclose();
			TTkclose();
			exit(1);
		}
#endif
		vttidy();
		if (f)
			exit(n);
		else
			exit(0);
	}
	mlwrite("");
	return s;
}

/*
 * Begin a keyboard macro.
 * Error if not at the top level in keyboard processing. Set up variables and
 * return.
 */
int ctlxlp(int f, int n)
{
	if (kbdmode != STOP) {
		mlwrite("%%Macro already active");
		return FALSE;
	}
	mlwrite("(Start macro)");
	kbdptr = kbdm;
	kbdend = kbdptr;
	kbdmode = RECORD;
	return TRUE;
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
int ctlxrp(int f, int n)
{
	if (kbdmode == STOP) {
		mlwrite("%%Macro not active");
		return FALSE;
	}
	if (kbdmode == RECORD) {
		mlwrite("(End macro)");
		kbdmode = STOP;
	}
	return TRUE;
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
int ctlxe(int f, int n)
{
	if (kbdmode != STOP) {
		mlwrite("%%Macro already active");
		return FALSE;
	}
	if (n <= 0)
		return TRUE;
	kbdrep = n;
	kbdmode = PLAY;
	kbdptr = kbdm;
	return TRUE;
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
int ctrlg(int f, int n)
{
	TTbeep();
	kbdmode = STOP;
	mlwrite("(Aborted)");
	return ABORT;
}

/*
 * Tell the user that this command is illegal while we are in VIEW mode.
 */
int rdonly(void)
{
	TTbeep();
	mlwrite("(Key illegal in VIEW mode)");
	return FALSE;
}

/* User function that does NOTHING */
int nullproc(int f, int n)
{
	return TRUE;
}

/* Compiler specific Library functions */

/*
 * On some primitave operation systems, and when emacs is used as a subprogram
 * to a larger project, emacs needs to de-alloc its own used memory.
 */

#if CLEAN

int cexit(int status)
{
	struct buffer *bp;
	struct window *wp, *tp;

	/* first clean up the windows */
	wp = wheadp;
	while (wp) {
		tp = wp->w_wndp;
		free(wp);
		wp = tp;
	}
	wheadp = NULL;

	/* then the buffers */
	bp = bheadp;
	while (bp) {
		bp->b_nwnd = 0;
		bp->b_flag = 0;	/* don't say anything about a changed buffer! */
		zotbuf(bp);
		bp = bheadp;
	}

	/* and the kill buffer */
	kdelete();

	/* and the video buffers */
	vtfree();

#undef	exit
	exit(status);
}
#endif
