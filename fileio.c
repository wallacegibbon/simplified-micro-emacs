/* FILEIO.C
 *
 * The routines in this file read and write ASCII files from the disk. All of
 * the knowledge about files are here.
 *
 *	modified by Petri Kutvonen
 */

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include <stdio.h>

static FILE *ffp;			/* File pointer, all functions. */
static int eofflag;			/* end-of-file flag */

int ffropen(char *fn)
{
	if ((ffp = fopen(fn, "r")) == NULL)
		return FIOFNF;
	eofflag = FALSE;
	return FIOSUC;
}

/*
 * Open a file for writing. Return TRUE if all is well, and FALSE on error
 * (cannot create).
 */
int ffwopen(char *fn)
{
	if ((ffp = fopen(fn, "w")) == NULL) {
		mlwrite("Cannot open file for writing");
		return FIOERR;
	}
	return FIOSUC;
}

/*
 * Close a file. Should look at the status in all systems.
 */
int ffclose(void)
{
	/* free this since we do not need it anymore */
	if (fline) {
		free(fline);
		fline = NULL;
	}
	eofflag = FALSE;

#if UNIX
	if (fclose(ffp) != FALSE) {
		mlwrite("Error closing file");
		return FIOERR;
	}
	return FIOSUC;
#else
	fclose(ffp);
	return FIOSUC;
#endif
}

/*
 * Write a line to the already opened file. The "buf" points to the buffer,
 * and the "nbuf" is its length, less the free newline. Return the status.
 * Check only at the newline.
 */
int ffputline(char *buf, int nbuf)
{
	int i;
	for (i = 0; i < nbuf; ++i)
		fputc(buf[i] & 0xFF, ffp);

	fputc('\n', ffp);

	if (ferror(ffp)) {
		mlwrite("Write I/O error");
		return FIOERR;
	}

	return FIOSUC;
}

/*
 * Read a line from a file, and store the bytes in the supplied buffer. The
 * "nbuf" is the length of the buffer. Complain about long lines and lines
 * at the end of the file that don't have a newline present. Check for I/O
 * errors too. Return status.
 */
int ffgetline(int *count)
{
	char *tmpline;	/* temp storage for expanding line */
	int c, i;

	/* if we are at the end...return it */
	if (eofflag)
		return FIOEOF;

	/* dump fline if it ended up too big */
	if (flen > NSTRING) {
		free(fline);
		fline = NULL;
	}

	/* if we don't have an fline, allocate one */
	if (fline == NULL)
		if ((fline = malloc(flen = NSTRING)) == NULL)
			return FIOMEM;

	/* read the line in */
	i = 0;
	while ((c = fgetc(ffp)) != EOF && c != '\n') {
		fline[i++] = c;
		if (i >= flen) {
			if ((tmpline = malloc(flen + NSTRING)) == NULL)
				return FIOMEM;
			strncpy(tmpline, fline, flen);
			flen += NSTRING;
			free(fline);
			fline = tmpline;
		}
	}

	/* test for any errors that may have occured */
	if (c == EOF) {
		if (ferror(ffp)) {
			mlwrite("File read error");
			return FIOERR;
		}
		if (i != 0)
			eofflag = TRUE;
		else
			return FIOEOF;
	}

	*count = i;
	return FIOSUC;
}

int fexist(char *fname)
{
	FILE *fp = fopen(fname, "r");
	if (fp == NULL)
		return FALSE;

	fclose(fp);
	return TRUE;
}
