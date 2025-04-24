#include <stdio.h>
#include <unistd.h>
#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "wrapper.h"

int quotec = 0x11;		/* quote char during mlreply() */

/* CAUTION:
 * Prefixed characters (like `CONTROL | 'A'`) may be put into this variable,
 * which should be okay since functions like `ctoec` will keep it unchanged.
 */
int reeat_char = -1;

/*
 * Ask a yes or no question in the message line. Return either TRUE, FALSE, or
 * ABORT. The ABORT status is returned if the user bumps out of the question
 * with a ^G. Used any time a confirmation is required.
 */
int mlyesno(char *prompt)
{
	char c;			/* input character */
	char buf[NPAT];		/* prompt to user */

	for (;;) {
		/* build and prompt the user */
		strcpy(buf, prompt);
		strcat(buf, " (y/n)? ");
		mlwrite(buf);

		/* get the responce */
		c = tgetc();

		if (c == ectoc(abortc))	/* Bail out! */
			return ABORT;

		if (c == 'y' || c == 'Y')
			return TRUE;

		if (c == 'n' || c == 'N')
			return FALSE;
	}
}

/*
 * Write a prompt into the message line, then read back a response. Keep
 * track of the physical position of the cursor. If we are in a keyboard
 * macro throw the prompt away, and return the remembered response. This
 * lets macros run at full speed. The reply is always terminated by a carriage
 * return. Handle erase, kill, and abort keys.
 */

int mlreply(char *prompt, char *buf, int nbuf)
{
	return getstring(prompt, buf, nbuf, ctoec('\n'));
}

int mlreplyt(char *prompt, char *buf, int nbuf, int eolchar)
{
	return getstring(prompt, buf, nbuf, eolchar);
}

/*
 * ectoc:
 *	expanded character to character
 *	collapse the CONTROL flags back into an ascii code
 */
int ectoc(int c)
{
	if (c & CONTROL)
		c = ~CONTROL & (c - '@');
	return c;
}

/*
 * ctoec:
 *	character to extended character
 *	pull out the CONTROL prefixes (if possible)
 */
int ctoec(int c)
{
	if (c >= 0x00 && c <= 0x1F)
		c = CONTROL | (c + '@');
	return c;
}

/*
 * match fname to a function in the names table
 * and return any match or NULL if none
 *
 * char *fname;		name to attempt to match
 */
int (*fncmatch(char *fname))(int, int)
{
	struct name_bind *ffp;

	/* scan through the table, returning any match */
	for (ffp = names; ffp->n_func != NULL; ffp++) {
		if (strcmp(fname, ffp->n_name) == 0)
			return ffp->n_func;
	}
	return NULL;
}

fn_t getname(void)
{
	int cpos = 0;		/* current column on screen output */
	char buf[NSTRING];	/* buffer to hold tentative command name */
	int c;

	/* build a name string from the keyboard */
	while (TRUE) {
		c = tgetc();

		/* if we are at the end, just match it */
		if (c == 0x0D) {
			buf[cpos] = 0;

			/* and match it off */
			return fncmatch(buf);

		} else if (c == ectoc(abortc)) {	/* Bell, abort */
			ctrlg(FALSE, 0);
			TTflush();
			return NULL;

		} else if (c == 0x7F || c == 0x08) {	/* rubout/erase */
			if (cpos != 0) {
				TTputc('\b');
				TTputc(' ');
				TTputc('\b');
				--ttcol;
				--cpos;
				TTflush();
			}

		} else {
			if (cpos < NSTRING - 1 && c > ' ') {
				buf[cpos++] = c;
				TTputc(c);
			}

			++ttcol;
			TTflush();
		}
	}
}

/* tgetc:	Get a key from the terminal driver, resolve any keyboard
		macro action */

int tgetc(void)
{
	int c;

	if (reeat_char != -1) {
		c = reeat_char;
		reeat_char = -1;
		return c;
	}

	/* if we are playing a keyboard macro back, */
	if (kbdmode == PLAY) {

		/* if there is some left... */
		if (kbdptr < kbdend)
			return (int)*kbdptr++;

		/* at the end of last repitition? */
		if (--kbdrep < 1) {
			kbdmode = STOP;
#if VISMAC == 0
			/* force a screen update after all is done */
			update(FALSE);
#endif
		} else {

			/* reset the macro to the begining for the next rep */
			kbdptr = kbdm;
			return (int)*kbdptr++;
		}
	}

	/* fetch a character from the terminal driver */
	c = TTgetc();

	/* record it for $lastkey */
	lastkey = c;

	/* save it if we need to */
	if (kbdmode == RECORD) {
		*kbdptr++ = c;
		kbdend = kbdptr;

		/* don't overrun the buffer */
		if (kbdptr == &kbdm[NKBDM - 1]) {
			kbdmode = STOP;
			TTbeep();
		}
	}

	/* and finally give the char back */
	return c;
}

/* GET1KEY:	Get one keystroke. The only prefixs legal here are the
		CONTROL prefixes.
 */

int get1key(void)
{
	return ctoec(tgetc());
}

/* GETCMD:	Get a command from the keyboard. Process all applicable
		prefix keys.
 */
int getcmd(void)
{
	int c = get1key();
	int cmask = 0;

	if (c == 128 + 27)
		goto handle_CSI;

	/* process META prefix */
	if (c == metac) {
#if VT220
proc_metac:
#endif
		c = get1key();
#if VT220
		if (c == '[' || c == 'O') {
		/* if CSI is not handled, events like scrolling won't work */
handle_CSI:
			c = get1key();
			if (c == 'A') {
				return cmask | CONTROL | 'P';
			} else if (c == 'B') {
				return cmask | CONTROL | 'N';
			} else if (c == 'C') {
				return cmask | CONTROL | 'F';
			} else if (c == 'D') {
				return cmask | CONTROL | 'B';
			} else if (c >= 'E' && c <= 'z') {
				return CTLX | META | CONTROL | 'Z';
			} else {
				/* There are many other CSI related keys */
				get1key();
				return CTLX | META | CONTROL | 'Z';
			}
		} else if (c == metac) {
			cmask |= META;
			goto proc_metac;
		} else if (c == ctlxc) {
			goto proc_ctlxc;
		} else {
			cmask |= META;
		}
#endif

		/* Force to upper to match bind configurations */
		if (islower(c))
			c ^= DIFCASE;
		c = ctoec(c);
		return cmask | c;
	}

	/* process CTLX prefix */
	if (c == ctlxc) {
#if VT220
proc_ctlxc:
#endif
		cmask |= CTLX;
		c = get1key();
#if VT220
		if (c == metac) {
			cmask |= META;
			goto proc_metac;
		}
#endif
		if (islower(c))
			c ^= DIFCASE;
		else
			c = ctoec(c);
		return cmask | c;
	}
	/* otherwise, just return it */
	return c;
}

/*
 * A more generalized prompt/reply function allowing the caller
 * to specify the proper terminator. If the terminator is not
 * a return ('\n') it will echo as "<NL>"
 */
int getstring(char *prompt, char *buf, int nbuf, int eolchar)
{
	int cpos = 0;		/* current character position in string */
	int quotef = FALSE;	/* are we quoting the next char? */
	int c;

	/* prompt the user for the input string */
	mlwrite(prompt);

	for (;;) {
		/* get a character from the user */
		c = get1key();

		if (c != eolchar) {
			/* If it is a <ret>, change it to a <NL> */
			if (c == (CONTROL | 0x4D) && !quotef)
				c = CONTROL | 0x40 | '\n';
		}

		/* if they hit the line terminate, wrap it up */
		if (c == eolchar && quotef == FALSE) {
			buf[cpos++] = 0;

			/* clear the message line */
			mlwrite("");
			TTflush();

			/* if we default the buffer, return FALSE */
			if (buf[0] == 0)
				return FALSE;

			return TRUE;
		}

		/* change from command form back to character form */
		c = ectoc(c);

		if (c == ectoc(abortc) && quotef == FALSE) {
			/* Abort the input? */
			ctrlg(FALSE, 0);
			TTflush();
			return ABORT;
		} else if ((c == 0x7F || c == 0x08) && quotef == FALSE) {
			/* rubout/erase */
			if (cpos != 0) {
				outstring("\b \b");
				--ttcol;

				if (buf[--cpos] < 0x20) {
					outstring("\b \b");
					--ttcol;
				}
				if (buf[cpos] == '\n') {
					outstring("\b\b  \b\b");
					ttcol -= 2;
				}

				TTflush();
			}

		} else if (c == quotec && quotef == FALSE) {
			quotef = TRUE;
		} else {
			quotef = FALSE;
			if (cpos < nbuf - 1) {
				buf[cpos++] = c;

				if ((c < ' ') && (c != '\n')) {
					outstring("^");
					++ttcol;
					c ^= 0x40;
				}

				if (c != '\n') {
					TTputc(c);
				} else {
					outstring("<NL>");
					ttcol += 3;
				}
				++ttcol;
				TTflush();
			}
		}
	}
}

/*
 * Execute a named command even if it is not bound.
 */
int namedcmd(int f, int n)
{
	fn_t kfunc;

	/* prompt the user to type a named command */
	mlwrite(": ");

	/* and now get the function name to execute */
	kfunc = getname();
	if (kfunc == NULL) {
		mlwrite("(No such function)");
		return FALSE;
	}

	/* and then execute the command */
	return kfunc(f, n);
}

void outstring(char *s)
{
	while (*s)
		TTputc(*s++);
}
