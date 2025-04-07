/* input.c
 *
 *	Various input routines
 *
 *	written by Daniel Lawrence 5/9/86
 *	modified by Petri Kutvonen
 */

#include <stdio.h>
#include <unistd.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "wrapper.h"

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
 *	collapse the CONTROL and SPEC flags back into an ascii code
 */
int ectoc(int c)
{
	if (c & CONTROL)
		c = ~CONTROL & (c - '@');
	if (c & SPEC)
		c = c & 255;
	return c;
}

/*
 * ctoec:
 *	character to extended character
 *	pull out the CONTROL and SPEC prefixes (if possible)
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
	struct name_bind *ffp;	/* pointer to entry in name binding table */

	/* scan through the table, returning any match */
	ffp = &names[0];
	while (ffp->n_func != NULL) {
		if (strcmp(fname, ffp->n_name) == 0)
			return ffp->n_func;
		++ffp;
	}
	return NULL;
}

/*
 * get a command name from the command line. Command completion means
 * that pressing a <SPACE> will attempt to complete an unfinished command
 * name if it is unique.
 */
fn_t getname(void)
{
	int cpos;	/* current column on screen output */
	int c;
	char buf[NSTRING];	/* buffer to hold tentative command name */

	/* starting at the beginning of the string buffer */
	cpos = 0;

	/* build a name string from the keyboard */
	while (TRUE) {
		c = tgetc();

		/* if we are at the end, just match it */
		if (c == 0x0d) {
			buf[cpos] = 0;

			/* and match it off */
			return fncmatch(&buf[0]);

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
	int c;			/* fetched character */

	/* if we are playing a keyboard macro back, */
	if (kbdmode == PLAY) {

		/* if there is some left... */
		if (kbdptr < kbdend)
			return (int) *kbdptr++;

		/* at the end of last repitition? */
		if (--kbdrep < 1) {
			kbdmode = STOP;
#if VISMAC == 0
			/* force a screen update after all is done */
			update(FALSE);
#endif
		} else {

			/* reset the macro to the begining for the next rep */
			kbdptr = &kbdm[0];
			return (int) *kbdptr++;
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

/* GET1KEY:	Get one keystroke. The only prefixs legal here
			are the SPEC and CONTROL prefixes.
 */

int get1key(void)
{
	return ctoec(tgetc());
}

/* GETCMD:	Get a command from the keyboard. Process all applicable
		prefix keys
 */
int getcmd(void)
{
	int c;			/* fetched keystroke */
#if VT220
	int d;			/* second character P.K. */
	int cmask = 0;
#endif
	/* get initial character */
	c = get1key();

#if VT220
proc_metac:
#endif
	if (c == 128+27)		/* CSI */
		goto handle_CSI;
	/* process META prefix */
	if (c == (CONTROL | '[')) {
		c = get1key();
#if VT220
		if (c == '[' || c == 'O') {	/* CSI P.K. */
handle_CSI:
			c = get1key();
			if (c >= 'A' && c <= 'D')
				return SPEC | c | cmask;
			if (c >= 'E' && c <= 'z' && c != 'i' && c != 'c')
				return SPEC | c | cmask;
			d = get1key();
			if (d == '~')	/* ESC [ n ~   P.K. */
				return SPEC | c | cmask;
			switch (c) {	/* ESC [ n n ~ P.K. */
			case '1':
				c = d + 32;
				break;
			case '2':
				c = d + 48;
				break;
			case '3':
				c = d + 64;
				break;
			default:
				c = '?';
				break;
			}
			if (d != '~')	/* eat tilde P.K. */
				get1key();
			if (c == 'i') {	/* DO key    P.K. */
				c = ctlxc;
				goto proc_ctlxc;
			} else if (c == 'c')	/* ESC key   P.K. */
				c = get1key();
			else
				return SPEC | c | cmask;
		}
#endif
#if VT220
		if (c == (CONTROL | '[')) {
			cmask = META;
			goto proc_metac;
		}
#endif
		if (islower(c))	/* Force to upper */
			c ^= DIFCASE;
		c = ctoec(c);
		return META | c;
	}
#if PKCODE
	else if (c == metac) {
		c = get1key();
#if VT220
		if (c == (CONTROL | '[')) {
			cmask = META;
			goto proc_metac;
		}
#endif
		if (islower(c))	/* Force to upper */
			c ^= DIFCASE;
		c = ctoec(c);
		return META | c;
	}
#endif


#if VT220
proc_ctlxc:
#endif
	/* process CTLX prefix */
	if (c == ctlxc) {
		c = get1key();
#if VT220
		if (c == (CONTROL | '[')) {
			cmask = CTLX;
			goto proc_metac;
		}
#endif
		if (c >= 'a' && c <= 'z')
			c -= 0x20;
		else
			c = ctoec(c);
		return CTLX | c;
	}

	/* otherwise, just return it */
	return c;
}

/* A more generalized prompt/reply function allowing the caller
	to specify the proper terminator. If the terminator is not
	a return ('\n') it will echo as "<NL>"
 */
int getstring(char *prompt, char *buf, int nbuf, int eolchar)
{
	int cpos;	/* current character position in string */
	int c;
	int quotef;	/* are we quoting the next char? */

	cpos = 0;
	quotef = FALSE;

	/* prompt the user for the input string */
	mlwrite(prompt);

	for (;;) {
		/* get a character from the user */
		c = get1key();

		if (c != eolchar) {
			/* If it is a <ret>, change it to a <NL> */
#if PKCODE
			if (c == (CONTROL | 0x4d) && !quotef)
#else
			if (c == (CONTROL | 0x4d))
#endif
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

		} else if ((c == quotec || c == 0x16) && quotef == FALSE) {
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
					if (disinp)
						TTputc(c);
				} else {	/* put out <NL> for <ret> */
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
	fn_t kfunc;	/* ptr to the requexted function to bind to */

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

/*
 * output a string of characters
 */
void outstring(char *s)
{
	if (disinp) {
		while (*s)
			TTputc(*s++);
	}
}

/*
 * output a string of output characters
 */
void ostring(char *s)
{
	if (discmd) {
		while (*s)
			TTputc(*s++);
	}
}
