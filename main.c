#include <stdio.h>

/* Make global definitions not external. */
#define	maindef

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"
#include "version.h"

#ifndef GOOD
#define GOOD    0
#endif

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
	fputs("      +          start at the end of file\n", stdout);
	fputs("      +<n>       start at line <n>\n", stdout);
	fputs("      -g[G]<n>   go to line <n>\n", stdout);
	fputs("      --help     display this help and exit\n", stdout);
	fputs("      --version  output version information and exit\n", stdout);
	exit(status);
}

int main(int argc, char **argv)
{
	int c = -1;		/* command character */
	int f;			/* default flag */
	int n;			/* numeric repeat count */
	int mflag;		/* negative flag on repeat */
	struct buffer *bp;	/* temp buffer pointer */
	int firstfile;		/* first file flag */
	int carg;		/* current arg to scan */
	struct buffer *firstbp = NULL;
				/* ptr to first buffer in cmd line */
	int basec;		/* c stripped of meta character */
	int viewflag;		/* are we starting in view mode? */
	int gotoflag;		/* do we need to goto a line at start? */
	int gline = 0;		/* if so, what line? */
	int searchflag;		/* Do we need to search at start? */
	int saveflag;		/* temp store for lastflag */
	char bname[NBUFN];	/* buffer name of file to read */
	int newc;

#if PKCODE & BSD
	sleep(1); /* Time for window manager. */
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

	viewflag = FALSE;
	gotoflag = FALSE;
	searchflag = FALSE;
	firstfile = TRUE;

	for (carg = 1; carg < argc; ++carg) {
		if (argv[carg][0] == '-') {
			switch (argv[carg][1]) {
			case 'e':	/* -e for Edit file */
			case 'E':
				viewflag = FALSE;
				break;
			case 'g':	/* -g for initial goto */
			case 'G':
				gotoflag = TRUE;
				gline = atoi(&argv[carg][2]);
				break;
			case 's':	/* -s for initial search string */
			case 'S':
				searchflag = TRUE;
				strncpy(pat, &argv[carg][2], NPAT);
				break;
			case 'v':	/* -v for View File */
			case 'V':
				viewflag = TRUE;
				break;
			default:	/* unknown switch */
				/* ignore this for now */
				break;
			}

		} else {

			/* Process an input file */

			/* set up a buffer for this file */
			makename(bname, argv[carg]);
			unqname(bname);

			/* set this to inactive */
			bp = bfind(bname, TRUE, 0);
			strcpy(bp->b_fname, argv[carg]);
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

	/* Deal with startup gotos and searches */
	if (gotoflag && searchflag) {
		update(FALSE);
		mlwrite("(Can not search and goto at the same time!)");
	} else if (gotoflag) {
		if (gotoline(TRUE, gline) == FALSE) {
			update(FALSE);
			mlwrite("(Bogus goto argument)");
		}
	} else if (searchflag) {
		if (forwhunt(FALSE, 0) == FALSE)
			update(FALSE);
	}

	/* Setup to process commands. */
	lastflag = 0;

loop:
	/* Execute the "command" macro...normally null. */
	saveflag = lastflag;  /* Preserve lastflag through this. */
	lastflag = saveflag;

#if TYPEAH && PKCODE
	if (typahead()) {
		newc = getcmd();
		update(FALSE);
		do {
			fn_t execfunc;

			if (c == newc && (execfunc = getbind(c)) != NULL
					&& execfunc != insert_newline)
				newc = getcmd();
			else
				break;
		} while (typahead());
		c = newc;
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
#if CLRMSG
		if (c == ' ')	/* ITS EMACS does this */
			goto loop;
#endif
	}
	f = FALSE;
	n = 1;

	/* do META-# processing if needed */

	basec = c & ~META;
	if ((c & META) && ((basec >= '0' && basec <= '9') || basec == '-')) {
		f = TRUE;
		n = 0;
		mflag = 1;	/* current minus flag */
		c = basec;	/* strip the META */
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

	if (c == reptc) {
		f = TRUE;
		n = 4;
		mflag = 0;
		mlwrite("Arg: 4");
		while (((c = getcmd()) >= '0' && c <= '9') || c == reptc ||
				c == '-') {
			if (c == reptc) {
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
				n++;
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
#if COLOR
	/* initalize colors to global defaults */
	wp->w_fcolor = gfcolor;
	wp->w_bcolor = gbcolor;
#endif
	wp->w_ntrows = term.t_nrow - 1;	/* "-1" for mode line. */
	wp->w_force = 0;
	wp->w_flag = WFMODE | WFHARD;	/* Full. */
}

/*
 * This function looks a key binding up in the binding table
 */
int (*getbind(int c))(int, int)
{
	struct key_tab *ktp;

	ktp = &keytab[0];
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
	if (c == (CONTROL | 'I'))
		c = '\t';

	if ((c >= 0x20 && c <= 0x7E) || c == '\t') {
		if (n <= 0) {	/* Fenceposts. */
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

		/* check auto-save mode */
		if (curbp->b_mode & MDASAVE) {
			if (--gacount == 0) {
				/* and save the file if needed */
				upscreen(FALSE, 0);
				filesave(FALSE, 0);
				gacount = gasave;
			}
		}
		lastflag = thisflag;
		return status;
	}
	TTbeep();
	mlwrite("(Key not bound)");	/* complain */
	lastflag = 0;			/* Fake last flags. */
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

	if (f != FALSE /* Argument forces it. */
			|| anycb() == FALSE /* All buffers clean. */
			/* User says it's OK. */
			|| (s = mlyesno("Modified buffers exist. Leave anyway")) == TRUE) {
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
			exit(GOOD);
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
	kbdptr = &kbdm[0];
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
	kbdrep = n;		/* remember how many times to execute */
	kbdmode = PLAY;		/* start us in play mode */
	kbdptr = &kbdm[0];	/* at the beginning */
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
 * tell the user that this command is illegal while we are in
 * VIEW (read-only) mode
 */
int rdonly(void)
{
	TTbeep();
	mlwrite("(Key illegal in VIEW mode)");
	return FALSE;
}

/* user function that does NOTHING */
int nullproc(int f, int n)
{
	return TRUE;
}


/* Compiler specific Library functions */

#if RAMSIZE
/*
 * These routines will allow me to track memory usage by placing a layer
 * on top of the standard system malloc() and free() calls.
 * With this code defined, the environment variable, $RAM, will
 * report on the number of bytes allocated via malloc.

 * With SHOWRAM defined, the number is also posted on the
 * end of the bottom mode line and is updated whenever it is changed.
*/

#undef	malloc
#undef	free

char *allocate(unsigned int nbytes)
{
	char *mp;
	char *malloc();

	mp = malloc(nbytes);
	if (mp) {
		envram += nbytes;
#if RAMSHOW
		dspram();
#endif
	}

	return mp;
}

void release(char *mp)
{
	unsigned int *lp;

	if (mp) {
		/* update amount of ram currently malloced */
		lp = ((unsigned *)mp) - 1;
		envram -= (long)*lp - 2;
		free(mp);
#if RAMSHOW
		dspram();
#endif
	}
}

#if RAMSHOW
/* display the amount of RAM currently malloced */
void dspram()
{
	char mbuf[20];
	char *sp;

	TTmove(term.t_nrow - 1, 70);
#if COLOR
	TTforg(7);
	TTbacg(0);
#endif
	sprintf(mbuf, "[%lu]", envram);
	sp = &mbuf[0];
	while (*sp)
		TTputc(*sp++);
	TTmove(term.t_nrow, 0);
	movecursor(term.t_nrow, 0);
}
#endif
#endif

/* On some primitave operation systems, and when emacs is used as
	a subprogram to a larger project, emacs needs to de-alloc its
	own used memory
*/

#if CLEAN

int cexit(int status)
{
	struct buffer *bp;	/* buffer list pointer */
	struct window *wp;	/* window list pointer */
	struct window *tp;	/* temporary window pointer */

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
