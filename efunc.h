/* efunc.h
 *
 *	Function declarations and names.
 *
 *	This file list all the C code functions used and the names to use
 *      to bind keys to them. To add functions,	declare it here in both the
 *      extern function list and the name binding table.
 *
 *	modified by Petri Kutvonen
 */

#include "utf8.h"

/* word.c */
int wrapword(int f, int n);
int backword(int f, int n);
int forwword(int f, int n);
int upperword(int f, int n);
int lowerword(int f, int n);
int capword(int f, int n);
int delfword(int f, int n);
int delbword(int f, int n);
int inword(void);

/* window.c */
int reposition(int f, int n);
int redraw(int f, int n);
int nextwind(int f, int n);
int prevwind(int f, int n);
int mvdnwind(int f, int n);
int mvupwind(int f, int n);
int onlywind(int f, int n);
int delwind(int f, int n);
int splitwind(int f, int n);
int enlargewind(int f, int n);
int shrinkwind(int f, int n);
int resize(int f, int n);
int scrnextup(int f, int n);
int scrnextdw(int f, int n);
int savewnd(int f, int n);
int restwnd(int f, int n);
int newsize(int f, int n);
int newwidth(int f, int n);
int getwpos(void);
void cknewwindow(void);
struct window *wpopup(void);  /* Pop up window creation. */

/* basic.c */
int gotobol(int f, int n);
int backchar(int f, int n);
int gotoeol(int f, int n);
int forwchar(int f, int n);
int gotoline(int f, int n);
int gotobob(int f, int n);
int gotoeob(int f, int n);
int forwline(int f, int n);
int backline(int f, int n);
int gotobop(int f, int n);
int gotoeop(int f, int n);
int forwpage(int f, int n);
int backpage(int f, int n);
int setmark(int f, int n);
int swapmark(int f, int n);

/* random.c */
int showcpos(int f, int n);
int getcline(void);
int getccol(int bflg);
int setccol(int pos);
int twiddle(int f, int n);
int quote(int f, int n);
int insert_tab(int f, int n);
int trim(int f, int n);
int openline(int f, int n);
int insert_newline(int f, int n);
int cinsert(void);
int insbrace(int n, int c);
int inspound(void);
int deblank(int f, int n);
int indent(int f, int n);
int forwdel(int f, int n);
int backdel(int f, int n);
int killtext(int f, int n);
int setemode(int f, int n);
int delmode(int f, int n);
int setgmode(int f, int n);
int delgmode(int f, int n);
int adjustmode(int kind, int global);
int clrmes(int f, int n);
int writemsg(int f, int n);
int getfence(int f, int n);
int fmatch(int ch);
int istring(int f, int n);
int ovstring(int f, int n);

/* main.c */
int (*getbind(int c))(int, int);
void edinit(char *bname);
int execute(int c, int f, int n);
int quickexit(int f, int n);
int quit(int f, int n);
int ctlxlp(int f, int n);
int ctlxrp(int f, int n);
int ctlxe(int f, int n);
int ctrlg(int f, int n);
int rdonly(void);
int resterr(void);
int nullproc(int f, int n);
int metafn(int f, int n);
int cex(int f, int n);
int unarg(int f, int n);
int cexit(int status);

/* display.c */
void vtinit(void);
void vtfree(void);
void vttidy(void);
void vtmove(int row, int col);
int upscreen(int f, int n);
int update(int force);
void updpos(void);
void upddex(void);
void updgar(void);
int updupd(int force);
void upmode(void);
void movecursor(int row, int col);
void mlerase(void);
void mlwrite(const char *fmt, ...);
void mlforce(char *s);
void mlputs(char *s);
void getscreensize(int *widthp, int *heightp);
void sizesignal(int signr);

/* region.c */
int killregion(int f, int n);
int copyregion(int f, int n);
int lowerregion(int f, int n);
int upperregion(int f, int n);
int getregion(struct region *rp);

/* posix.c */
void ttopen(void);
void ttclose(void);
int ttputc(int c);
void ttflush(void);
int ttgetc(void);
int typahead(void);

/* input.c */
int mlyesno(char *prompt);
int mlreply(char *prompt, char *buf, int nbuf);
int mlreplyt(char *prompt, char *buf, int nbuf, int eolchar);
int ectoc(int c);
int ctoec(int c);
fn_t getname(void);
int tgetc(void);
int get1key(void);
int getcmd(void);
int getstring(char *prompt, char *buf, int nbuf, int eolchar);
int namedcmd(int f, int n);
void outstring(char *s);
void ostring(char *s);

/* buffer.c */
int usebuffer(int f, int n);
int nextbuffer(int f, int n);
int swbuffer(struct buffer *bp);
int killbuffer(int f, int n);
int zotbuf(struct buffer *bp);
int namebuffer(int f, int n);
int listbuffers(int f, int n);
int makelist(int iflag);
void e_ltoa(char *buf, int width, long num);
int addline(char *text);
int anycb(void);
int bclear(struct buffer *bp);
int unmark(int f, int n);
/* Lookup a buffer by name. */
struct buffer *bfind(char *bname, int cflag, int bflag);

/* file.c */
int fileread(int f, int n);
int insfile(int f, int n);
int filefind(int f, int n);
int viewfile(int f, int n);
int getfile(char *fname, int lockfl);
int readin(char *fname, int lockfl);
void makename(char *bname, char *fname);
void unqname(char *name);
int filewrite(int f, int n);
int filesave(int f, int n);
int writeout(char *fn);
int filename(int f, int n);
int ifile(char *fname);

/* fileio.c */
int ffropen(char *fn);
int ffwopen(char *fn);
int ffclose(void);
int ffputline(char *buf, int nbuf);
int ffgetline(void);
int fexist(char *fname);

/* spawn.c */
int spawncli(int f, int n);
int bktoshell(int f, int n);
void rtfrmshell(void);
int spawn(int f, int n);
int execprg(int f, int n);
int pipecmd(int f, int n);
int filter_buffer(int f, int n);
int sys(char *cmd);
int shellprog(char *cmd);
int execprog(char *cmd);

/* search.c */
int forwsearch(int f, int n);
int forwhunt(int f, int n);
int backsearch(int f, int n);
int backhunt(int f, int n);
int scanner(const char *patrn, int direct, int beg_or_end);
int eq(unsigned char bc, unsigned char pc);
void savematch(void);
void rvstrcpy(char *rvstr, char *str);
int sreplace(int f, int n);
int qreplace(int f, int n);
int delins(int dlength, char *instr, int use_meta);
int expandp(char *srcstr, char *deststr, int maxlength);
int boundry(struct line *curline, int curoff, int dir);

/* isearch.c */
int risearch(int f, int n);
int fisearch(int f, int n);
int isearch(int f, int n);
int checknext(char chr, char *patrn, int dir);
int scanmore(char *patrn, int dir);
int match_pat(char *patrn);
int promptpattern(char *prompt);
int get_char(void);
int uneat(void);
void reeat(int c);

/* lock.c */
int lockchk(char *fname);
int lockrel(void);
int lock(char *fname);
int unlock(char *fname);
void lckerror(char *errstr);

/* pklock.c */
char *dolock(char *fname);
char *undolock(char *fname);
