#ifndef __EPATH_H
#define __EPATH_H

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

#endif
