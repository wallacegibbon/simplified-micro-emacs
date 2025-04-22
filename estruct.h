#define MAXCOL	500
#define MAXROW	500

/* Machine/OS definitions. */

#if defined(AUTOCONF) || defined(BSD) || defined(SYSV)

/* Make an intelligent guess about the target system. */

#if defined(BSD) || defined(sun) || defined(ultrix) || (defined(vax) && defined(unix)) || defined(ultrix) || defined(__osf__)
#ifndef BSD
#define BSD 1 /* Berkeley UNIX */
#endif
#else
#define	BSD 0
#endif

#if defined(SVR4) || defined(__linux__)	/* ex. SunOS 5.3 */
#define SVR4 1
#define SYSV 1
#undef BSD
#endif

#if defined(SYSV) || defined(u3b2) || defined(_AIX) || (defined(i386) && defined(unix)) || defined(__hpux)
#define	USG 1 /* System V UNIX */
#else
#define	USG 0
#endif

#define	V7 0 /* No more. */

#else

#define V7      0		/* V7 UNIX or Coherent or BSD4.2 */
#define	BSD	0		/* UNIX BSD 4.2 and ULTRIX */
#define	USG	0		/* UNIX system V */

#endif /* AUTOCONF || BSD || SYSV */

#ifndef	AUTOCONF
#define	UNIX	0		/* a random UNIX compiler */
#else
#define	UNIX	(V7 | BSD | USG)
#endif /* AUTOCONF */

/* Debugging options */

#define	RAMSIZE	0		/* dynamic RAM memory usage tracking */
#define	RAMSHOW	0		/* auto dynamic RAM reporting */

#ifndef	AUTOCONF

/* Special keyboard definitions */

#define VT220	0		/* Use keypad escapes P.K. */
#define VT100   0		/* Handle VT100 style keypad. */

/* Terminal Output definitions */

#define ANSI    0		/* ANSI escape sequences */
#define VT52    0		/* VT52 terminal (Zenith). */
#define TERMCAP 0		/* Use TERMCAP */

#else

#define	VT220	UNIX
#define	VT100	0
#define	ANSI	0
#define	VT52	0
#define	TERMCAP	UNIX

#endif /* AUTOCONF */

/* Configuration options */

#define CVMVAS  1  /* arguments to page forward/back in pages */
#define	CLRMSG	0  /* space clears the message line with no insert */
#define	TYPEAH	1  /* type ahead causes update to be skipped */
#define	VISMAC	0  /* update display during keyboard macros */
#define ADDCR	0  /* ajout d'un CR en fin de chaque ligne (ST520) */
#define	NBRACE	1  /* new style brace matching command */
#define	REVSTA	1  /* Status line appears in reverse video */

#ifndef	AUTOCONF

#define	COLOR	1  /* color commands and windows */
#define	FILOCK	0  /* file locking under unix BSD 4.2 */

#else

#define	COLOR	0
#ifdef  SVR4
#define FILOCK  1
#else
#define	FILOCK	BSD
#endif

#endif /* AUTOCONF. */

#define	CLEAN	0  /* de-alloc memory on exit */

#ifndef	AUTOCONF
#define	XONXOFF	0  /* don't disable XON-XOFF flow control P.K. */
#else
#define	XONXOFF	UNIX
#endif /* AUTOCONF */

#define	PKCODE	1      /* include my extensions P.K., define always */
#define SCROLLCODE 1   /* scrolling code P.K. */


/* Define some ability flags. */

#if V7 | USG | BSD
#define	ENVFUNC	1
#else
#define	ENVFUNC	0
#endif

/* Emacs global flag bit definitions (for gflags). */

#define	GFREAD	1

/* Internal constants. */

#define NFILEN  256		/* # of bytes, file name */
#define NBUFN   16		/* # of bytes, buffer name */
#define NLINE   256		/* # of bytes, input line */
#define	NSTRING	128		/* # of bytes, string buffers */
#define NKBDM   256		/* # of strokes, keyboard macro */
#define NPAT    128		/* # of bytes, pattern */
#define HUGE    1000		/* Huge number */
#define	NLOCKS	100		/* max # of file locks active */
#define	NCOLORS	8		/* number of supported colors */
#define	KBLOCK	250		/* sizeof kill buffer chunks */

#define CONTROL	0x2000		/* Control flag, or'ed in */
#define META	0x4000		/* Meta flag, or'ed in */
#define CTLX	0x8000		/* ^X flag, or'ed in */

#ifdef	FALSE
#undef	FALSE
#endif
#ifdef	TRUE
#undef	TRUE
#endif

#define FALSE   0		/* False, no, bad, etc. */
#define TRUE    1		/* True, yes, good, etc. */
#define ABORT   2		/* Death, ^G, abort, etc. */
#define	FAILED	3		/* not-quite fatal false return */

#define	STOP	0		/* keyboard macro not in use */
#define	PLAY	1		/* playing */
#define	RECORD	2		/* recording */

/*
 * PTBEG, PTEND, FORWARD, and REVERSE are all toggle-able values for
 * the scan routines.
 */

#define	PTBEG	0		/* Leave the point at the beginning on search */
#define	PTEND	1		/* Leave the point at the end on search */
#define	FORWARD	0		/* forward direction */
#define REVERSE	1		/* backwards direction */

#define FIOSUC  0		/* File I/O, success. */
#define FIOFNF  1		/* File I/O, file not found. */
#define FIOEOF  2		/* File I/O, end of file. */
#define FIOERR  3		/* File I/O, error. */
#define	FIOMEM	4		/* File I/O, out of memory */
#define	FIOFUN	5		/* File I/O, eod of file/bad line */

#define CFCPCN  0x0001		/* Last command was C-P, C-N */
#define CFKILL  0x0002		/* Last command was a kill */

#define	BELL	0x07		/* BELL character */
#define	TAB	0x09		/* TAB character */
#define ESC     0x1B		/* ESC character. */

/*
 * DIFCASE represents the integer difference between upper
 * and lower case letters.  It is an xor-able value, which is
 * fortunate, since the relative positions of upper to lower
 * case letters is the opposite of ascii in ebcdic.
 */

#define	DIFCASE		0x20

#ifdef islower
#undef islower
#endif

#ifdef isupper
#undef isupper
#endif

#define islower(c)	('a' <= (c) && (c) <= 'z')
#define isupper(c)	('A' <= (c) && (c) <= 'Z')
#define isletter(c)	(islower(c) || isupper(c))
#define isvisible(c)	(((c) >= 0x20 && (c) <= 0x7E) || (c) == '\t')

/* Dynamic RAM tracking and reporting redefinitions */

#if RAMSIZE
#define	malloc	allocate
#define	free	release
#endif

#if CLEAN
#define	exit(a)	cexit(a)
#endif

/*
 * There is a window structure allocated for every active display window. The
 * windows are kept in a big list, in top to bottom screen order, with the
 * listhead at "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands to guide
 * redisplay. Although this is a bit of a compromise in terms of decoupling,
 * the full blown redisplay is just too expensive to run for every input
 * character.
 */
struct window {
	struct window *w_wndp;	/* Next window */
	struct buffer *w_bufp;	/* Buffer displayed in window */
	struct line *w_linep;	/* Top line in the window */
	struct line *w_dotp;	/* Line containing "." */
	struct line *w_markp;	/* Line containing "mark" */
	int w_doto;		/* Byte offset for "." */
	int w_marko;		/* Byte offset for "mark" */
	int w_toprow;		/* Origin 0 top row of window */
	int w_ntrows;		/* # of rows of text in window */
	char w_force;		/* If NZ, forcing row. */
	char w_flag;		/* Flags. */
#if COLOR
	char w_fcolor;		/* current forground color */
	char w_bcolor;		/* current background color */
#endif
};

#define	WFFORCE 0x01		/* Window needs forced reframe */
#define	WFMOVE  0x02		/* Movement from line to line */
#define	WFEDIT  0x04		/* Editing within a line */
#define	WFHARD  0x08		/* Better to a full display */
#define	WFMODE  0x10		/* Update mode line. */
#define	WFCOLR	0x20		/* Needs a color change */

#if SCROLLCODE
#define WFKILLS 0x40		/* something was deleted */
#define WFINS   0x80		/* something was inserted */
#endif


/*
 * Text is kept in buffers. A buffer header, described below, exists for every
 * buffer in the system. The buffers are kept in a big list, so that commands
 * that search for a buffer by name can find the buffer header. There is a
 * safe store for the dot and mark in the header, but this is only valid if
 * the buffer is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with a pointer to
 * the header line in "b_linep".
 * 	Buffers may be "Inactive" which means the files associated with them
 * have not been read in yet. These get read in at "use buffer" time.
 */
struct buffer {
        struct buffer *b_bufp;	/* Link to next struct buffer */
	struct line *b_dotp;	/* Link to "." struct line structure */
	struct line *b_markp;	/* The same as the above two, */
	struct line *b_linep;	/* Link to the header struct line */
	int b_doto;		/* Offset of "." in above struct line */
	int b_marko;		/* but for the "mark" */
	int b_mode;		/* editor mode of this buffer */
	char b_active;		/* window activated flag */
	char b_nwnd;		/* Count of windows on buffer */
	char b_flag;		/* Flags */
	char b_fname[NFILEN];	/* File name */
	char b_bname[NBUFN];	/* Buffer name */
};

#define BFINVS  0x01		/* Internal invisable buffer */
#define BFCHG   0x02		/* Changed since last write */
#define	BFTRUNC	0x04		/* buffer was truncated when read */

/* mode flags */
#define	MDVIEW	0x0001		/* read-only buffer */
#define	MDEXACT	0x0002		/* Exact matching for searches */
#define	MDOVER	0x0004		/* overwrite mode */
#define	MDASAVE	0x0008		/* auto-save mode */

struct region {
	struct line *r_linep;	/* Origin struct line address. */
	int r_offset;		/* Origin struct line offset. */
	long r_size;		/* Length in characters. */
};

struct terminal {
	short t_mrow;		/* max number of rows allowable */
	short t_nrow;		/* current number of rows used */
	short t_mcol;		/* max Number of columns. */
	short t_ncol;		/* current Number of columns. */
	short t_margin;		/* min margin for extended lines */
	short t_scrsiz;		/* size of scroll region " */
	int t_pause;		/* # times thru update to pause */
	void (*t_open)(void);	/* Open terminal at the start. */
	void (*t_close)(void);	/* Close terminal at end. */
	void (*t_kopen)(void);	/* Open keyboard */
	void (*t_kclose)(void);	/* close keyboard */
	int (*t_getchar)(void);	/* Get character from keyboard. */
	int (*t_putchar)(int);	/* Put character to display. */
	void (*t_flush) (void);	/* Flush output buffers. */
	void (*t_move)(int, int);/* Move the cursor, origin 0. */
	void (*t_eeol)(void);	/* Erase to end of line. */
	void (*t_eeop)(void);	/* Erase to end of page. */
	void (*t_beep)(void);	/* Beep. */
	void (*t_rev)(int);	/* set reverse video state */
	int (*t_rez)(char *);	/* change screen resolution */
#if COLOR
	void (*t_setfor)(int);	/* set forground color */
	void (*t_setback)(int);	/* set background color */
#endif
#if SCROLLCODE
	void (*t_scroll)(int, int, int);/* scroll a region of the screen */
#endif
};

#define	TTopen		(*term.t_open)
#define	TTclose		(*term.t_close)
#define	TTkopen		(*term.t_kopen)
#define	TTkclose	(*term.t_kclose)
#define	TTgetc		(*term.t_getchar)
#define	TTputc		(*term.t_putchar)
#define	TTflush		(*term.t_flush)
#define	TTmove		(*term.t_move)
#define	TTeeol		(*term.t_eeol)
#define	TTeeop		(*term.t_eeop)
#define	TTbeep		(*term.t_beep)
#define	TTrev		(*term.t_rev)
#define	TTrez		(*term.t_rez)
#if COLOR
#define	TTforg		(*term.t_setfor)
#define	TTbacg		(*term.t_setback)
#endif

/* Structure for the table of initial key bindings. */
struct key_tab {
	int k_code;		 /* Key code */
	int (*k_fp)(int, int);	 /* Routine to handle it */
};

/* Structure for the name binding table. */
struct name_bind {
	const char *n_name;	 /* name of function key */
	int (*n_func)(int, int); /* function name is bound to */
};

/*
 * The editor holds deleted text chunks in the struct kill buffer. The
 * kill buffer is logically a stream of ascii characters, however
 * due to its unpredicatable size, it gets implemented as a linked
 * list of chunks. (The d_ prefix is for "deleted" text, as k_
 * was taken up by the keycode structure).
 */
struct kill {
	struct kill *d_next;   /* Link to next chunk, NULL if last. */
	char d_chunk[KBLOCK];  /* Deleted text. */
};
