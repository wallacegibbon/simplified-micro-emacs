/* EPATH.H
 *
 *	This file contains certain info needed to locate the
 *	initialization (etc) files on a system dependent basis
 *
 *	modified by Petri Kutvonen
 */
#ifndef __EPATH_H
#define __EPATH_H

/* possible names and paths of help files under different OSs */
static char *pathname[] =
#if V7 | BSD | USG
{
	"emacs.hlp",
#if PKCODE
	"/usr/global/lib/", "/usr/local/bin/", "/usr/local/lib/",
#endif
	"/usr/local/", "/usr/lib/", ""
};
#endif

#if VMS
{
	"emacs.hlp", "",
#if PKCODE
	"sys$login:", "emacs_dir:",
#endif
	"sys$sysdevice:[vmstools]"
};
#endif

#endif
