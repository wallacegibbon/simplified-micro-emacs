/* ANSI.C
 *
 * The routines in this file provide support for ANSI style terminals
 * over a serial line. The serial I/O services are provided by routines in
 * "termio.c". It compiles into nothing if not an ANSI device.
 *
 *	modified by Petri Kutvonen
 */

#define	termdef	1		/* don't define "term" external */

#include <stdio.h>
#include "estruct.h"
#include "edef.h"

#if ANSI

#define NROW    25		/* Screen size. */
#define NCOL    80		/* Edit if you want to. */

#if PKCODE
#define	MROW	64
#endif
#define	NPAUSE	100		/* # times thru update to pause */
#define	MARGIN	8		/* size of minimim margin and */
#define	SCRSIZ	64		/* scroll size for extended lines */
#define BEL     0x07		/* BEL character. */
#define ESC     0x1B		/* ESC character. */

void ttopen(void);
void ttclose(void);
void ansiopen(void);
void ansiclose(void);
void ansikopen(void);
void ansikclose(void);
int ttgetc(void);
int ttputc(void);
void ttflush(void);
void ansimove(int, int);
void ansieeol(void);
void ansieeop(void);
void ansibeep(void);
void ansirev(int);
void ansicres(char *);

#if COLOR
void ansifcol(int);
void ansibcol(int);

int cfcolor = -1;		/* current forground color */
int cbcolor = -1;		/* current background color */

#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
struct terminal term = {
#if PKCODE
	MROW - 1,
#else
	NROW - 1,
#endif
	NROW - 1,
	NCOL,
	NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	ansiopen,
	ansiclose,
	ansikopen,
	ansikclose,
	ttgetc,
	ttputc,
	ttflush,
	ansimove,
	ansieeol,
	ansieeop,
	ansibeep,
	ansirev,
	ansicres
#if COLOR
	,
	ansifcol,
	ansibcol
#endif
#if SCROLLCODE
	,
	NULL
#endif
};

#if COLOR
void ansifcol(int color)
{
	if (color == cfcolor)
		return;
	ttputc(ESC);
	ttputc('[');
	ansiparm(color + 30);
	ttputc('m');
	cfcolor = color;
}

/* Set the current background color.
 * color: color to set.
 */
void ansibcol(int color)
{
	if (color == cbcolor)
		return;
	ttputc(ESC);
	ttputc('[');
	ansiparm(color + 40);
	ttputc('m');
	cbcolor = color;
}
#endif

void ansimove(int row, int col)
{
	ttputc(ESC);
	ttputc('[');
	ansiparm(row + 1);
	ttputc(';');
	ansiparm(col + 1);
	ttputc('H');
}

void ansieeol(void)
{
	ttputc(ESC);
	ttputc('[');
	ttputc('K');
}

void ansieeop(void)
{
#if COLOR
	ansifcol(gfcolor);
	ansibcol(gbcolor);
#endif
	ttputc(ESC);
	ttputc('[');
	ttputc('J');
}

/* Change reverse video state.
 * state: TRUE = reverse, FALSE = normal
 */
void ansirev(int state)
{
#if COLOR
	int ftmp, btmp;		/* temporaries for colors */
#endif

	ttputc(ESC);
	ttputc('[');
	ttputc(state ? '7' : '0');
	ttputc('m');
#if COLOR
	if (state == FALSE) {
		ftmp = cfcolor;
		btmp = cbcolor;
		cfcolor = -1;
		cbcolor = -1;
		ansifcol(ftmp);
		ansibcol(btmp);
	}
#endif
}

/* Change screen resolution. */
int ansicres(char *res)
{
	return TRUE;
}

void ansibeep(void)
{
	ttputc(BEL);
	ttflush();
}

void ansiparm(int n)
{
	int q, r;

	q = n / 10;
	if (q != 0) {
		r = q / 10;
		if (r != 0) {
			ttputc((r % 10) + '0');
		}
		ttputc((q % 10) + '0');
	}
	ttputc((n % 10) + '0');
}

void ansiopen(void)
{
#if V7 | USG | BSD
	char *cp;

	if ((cp = getenv("TERM")) == NULL) {
		puts("Shell variable TERM not defined!");
		exit(1);
	}
	if (strcmp(cp, "vt100") != 0) {
		puts("Terminal type not 'vt100'!");
		exit(1);
	}
#endif
	strcpy(sres, "NORMAL");
	revexist = TRUE;
	ttopen();
}

void ansiclose(void)
{
#if COLOR
	ansifcol(7);
	ansibcol(0);
#endif
	ttclose();
}

/* Open the keyboard (a noop here). */
void ansikopen(void)
{
}

/* Close the keyboard (a noop here). */
void ansikclose(void)
{
}

#endif
