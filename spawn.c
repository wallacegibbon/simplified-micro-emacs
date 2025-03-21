/* spawn.c
 *
 *	Various operating system access commands.
 *
 *	<odified by Petri Kutvonen
 */

#include <stdio.h>
#include <unistd.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"

#if VMS
#define EFN     0		/* Event flag. */

#include <ssdef.h>	/* Random headers. */
#include <stsdef.h>
#include <descrip.h>
#include <iodef.h>

extern int oldmode[3];		/* In "termio.c" */
extern int newmode[3];		/* In "termio.c" */
extern short iochan;		/* In "termio.c" */
#endif

#if V7 | USG | BSD
#include <signal.h>
#ifdef SIGWINCH
extern int chg_width, chg_height;
void sizesignal(int);
#endif
#endif

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
int spawncli(int f, int n)
{
#if V7 | USG | BSD
	char *cp;
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

#if VMS
	movecursor(term.t_nrow, 0);	/* In last line. */
	mlputs("(Starting DCL)\r\n");
	TTflush();		/* Ignore "ttcol". */
	sgarbf = TRUE;
	sys(NULL);
	sleep(1);
	mlputs("\r\n(Returning from DCL)\r\n");
	TTflush();
	sleep(1);
	return TRUE;
#endif
#if V7 | USG | BSD
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

	/* ?? what is this sleep for ?? (2025/03/19) */
	//sleep(2);

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

#if BSD | __hpux | SVR4

int bktoshell(int f, int n)
{				/* suspend MicroEMACS and wait to wake up */
	vttidy();
/******************************
	int pid;

	pid = getpid();
	kill(pid,SIGTSTP);
******************************/
	kill(0, SIGTSTP);
	return TRUE;
}

void rtfrmshell(void)
{
	TTopen();
	curwp->w_flag = WFHARD;
	sgarbf = TRUE;
}
#endif

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
int spawn(int f, int n)
{
	int s;
	char line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

#if VMS
	if ((s = mlreply("!", line, NLINE)) != TRUE)
		return s;
	movecursor(term.t_nrow, 0);
	TTflush();
	s = sys(line);		/* Run the command. */

	mlputs("\r\n\n(End)");	/* Pause. */
	TTflush();
	tgetc();

	sgarbf = TRUE;
	return s;
#endif
#if V7 | USG | BSD
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
 * Run an external program with arguments. When it returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X $".
 */

int execprg(int f, int n)
{
	int s;
	char line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

#if VMS
	if ((s = mlreply("!", line, NLINE)) != TRUE)
		return s;
	TTflush();
	s = sys(line);		/* Run the command. */
	mlputs("\r\n\n(End)");	/* Pause. */
	TTflush();
	tgetc();
	sgarbf = TRUE;
	return s;
#endif

#if V7 | USG | BSD
	if ((s = mlreply("!", line, NLINE)) != TRUE)
		return s;
	TTputc('\n');		/* Already have '\r' */
	TTflush();
	TTclose();		/* stty to old modes */
	TTkclose();
	system(line);
	fflush(stdout);		/* to be sure P.K. */
	TTopen();
	mlputs("(End)");	/* Pause. */
	TTflush();
	while ((s = tgetc()) != '\r' && s != ' ');
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
	int s;		/* return status from CLI */
	struct window *wp;	/* pointer to new window */
	struct buffer *bp;	/* pointer to buffer to zot */
	char line[NLINE];	/* command line send to shell */
	static char bname[] = "command";

	static char filnam[NSTRING] = "command";

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

#if VMS
	mlwrite("Not available under VMS");
	return FALSE;
#endif

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
#if V7 | USG | BSD
	TTflush();
	TTclose();		/* stty to old modes */
	TTkclose();
	strcat(line, ">");
	strcat(line, filnam);
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
	if (getfile(filnam, FALSE) == FALSE)
		return FALSE;

	/* make this window in VIEW mode, update all mode lines */
	curwp->w_bufp->b_mode |= MDVIEW;
	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}

	/* and get rid of the temporary file */
	unlink(filnam);
	return TRUE;
}

#if VMS
/*
 * Run a command. The "cmd" is a pointer to a command string, or NULL if you
 * want to run a copy of DCL in the subjob (this is how the standard routine
 * LIB$SPAWN works. You have to do wierd stuff with the terminal on the way in
 * and the way out, because DCL does not want the channel to be in raw mode.
 */
int sys(char *cmd)
{
	struct dsc$descriptor cdsc;
	struct dsc$descriptor *cdscp;
	long status;
	long substatus;
	long iosb[2];

	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status != SS$_NORMAL || (iosb[0] & 0xFFFF) != SS$_NORMAL)
		return FALSE;
	cdscp = NULL;		/* Assume DCL. */
	if (cmd != NULL) {	/* Build descriptor. */
		cdsc.dsc$a_pointer = cmd;
		cdsc.dsc$w_length = strlen(cmd);
		cdsc.dsc$b_dtype = DSC$K_DTYPE_T;
		cdsc.dsc$b_class = DSC$K_CLASS_S;
		cdscp = &cdsc;
	}
	status = LIB$SPAWN(cdscp, 0, 0, 0, 0, 0, &substatus, 0, 0, 0);
	if (status != SS$_NORMAL)
		substatus = status;
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);
	if (status != SS$_NORMAL || (iosb[0] & 0xFFFF) != SS$_NORMAL)
		return FALSE;
	if ((substatus & STS$M_SUCCESS) == 0)	/* Command failed. */
		return FALSE;
	return TRUE;
}
#endif
