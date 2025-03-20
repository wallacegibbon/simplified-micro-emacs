/*
 * Command table.
 * This table  is *roughly* in ASCII order, left to right across the
 * characters of the command. This explains the funny location of the
 * control-X commands.
 */

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

struct key_tab keytab[] = {
	{CONTROL | 'A', gotobol},
	{CONTROL | 'B', backchar},
	{CONTROL | 'D', forwdel},
	{CONTROL | 'E', gotoeol},
	{CONTROL | 'F', forwchar},
	{CONTROL | 'G', ctrlg},
	{CONTROL | 'H', backdel},
	{CONTROL | 'I', insert_tab},
	{CONTROL | 'J', indent},
	{CONTROL | 'K', killtext},
	{CONTROL | 'L', redraw},
	{CONTROL | 'M', insert_newline},
	{CONTROL | 'N', forwline},
	{CONTROL | 'O', openline},
	{CONTROL | 'P', backline},
	{CONTROL | 'Q', quote},
	{CONTROL | 'R', risearch},
	{CONTROL | 'S', fisearch},
	{CONTROL | 'T', twiddle},
	{CONTROL | 'U', unarg},
	{CONTROL | 'V', forwpage},
	{CONTROL | 'W', killregion},
	{CONTROL | 'X', cex},
	{CONTROL | 'Y', yank},
	{CONTROL | 'Z', backpage},
	{CONTROL | ']', metafn},
	{CTLX | CONTROL | 'B', listbuffers},
	{CTLX | CONTROL | 'C', quit},
	{CTLX | CONTROL | 'F', filefind},
	{CTLX | CONTROL | 'I', insfile},
	{CTLX | CONTROL | 'L', lowerregion},
	{CTLX | CONTROL | 'M', delmode},
	{CTLX | CONTROL | 'N', mvdnwind},
	{CTLX | CONTROL | 'O', deblank},
	{CTLX | CONTROL | 'P', mvupwind},
	{CTLX | CONTROL | 'R', fileread},
	{CTLX | CONTROL | 'S', filesave},
#if AEDIT
	{CTLX | CONTROL | 'T', trim},
#endif
	{CTLX | CONTROL | 'U', upperregion},
	{CTLX | CONTROL | 'V', viewfile},
	{CTLX | CONTROL | 'W', filewrite},
	{CTLX | CONTROL | 'X', swapmark},
	{CTLX | CONTROL | 'Z', shrinkwind},
	{CTLX | '!', spawn},
	{CTLX | '@', pipecmd},
	{CTLX | '#', filter_buffer},
	{CTLX | '$', execprg},
	{CTLX | '=', showcpos},
	{CTLX | '(', ctlxlp},
	{CTLX | ')', ctlxrp},
	{CTLX | '^', enlargewind},
	{CTLX | '0', delwind},
	{CTLX | '1', onlywind},
	{CTLX | '2', splitwind},
	{CTLX | 'B', usebuffer},
	{CTLX | 'C', spawncli},
#if BSD | __hpux | SVR4
	{CTLX | 'D', bktoshell},
#endif
	{CTLX | 'E', ctlxe},
	{CTLX | 'F', setfillcol},
	{CTLX | 'K', killbuffer},
	{CTLX | 'M', setemode},
	{CTLX | 'N', filename},
	{CTLX | 'O', nextwind},
	{CTLX | 'P', prevwind},
	{CTLX | 'W', resize},
	{CTLX | 'X', nextbuffer},
	{CTLX | 'Z', enlargewind},
#if CFENCE
	{META | CONTROL | 'F', getfence},
#endif
	{META | CONTROL | 'H', delbword},
	{META | CONTROL | 'L', reposition},
	{META | CONTROL | 'M', delgmode},
	{META | CONTROL | 'N', namebuffer},
	{META | CONTROL | 'R', qreplace},
	{META | CONTROL | 'S', newsize},
	{META | CONTROL | 'T', newwidth},
	{META | CONTROL | 'V', scrnextdw},
	{META | CONTROL | 'Z', scrnextup},
	{META | ' ', setmark},
	{META | '!', reposition},
	{META | '.', setmark},
	{META | '>', gotoeob},
	{META | '<', gotobob},
	{META | '~', unmark},
	{META | '%', qreplace},
	{META | 'B', backword},
	{META | 'C', capword},
	{META | 'D', delfword},
	{META | 'F', forwword},
	{META | 'G', gotoline},
	{META | 'L', lowerword},
	{META | 'M', setgmode},
	{META | 'R', sreplace},
#if BSD
	{META | 'S', bktoshell},
#endif
	{META | 'U', upperword},
	{META | 'V', backpage},
	{META | 'W', copyregion},
	{META | 'X', namedcmd},
	{META | 'Z', quickexit},
	{META | 0x7F, delbword},

#if MSDOS
	{SPEC | CONTROL | '_', forwhunt},
	{SPEC | CONTROL | 'S', backhunt},
	{SPEC | 71, gotobol},
	{SPEC | 72, backline},
	{SPEC | 73, backpage},
	{SPEC | 75, backchar},
	{SPEC | 77, forwchar},
	{SPEC | 79, gotoeol},
	{SPEC | 80, forwline},
	{SPEC | 81, forwpage},
	{SPEC | 82, insspace},
	{SPEC | 83, forwdel},
	{SPEC | 115, backword},
	{SPEC | 116, forwword},
#if PKCODE
	{SPEC | 117, gotoeob},
	{SPEC | 119, gotobob},
	{SPEC | 141, gotobop},
	{SPEC | 145, gotoeop},
	{SPEC | 146, yank},
	{SPEC | 147, killregion},
#endif /* PKCODE */
#endif /* MSDOS */

#if VT220
	{SPEC | '1', fisearch},
	{SPEC | '2', yank},
	{SPEC | '3', killregion},
	{SPEC | '4', setmark},
	{SPEC | '5', backpage},
	{SPEC | '6', forwpage},
	{SPEC | 'A', backline},
	{SPEC | 'B', forwline},
	{SPEC | 'C', forwchar},
	{SPEC | 'D', backchar},
	{SPEC | 'c', metafn},
	{SPEC | 'd', backchar},
	{SPEC | 'e', forwline},
	{SPEC | 'f', gotobob},
	{SPEC | 'i', cex},
#endif /* VT200 */

	{0x7F, backdel},

	/* special internal bindings */
	{ SPEC | META | 'W', wrapword },	/* called on word wrap */
	{ SPEC | META | 'C', nullproc },	/* every command input */
	{ SPEC | META | 'R', nullproc },	/* on file read */
	{ SPEC | META | 'X', nullproc },	/* on window change P.K. */

	{0, NULL}
};
