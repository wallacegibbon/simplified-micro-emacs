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

#define termdef	1		/* don't define "term" external */

#include "estruct.h"
#include "edef.h"

#if VT52

#define NROW    24		/* Screen size. */
#define NCOL    80		/* Edit if you want to. */
#define MARGIN	8		/* size of minimim margin and */
#define SCRSIZ	64		/* scroll size for extended lines */
#define NPAUSE	100		/* # times thru update to pause */
#define BIAS    0x20		/* Origin 0 coordinate bias. */

static void ttopen(void);
static void ttclose(void);
static void vt52open(void);
static void vt52kopen(void);
static void vt52kclose(void);
static int ttgetc(void);
static int ttputc(void);
static void vt52move(int, int);
static void vt52eeol(void);
static void vt52eeop(void);
static void vt52beep(void);
static void vt52rev(int);
static int vt52cres(int, int);

#if COLOR
static void vt52fcol(int);
static void vt52bcol(int);
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
	vt52open,
	ttclose,
	vt52kopen,
	vt52kclose,
	ttgetc,
	ttputc,
	ttflush,
	vt52move,
	vt52eeol,
	vt52eeop,
	vt52beep,
	vt52rev,
	vt52cres
#if COLOR
	,
	vt52fcol,
	vt52bcol
#endif
#if SCROLLCODE
	,
	NULL
#endif
};

static void vt52move(int row, int col)
{
	ttputc(ESC);
	ttputc('Y');
	ttputc(row + BIAS);
	ttputc(col + BIAS);
}

static void vt52eeol(void)
{
	ttputc(ESC);
	ttputc('K');
}

static void vt52eeop(void)
{
	ttputc(ESC);
	ttputc('J');
}

/* TRUE = reverse video, FALSE = normal video */
static void vt52rev(int status)
{
	/* can't do this here, so we won't */
}

/* change screen resolution - (not here though) */
static int vt52cres(char *res)
{
	return TRUE;
}

#if COLOR
/* set the forground color [NOT IMPLIMENTED] */
static void vt52fcol(void)
{
}

/* set the background color [NOT IMPLIMENTED] */
static void vt52bcol(int color)
{
}
#endif

static void vt52beep(void)
{
	ttputc(BELL);
	ttflush();
}

static void vt52open(void)
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

static void vt52kopen(void)
{
}

static void vt52kclose(void)
{
}

#endif
