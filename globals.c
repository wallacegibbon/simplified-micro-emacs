#include "estruct.h"
#include "edef.h"

int kbdm[NKBDM];		/* Keyboard Macro */

char golabel[NPAT] = "";	/* current line to go to */
int eolexist = TRUE;		/* does clear to EOL exist */
int revexist = FALSE;		/* does reverse video exist? */
int flickcode = FALSE;		/* do flicker supression? */

const char *modename[NUMMODES] = {"ASAVE", "VIEW", "EXACT", "OVER"};
int modevalue[NUMMODES] = {MDASAVE, MDVIEW, MDEXACT, MDOVER};
char modecode[NUMMODES] = "AVEO";

int gmode = MDASAVE;		/* global editor mode */
int gflags = GFREAD;		/* global control flag */
int gfcolor = 7;		/* global forgrnd color (white) */
int gbcolor = 0;		/* global backgrnd color (black) */
int gasave = 256;		/* global ASAVE size */
int gacount = 256;		/* count until next ASAVE */
int sgarbf = TRUE;		/* TRUE if screen is garbage */
int mpresf = FALSE;		/* TRUE if message in last line */
int vtrow = 0;			/* Row location of SW cursor */
int vtcol = 0;			/* Column location of SW cursor */
int ttrow = HUGE;		/* Row location of HW cursor */
int ttcol = HUGE;		/* Column location of HW cursor */
int lbound = 0;			/* leftmost column of current line being displayed */
int taboff = 0;			/* tab offset for display */

int metac = (CONTROL | '[');	/* META character */
int ctlxc = (CONTROL | 'X');	/* CONTROL-X prefix char */
int abortc = (CONTROL | 'G');	/* ABORT command char */
int enterc = (CONTROL | 'M');	/* ENTER/CR char */
int reptc = (CONTROL | 'U');	/* Universal repeat char */

int tabmask = 0x07;		/* tabulator mask */

char *cname[] = {		/* names of colors */
	"BLACK", "RED", "GREEN", "YELLOW", "BLUE", "MAGENTA", "CYAN", "WHITE"
};

struct kill *kbufp = NULL;	/* current kill buffer chunk pointer */
struct kill *kbufh = NULL;	/* kill buffer header pointer */

int kused = KBLOCK;		/* # of bytes used in kill buffer */
struct window *swindow = NULL;	/* saved window pointer */
int *kbdptr;			/* current position in keyboard buf */
int *kbdend = kbdm;		/* ptr to end of the keyboard */
int kbdmode = STOP;		/* current keyboard macro mode */
int kbdrep = 0;			/* number of repetitions */
int lastkey = 0;		/* last keystoke */
long envram = 0l;		/* # of bytes current in use by malloc */
int saveflag = 0;		/* Flags, saved with the $target var */
char *fline = NULL;		/* dynamic return line */
int flen = 0;			/* current length of fline */
int rval = 0;			/* return value of a subprocess */

int overlap = 0;		/* line overlap in forw/back page */
int scrollcount = 1;		/* number of lines to scroll */

unsigned int matchlen = 0;	/* The length of the matched string */
unsigned int mlenold = 0;
char *patmatch = NULL;		/* The string that satisfies the search */
struct line *matchline = NULL;	/* The line of the *start* of match */
int matchoff = 0;		/* The offset of the *start* of match */

struct window *curwp;		/* Current window */
struct buffer *curbp;		/* Current buffer */
struct buffer *prevbp;		/* Previous buffer */
struct window *wheadp;		/* Head of list of windows */
struct buffer *bheadp;		/* Head of list of buffers */
struct buffer *blistp;		/* Buffer for C-X C-B */
int currow;			/* Cursor row */
int curcol;			/* Cursor column */
int curgoal;			/* Goal for C-P, C-N */
int thisflag;			/* Flags, this command */
int lastflag = 0;		/* Flags, last command */

char sres[NBUFN];		/* Current screen resolution */
char pat[NPAT];			/* Search pattern */
char tap[NPAT];			/* Reversed pattern array. */
char rpat[NPAT];		/* Replacement pattern */
