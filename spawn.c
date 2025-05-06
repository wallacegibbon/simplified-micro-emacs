#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include <stdio.h>
#include <unistd.h>

#if UNIX
#include <signal.h>
#ifdef SIGWINCH
extern int chg_width, chg_height;
void sizesignal(int);
#endif
#endif

/*
 * Create a subjob with a copy of the command intrepreter in it.  When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C".
 */
int spawncli(int f, int n)
{
#if UNIX
	char *cp;
	movecursor(term.t_nrow, 0);	/* Seek to last line. */
	TTflush();
	TTclose();		/* stty to old settings */
	TTkclose();		/* Close "keyboard" */
	if ((cp = getenv("SHELL")) != NULL && *cp != '\0')
		system(cp);
	else
#if BSD
		system("exec /bin/csh");
#else
		system("exec /bin/sh");
#endif
	sgarbf = TRUE;

	/*
	sleep(2);
	*/

	TTopen();
	TTkopen();
#ifdef SIGWINCH
/*
 * This fools the update routines to force a full
 * redraw with complete window size checking.
 *		-lbt
 */
	chg_width = term.t_ncol;
	chg_height = term.t_nrow + 1;
	term.t_nrow = term.t_ncol = 0;
#endif
	return TRUE;
#endif
}

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
int spawn(int f, int n)
{
	char line[NLINE];
	int s;

#if UNIX
	if ((s = mlreply("!", line, NLINE)) != TRUE)
		return s;
	TTflush();
	TTclose();		/* stty to old modes */
	TTkclose();
	system(line);
	fflush(stdout);		/* to be sure P.K. */
	TTopen();

	mlputs("(End)");	/* Pause. */
	TTflush();
	while ((s = tgetc()) != '\r' && s != ' ');
	mlputs("\r\n");

	TTkopen();
	sgarbf = TRUE;
	return TRUE;
#endif
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
int pipecmd(int f, int n)
{
	struct window *wp;	/* pointer to new window */
	struct buffer *bp;	/* pointer to buffer to zot */
	char line[NLINE];	/* command line send to shell */
	int s;			/* return status from CLI */

	static char bname[] = "_me_cmd_tmp";
	static char filename[NSTRING] = "_me_cmd_tmp";

	/* get the command to pipe in */
	if ((s = mlreply("@", line, NLINE)) != TRUE)
		return s;

	/* get rid of the command output buffer if it exists */
	if ((bp = bfind(bname, FALSE, 0)) != FALSE) {
		/* try to make sure we are off screen */
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_bufp == bp) {
#if PKCODE
				if (wp == curwp)
					delwind(FALSE, 1);
				else
					onlywind(FALSE, 1);
				break;
#else
				onlywind(FALSE, 1);
				break;
#endif
			}
			wp = wp->w_wndp;
		}
		if (zotbuf(bp) != TRUE)
			return FALSE;
	}
#if UNIX
	TTflush();
	TTclose();		/* stty to old modes */
	TTkclose();
	strcat(line, ">");
	strcat(line, filename);
	system(line);
	TTopen();
	TTkopen();
	TTflush();
	sgarbf = TRUE;
	s = TRUE;
#endif

	if (s != TRUE)
		return s;

	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE)
		return FALSE;

	/* and read the stuff in */
	if (getfile(filename, FALSE) == FALSE)
		return FALSE;

	/* make this window in VIEW mode, update all mode lines */
	curwp->w_bufp->b_mode |= MDVIEW;
	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}

	/* and get rid of the temporary file */
	unlink(filename);
	return TRUE;
}

/*
 * filter a buffer through an external program
 * Bound to ^X #
 */
int filter_buffer(int f, int n)
{
	struct buffer *bp;		/* pointer to buffer to zot */
	char line[NLINE];		/* command line send to shell */
	char tmpnam[NFILEN];		/* place to store real file name */
	int s;				/* return status from CLI */

	static char bname[] = "_me_filter_tmp";
	static char filename_in[] = "_me_filter_tmp_in";
	static char filename_out[] = "_me_filter_tmp_out";

	if (curbp->b_mode & MDVIEW)
		return rdonly();

	/* get the filter name and its args */
	if ((s = mlreply("#", line, NLINE)) != TRUE)
		return s;

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);	/* save the original name */
	strcpy(bp->b_fname, bname);	/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filename_in) != TRUE) {
		mlwrite("(Cannot write filter file)");
		strcpy(bp->b_fname, tmpnam);
		return FALSE;
	}

#if UNIX
	TTputc('\n');			/* Already have '\r' */
	TTflush();
	TTclose();			/* stty to old modes */
	TTkclose();
	strcat(line, "<");
	strcat(line, filename_in);
	strcat(line, ">");
	strcat(line, filename_out);
	system(line);
	TTopen();
	TTkopen();
	TTflush();
	sgarbf = TRUE;
	s = TRUE;
#endif

	/* on failure, escape gracefully */
	if (s != TRUE || (readin(filename_out, FALSE) == FALSE)) {
		mlwrite("(Execution failed)");
		strcpy(bp->b_fname, tmpnam);
		unlink(filename_in);
		unlink(filename_out);
		return s;
	}

	/* reset file name */
	strcpy(bp->b_fname, tmpnam);	/* restore name */
	bp->b_flag |= BFCHG;		/* flag it as changed */

	/* and get rid of the temporary file */
	unlink(filename_in);
	unlink(filename_out);
	return TRUE;
}
