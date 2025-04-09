#ifndef __EDEF_H
#define __EDEF_H

#include <stdlib.h>
#include <string.h>

/* Some global fuction declarations. */
typedef int (*fn_t)(int, int);

/* Initialized global external declarations. */

extern int kbdm[];		/* Holds keyboard macro data */

extern char pat[];		/* Search pattern */
extern char rpat[];		/* Replacement pattern */

extern char golabel[];		/* current line to go to */

extern int eolexist;		/* does clear to EOL exist? */
extern int revexist;		/* does reverse video exist? */

#define	NUMMODES	5
extern const char *modename[NUMMODES];	/* text names of modes */
extern const int modevalue[NUMMODES];	/* value of modes */
extern char modecode[NUMMODES];	/* letters to represent modes */

extern struct key_tab keytab[];	/* key bind to functions table */
extern struct name_bind names[];/* name to function table */
extern struct key_tab keytab[];	/* key to function table */

extern int gmode;		/* global editor mode */
extern int gflags;		/* global control flag */
extern int gfcolor;		/* global forgrnd color (white) */
extern int gbcolor;		/* global backgrnd color (black) */
extern int gasave;		/* global ASAVE size */
extern int gacount;		/* count until next ASAVE */
extern int sgarbf;		/* State of screen unknown */
extern int mpresf;		/* Stuff in message line */
extern int discmd;		/* display command flag */
extern int disinp;		/* display input characters */
extern int vtrow;		/* Row location of SW cursor */
extern int vtcol;		/* Column location of SW cursor */
extern int ttrow;		/* Row location of HW cursor */
extern int ttcol;		/* Column location of HW cursor */
extern int lbound;		/* leftmost column of current line displayed */
extern int taboff;		/* tab offset for display */

extern int metac;		/* current meta character */
extern int enterc;		/* current enter character */
extern int ctlxc;		/* current control X prefix char */
extern int reptc;		/* current universal repeat char */
extern int abortc;		/* current abort command char */

extern int quotec;		/* quote char during mlreply() */

extern int tabmask;

extern char *cname[];		/* names of colors */

extern struct kill *kbufp;	/* current kill buffer chunk pointer */
extern struct kill *kbufh;	/* kill buffer header pointer */

extern int kused;		/* # of bytes used in KB */

extern struct window *swindow;	/* saved window pointer */

extern int *kbdptr;		/* current position in keyboard buf */
extern int *kbdend;		/* ptr to end of the keyboard */
extern int kbdmode;		/* current keyboard macro mode */
extern int kbdrep;		/* number of repetitions */

extern int restflag;		/* restricted use? */
extern int lastkey;		/* last keystoke */
extern long envram;		/* # of bytes current in use by malloc */
extern int saveflag;		/* Flags, saved with the $target var */
extern char *fline;		/* dynamic return line */
extern int flen;		/* current length of fline */
extern int rval;		/* return value of a subprocess */

extern int overlap;		/* line overlap in forw/back page */
extern int scrollcount;		/* number of lines to scroll */

/* Uninitialized global external declarations. */

extern int currow;		/* Cursor row */
extern int curcol;		/* Cursor column */
extern int thisflag;		/* Flags, this command */
extern int lastflag;		/* Flags, last command */
extern int curgoal;		/* Goal for C-P, C-N */
extern struct window *curwp;	/* Current window */
extern struct buffer *curbp;	/* Current buffer */
extern struct buffer *prevbp;	/* Previous buffer */
extern struct window *wheadp;	/* Head of list of windows */
extern struct buffer *bheadp;	/* Head of list of buffers */
extern struct buffer *blistp;	/* Buffer for C-X C-B */

extern char sres[NBUFN];	/* Current screen resolution. */
extern char pat[];		/* Search pattern. */
extern char tap[];		/* Reversed pattern array. */
extern char rpat[];		/* Replacement pattern. */

extern unsigned int matchlen;
extern unsigned int mlenold;
extern char *patmatch;
extern struct line *matchline;
extern int matchoff;

/* Terminal table defined only in term.c */
extern struct terminal term;

#endif
