#ifndef __LINE_H
#define __LINE_H

#include "utf8.h"

struct line {
	struct line *l_fp;	/* Link to the next line */
	struct line *l_bp;	/* Link to the previous line */
	int l_size;		/* Allocated size */
	int l_used;		/* Used size */
	char l_text[1];		/* A bunch of characters. */
};

#define lforw(lp)       ((lp)->l_fp)
#define lback(lp)       ((lp)->l_bp)
#define lgetc(lp, n)    ((lp)->l_text[(n)] & 0xFF)
#define lputc(lp, n, c) ((lp)->l_text[(n)] = (c))
#define llength(lp)     ((lp)->l_used)

void lfree(struct line *lp);
void lchange(int flag);
int linstr(char *instr);
int linsert(int n, int c);
int lnewline(void);
int ldelete(long n, int kflag);
int ldelchar(long n, int kflag);
int lgetchar(unicode_t *);
int ldelnewline(void);
void kdelete(void);
int kinsert(int c);
int yank(int f, int n);
struct line *lalloc(int);

#endif
