/* Name to function binding table.
 *
 * This table gives the names of all the bindable functions
 * end their C function address. These are used for the bind-to-key
 * function.
*/

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

struct name_bind names[] = {
	{"abort-command", ctrlg},
	{"add-mode", setemode},
	{"add-global-mode", setgmode},
	{"backward-character", backchar},
	{"begin-macro", ctlxlp},
	{"beginning-of-file", gotobob},
	{"beginning-of-line", gotobol},
	{"buffer-position", showcpos},
	{"case-region-lower", lowerregion},
	{"case-region-upper", upperregion},
	{"case-word-capitalize", capword},
	{"case-word-lower", lowerword},
	{"case-word-upper", upperword},
	{"change-file-name", filename},
	{"clear-and-redraw", redraw},
	{"clear-message-line", clrmes},
	{"copy-region", copyregion},
	{"ctlx-prefix", cex},
	{"delete-blank-lines", deblank},
	{"delete-buffer", killbuffer},
	{"delete-mode", delmode},
	{"delete-global-mode", delgmode},
	{"delete-next-character", forwdel},
	{"delete-next-word", delfword},
	{"delete-other-windows", onlywind},
	{"delete-previous-character", backdel},
	{"delete-previous-word", delbword},
	{"delete-window", delwind},
	{"end-macro", ctlxrp},
	{"end-of-file", gotoeob},
	{"end-of-line", gotoeol},
	{"exchange-point-and-mark", swapmark},
	{"execute-macro", ctlxe},
	{"execute-program", execprg},
	{"exit-emacs", quit},
	{"filter-buffer", filter_buffer},
	{"find-file", filefind},
	{"forward-character", forwchar},
	{"goto-line", gotoline},
#if CFENCE
	{"goto-matching-fence", getfence},
#endif
	{"grow-window", enlargewind},
	{"handle-tab", insert_tab},
	{"hunt-forward", forwhunt},
	{"hunt-backward", backhunt},
	{"i-shell", spawncli},
	{"incremental-search", fisearch},
	{"insert-file", insfile},
	{"insert-space", insspace},
	{"insert-string", istring},
	{"kill-region", killregion},
	{"kill-to-end-of-line", killtext},
	{"list-buffers", listbuffers},
	{"meta-prefix", metafn},
	{"move-window-down", mvdnwind},
	{"move-window-up", mvupwind},
	{"name-buffer", namebuffer},
	{"newline", insert_newline},
	{"newline-and-indent", indent},
	{"next-buffer", nextbuffer},
	{"next-line", forwline},
	{"next-page", forwpage},
	{"next-window", nextwind},
	{"next-word", forwword},
	{"nop", nullproc},
	{"open-line", openline},
	{"overwrite-string", ovstring},
	{"pipe-command", pipecmd},
	{"previous-line", backline},
	{"previous-page", backpage},
	{"previous-window", prevwind},
	{"previous-word", backword},
	{"query-replace-string", qreplace},
	{"quick-exit", quickexit},
	{"quote-character", quote},
	{"read-file", fileread},
	{"redraw-display", reposition},
	{"resize-window", resize},
	{"restore-window", restwnd},
	{"replace-string", sreplace},
	{"reverse-incremental-search", risearch},
	{"save-file", filesave},
	{"save-window", savewnd},
	{"scroll-next-up", scrnextup},
	{"scroll-next-down", scrnextdw},
	{"search-forward", forwsearch},
	{"search-reverse", backsearch},
	{"select-buffer", usebuffer},
	{"set-mark", setmark},
	{"shell-command", spawn},
	{"shrink-window", shrinkwind},
	{"split-current-window", splitwind},
#if BSD | __hpux | SVR4
	{"suspend-emacs", bktoshell},
#endif
	{"transpose-characters", twiddle},
	{"trim-line", trim},
	{"universal-argument", unarg},
	{"unmark-buffer", unmark},
	{"update-screen", upscreen},
	{"view-file", viewfile},
	{"wrap-word", wrapword},
	{"write-file", filewrite},
	{"write-message", writemsg},
	{"yank", yank},

	{"", NULL}
};
