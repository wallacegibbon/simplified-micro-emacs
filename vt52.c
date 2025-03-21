/* vt52.c
 *
 * The routines in this file
 * provide support for VT52 style terminals
 * over a serial line. The serial I/O services are
 * provided by routines in "termio.c". It compiles
 * into nothing if not a VT52 style device. The
 * bell on the VT52 is terrible, so the "beep"
 * routine is conditionalized on defining BEL.
 *
 *	modified by Petri Kutvonen
 */

#define	termdef	1		/* don't define "term" external */

#include <stdio.h>
#include "estruct.h"
#include "edef.h"

#if VT52

#define NROW    24		/* Screen size. */
#define NCOL    80		/* Edit if you want to. */
#define	MARGIN	8		/* size of minimim margin and */
#define	SCRSIZ	64		/* scroll size for extended lines */
#define	NPAUSE	100		/* # times thru update to pause */
#define BIAS    0x20		/* Origin 0 coordinate bias. */
#define ESC     0x1B		/* ESC character. */
#define BEL     0x07		/* ascii bell character */

void ttopen(void);
void ttclose(void);
void vt52open(void);
void vt52kopen(void);
void vt52kclose(void);
int ttgetc(void);
int ttputc(void);
void ttflush(void);
void vt52move(int, int);
void vt52eeol(void);
void vt52eeop(void);
void vt52beep(void);
void vt52rev(int);
int vt52cres(int, int);

#if COLOR
void vt52fcol(int);
void vt52bcol(int);
#endif

/*
 * Dispatch table.
 * All the hard fields just point into the terminal I/O code.
 */
struct terminal term = {
	NROW - 1,
	NROW - 1,
	NCOL,
	NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	&vt52open,
	&ttclose,
	&vt52kopen,
	&vt52kclose,
	&ttgetc,
	&ttputc,
	&ttflush,
	&vt52move,
	&vt52eeol,
	&vt52eeop,
	&vt52beep,
	&vt52rev,
	&vt52cres
#if COLOR
	,
	&vt52fcol,
	&vt52bcol
#endif
#if SCROLLCODE
	,
	NULL
#endif
};

void vt52move(int row, int col)
{
	ttputc(ESC);
	ttputc('Y');
	ttputc(row + BIAS);
	ttputc(col + BIAS);
}

void vt52eeol(void)
{
	ttputc(ESC);
	ttputc('K');
}

void vt52eeop(void)
{
	ttputc(ESC);
	ttputc('J');
}

/* TRUE = reverse video, FALSE = normal video */
void vt52rev(int status)
{
	/* can't do this here, so we won't */
}

/* change screen resolution - (not here though) */
int vt52cres(char *res)
{
	return TRUE;
}

#if COLOR
/* set the forground color [NOT IMPLIMENTED] */
void vt52fcol(void)
{
}

/* set the background color [NOT IMPLIMENTED] */
void vt52bcol(int color)
{
}
#endif

void vt52beep(void)
{
#ifdef  BEL
	ttputc(BEL);
	ttflush();
#endif
}

void vt52open(void)
{
#if V7 | BSD
	char *cp;
	char *getenv();

	if ((cp = getenv("TERM")) == NULL) {
		puts("Shell variable TERM not defined!");
		exit(1);
	}
	if (strcmp(cp, "vt52") != 0 && strcmp(cp, "z19") != 0) {
		puts("Terminal type not 'vt52'or 'z19' !");
		exit(1);
	}
#endif
	ttopen();
}

void vt52kopen(void)
{
}

void vt52kclose(void)
{
}

#endif
