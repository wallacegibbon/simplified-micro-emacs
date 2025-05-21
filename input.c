#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "wrapper.h"

/*
 * CAUTION: Prefixed chars (e.g. `CTL | 'A'`) may be stored in this variable,
 * which should be okay since functions like `ctoec` will keep it unchanged.
 */
int reeat_char = -1;

void tputs(char *s);

/* Get a key from the terminal driver, resolve any keyboard macro action */
int tgetc(void)
{
	int c;

	if ((c = reeat_char) != -1) {
		reeat_char = -1;
		return c;
	}

	/* if we are playing a keyboard macro back, */
	if (kbdmode == PLAY) {
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

	c = TTgetc();
	lastkey = c;

	if (kbdmode == RECORD) {
		*kbdptr++ = c;
		kbdend = kbdptr;

		/* don't overrun the buffer */
		if (kbdptr == &kbdm[NKBDM - 1]) {
			kbdmode = STOP;
			TTbeep();
		}
	}

	return c;
}

/* Get one keystroke.  The only prefixs legal here are the CTL prefixes. */
int get1key(void)
{
	return ctoec(tgetc());
}

#define NULLPROC_KEYS	(CTLX | META | CTL | 'Z')

static int transform_csi_1(int ch)
{
	switch (ch) {
	case 'A':	return CTL | 'P';
	case 'B':	return CTL | 'N';
	case 'C':	return CTL | 'F';
	case 'D':	return CTL | 'B';
	default:	return NULLPROC_KEYS;
	}
}

/* Get a command from the keyboard.  Process all applicable prefix keys. */
int getcmd(void)
{
	int c = get1key();
	int cmask = 0;

	if (c == 128 + 27)
		goto handle_CSI;

	/* process META prefix */
	if (c == METAC) {
#if VT220
proc_metac:
#endif
		c = get1key();
#if VT220
		if (c == '[' || c == 'O') {
		/* if CSI is not handled, events like scrolling won't work */
handle_CSI:
			c = get1key();
			if (c >= 'A' && c <= 'z') {
				return cmask | transform_csi_1(c);
			} else {
				/* There are many other CSI related keys */
				get1key();
				return NULLPROC_KEYS;
			}
		} else if (c == METAC) {
			cmask |= META;
			goto proc_metac;
		} else if (c == CTLXC) {
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
	if (c == CTLXC) {
#if VT220
proc_ctlxc:
#endif
		cmask |= CTLX;
		c = get1key();
#if VT220
		if (c == METAC) {
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
 * A more generalized prompt/reply function allowing the caller to specify
 * the proper terminator.
 */
int getstring(char *prompt, char *buf, int nbuf, int eolchar)
{
	int quotef = FALSE, cpos = 0, c, expc;

	mlwrite(prompt);

	for (;;) {
		c = ectoc(expc = get1key());

		/* if they hit the line terminate, wrap it up */
		if (expc == eolchar && quotef == FALSE) {
			buf[cpos++] = 0;
			mlwrite("");
			TTflush();

			/* if we default the buffer, return FALSE */
			if (buf[0] == 0)
				return FALSE;

			return TRUE;
		}

		if (expc == ABORTC && quotef == FALSE) {
			ctrlg(FALSE, 0);
			TTflush();
			return ABORT;

		} else if ((c == 0x7F || c == '\b') && quotef == FALSE) {
			if (cpos != 0) {
				tputs("\b \b");
				--ttcol;

				if (buf[--cpos] < 0x20) {
					tputs("\b \b");
					--ttcol;
				}
				if (buf[cpos] == '\n') {
					tputs("\b\b  \b\b");
					ttcol -= 2;
				}

				TTflush();
			}

		} else if (expc == QUOTEC && quotef == FALSE) {
			quotef = TRUE;

		} else {
			quotef = FALSE;
			if (cpos < nbuf - 1) {
				buf[cpos++] = c;
				ttcol += put_c(c, TTputc);
				TTflush();
			}
		}
	}
}

int (*getfn_byname(char *fname))(int, int)
{
	struct name_bind *ffp;

	for (ffp = names; ffp->n_func != NULL; ++ffp) {
		if (strcmp(fname, ffp->n_name) == 0)
			return ffp->n_func;
	}
	return NULL;
}

/* Execute a named command even if it is not bound. */
int namedcmd(int f, int n)
{
	char buf[NSTRING];
	fn_t fn;

	if (getstring(": ", buf, NSTRING, ENTERC) != TRUE)
		return FALSE;

	fn = getfn_byname(buf);
	if (fn == NULL) {
		mlwrite("(No such function)");
		return FALSE;
	}

	return fn(f, n);
}

int mlreply(char *prompt, char *buf, int nbuf)
{
	return getstring(prompt, buf, nbuf, ENTERC);
}

int mlyesno(char *prompt)
{
	char buf[64 /* prompt */ + 8 /* " (y/n)? " */ + 1];

	for (;;) {
		strncpy(buf, prompt, 64);
		strcat(buf, " (y/n)? ");
		mlwrite(buf);

		switch (ctoec(tgetc())) {
		case 'Y': case 'y':	return TRUE;
		case 'N': case 'n':	return FALSE;
		case ABORTC:		return ABORT;
		default:		/*ignore*/
		}
	}
}

int ectoc(int c)
{
	if (c & CTL)
		c = ~CTL & (c - '@');
	return c;
}

int ctoec(int c)
{
	if (c >= 0x00 && c <= 0x1F)
		c = CTL | (c + '@');
	return c;
}

void tputs(char *s)
{
	if (s != NULL) {
		while (*s)
			TTputc(*s++);
	}
}
