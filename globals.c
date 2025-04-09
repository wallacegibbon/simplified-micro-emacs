#include "estruct.h"
#include "edef.h"

int kbdm[NKBDM];		/* Macro */

char golabel[NPAT] = "";	/* current line to go to */
int eolexist = TRUE;		/* does clear to EOL exist */
int revexist = FALSE;		/* does reverse video exist? */
int flickcode = FALSE;		/* do flicker supression? */

const int modevalue[] = {MDCMOD, MDASAVE, MDVIEW, MDEXACT, MDOVER};

const char *modename[] = {"CMODE", "ASAVE", "VIEW", "EXACT", "OVER"};

char modecode[] = "CAVEO";	/* letters to represent modes */

int gmode = MDASAVE;		/* global editor mode */
int gflags = GFREAD;		/* global control flag */
int gfcolor = 7;		/* global forgrnd color (white) */
int gbcolor = 0;		/* global backgrnd color (black) */
int gasave = 256;		/* global ASAVE size */
int gacount = 256;		/* count until next ASAVE */
int sgarbf = TRUE;		/* TRUE if screen is garbage */
int mpresf = FALSE;		/* TRUE if message in last line */
int discmd = TRUE;		/* display command flag */
int disinp = TRUE;		/* display input characters */
int vtrow = 0;			/* Row location of SW cursor */
int vtcol = 0;			/* Column location of SW cursor */
int ttrow = HUGE;		/* Row location of HW cursor */
int ttcol = HUGE;		/* Column location of HW cursor */
int lbound = 0;			/* leftmost column of current line being displayed */
int taboff = 0;			/* tab offset for display */

int metac = CONTROL | '[';	/* current meta character */
int ctlxc = CONTROL | 'X';	/* current control X prefix char */
int reptc = CONTROL | 'U';	/* current universal repeat char */
int abortc = CONTROL | 'G';	/* current abort command char */
int enterc = CONTROL | 'M';	/* current enter/CR char */

int quotec = 0x11;		/* quote char during mlreply() */
int tabmask = 0x07;		/* tabulator mask */

char *cname[] = {		/* names of colors */
	"BLACK", "RED", "GREEN", "YELLOW", "BLUE", "MAGENTA", "CYAN", "WHITE"
};

struct kill *kbufp = NULL;	/* current kill buffer chunk pointer */
struct kill *kbufh = NULL;	/* kill buffer header pointer */

int kused = KBLOCK;		/* # of bytes used in kill buffer */
struct window *swindow = NULL;	/* saved window pointer */
int *kbdptr;			/* current position in keyboard buf */
int *kbdend = &kbdm[0];		/* ptr to end of the keyboard */
int kbdmode = STOP;		/* current keyboard macro mode */
int kbdrep = 0;			/* number of repetitions */
int restflag = FALSE;		/* restricted use? */
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


int currow;			/* Cursor row */
int curcol;			/* Cursor column */
int thisflag;			/* Flags, this command */
int lastflag;			/* Flags, last command */
int curgoal;			/* Goal for C-P, C-N */
struct window *curwp;		/* Current window */
struct buffer *curbp;		/* Current buffer */
struct buffer *prevbp;		/* Previous buffer */
struct window *wheadp;		/* Head of list of windows */
struct buffer *bheadp;		/* Head of list of buffers */
struct buffer *blistp;		/* Buffer for C-X C-B */

char sres[NBUFN];		/* Current screen resolution */
char pat[NPAT];			/* Search pattern */
char tap[NPAT];			/* Reversed pattern array. */
char rpat[NPAT];		/* Replacement pattern */
