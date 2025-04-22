#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

struct key_tab keytab[] = {
	/* Normal chars (without prefix) can be rebind, too */
	/*
	{'A', backdel},
	*/
	{0x7F, backdel},

	{CONTROL | 'A', gotobol},
	{CONTROL | 'B', backchar},
	{CONTROL | 'D', forwdel},
	{CONTROL | 'E', gotoeol},
	{CONTROL | 'F', forwchar},
	{CONTROL | 'G', ctrlg},
	{CONTROL | 'H', backdel},
	{CONTROL | 'J', indent},
	{CONTROL | 'K', killtext},
	{CONTROL | 'L', redraw},
	{CONTROL | 'M', insert_newline},
	{CONTROL | 'N', forwline},
	{CONTROL | 'O', openline},
	{CONTROL | 'P', backline},
	{CONTROL | 'Q', quote},
	/*
	{CONTROL | 'R', backsearch},
	{CONTROL | 'S', forwsearch},
	*/
	{CONTROL | 'R', risearch},
	{CONTROL | 'S', fisearch},
	{CONTROL | 'T', twiddle},
	{CONTROL | 'V', forwpage},
	{CONTROL | 'W', killregion},
	{CONTROL | 'Y', yank},
	{CONTROL | 'Z', backpage},

	{META | ' ', setmark},
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
	{META | 'P', prevbuffer},
	{META | 'U', upperword},
	{META | 'V', backpage},
	{META | 'W', copyregion},
	{META | 'X', namedcmd},
	{META | 'Z', quickexit},
	{META | 0x7F, delbword},

	{META | CONTROL | 'H', delbword},
	{META | CONTROL | 'L', reposition},
	{META | CONTROL | 'M', delgmode},
	{META | CONTROL | 'N', namebuffer},
	{META | CONTROL | 'R', qreplace},
	{META | CONTROL | 'V', scrnextdw},
	{META | CONTROL | 'Z', scrnextup},

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
	{CTLX | 'E', ctlxe},
	{CTLX | 'K', killbuffer},
	{CTLX | 'M', setemode},
	{CTLX | 'N', filename},
	{CTLX | 'O', nextwind},
	{CTLX | 'P', prevwind},
	{CTLX | 'W', resize},
	{CTLX | 'X', nextbuffer},
	{CTLX | 'Z', enlargewind},

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
	{CTLX | CONTROL | 'T', trim},
	{CTLX | CONTROL | 'U', upperregion},
	{CTLX | CONTROL | 'V', viewfile},
	{CTLX | CONTROL | 'W', filewrite},
	{CTLX | CONTROL | 'X', swapmark},
	{CTLX | CONTROL | 'Z', shrinkwind},

	/* CTLX + META + CONTROL is also working in this version */
	/*
	{CTLX | META | CONTROL | 'F', nullproc},
	{CTLX | META | CONTROL | 'G', ctrlg},
	*/

	{0, NULL}
};
