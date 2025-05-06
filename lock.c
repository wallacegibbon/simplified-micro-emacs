#include "estruct.h"
#include "edef.h"
#include "efunc.h"

#if BSD | SVR4
#include <sys/errno.h>

static char *lname[NLOCKS];		/* names of all locked files */
static int numlocks;			/* # of current locks active */

/* check a file for locking and add it to the list */
int lockchk(char *fname)
{
	int status, i;

	/* check to see if that file is already locked here */
	if (numlocks > 0)
		for (i = 0; i < numlocks; ++i)
			if (strcmp(fname, lname[i]) == 0)
				return TRUE;

	/* if we have a full locking table, bitch and leave */
	if (numlocks == NLOCKS) {
		mlwrite("LOCK ERROR: Lock table full");
		return ABORT;
	}

	/* next, try to lock it */
	status = lock(fname);
	if (status == ABORT)	/* file is locked, no override */
		return ABORT;
	if (status == FALSE)	/* locked, overriden, dont add to table */
		return TRUE;

	/* we have now locked it, add it to our table */
	lname[++numlocks - 1] = malloc(strlen(fname) + 1);
	if (lname[numlocks - 1] == NULL) {	/* malloc failure */
		undolock(fname);	/* free the lock */
		mlwrite("Cannot lock, out of memory");
		--numlocks;
		return ABORT;
	}

	/* everthing is cool, add it to the table */
	strcpy(lname[numlocks - 1], fname);
	return TRUE;
}

/* Release all the file locks so others may edit */
int lockrel(void)
{
	int status, s, i;

	status = TRUE;
	if (numlocks > 0)
		for (i = 0; i < numlocks; ++i) {
			if ((s = unlock(lname[i])) != TRUE)
				status = s;
			free(lname[i]);
		}
	numlocks = 0;
	return status;
}

/*
 * Check and lock a file from access by others
 * returns	TRUE = files was not locked and now is
 *		FALSE = file was locked and overridden
 *		ABORT = file was locked, abort command
 */
int lock(char *fname)
{
	char *locker;
	char msg[NSTRING];
	int status;

	/* attempt to lock the file */
	locker = dolock(fname);
	if (locker == NULL)	/* we win */
		return TRUE;

	/* file failed...abort */
	if (strncmp(locker, "LOCK", 4) == 0) {
		lckerror(locker);
		return ABORT;
	}

	/* someone else has it....override? */
	strcpy(msg, "File in use by ");
	strcat(msg, locker);
	strcat(msg, ", override?");
	status = mlyesno(msg);	/* ask them */
	if (status == TRUE)
		return FALSE;
	else
		return ABORT;
}

/* Unlock a file.  This only warns the user if it fails */
int unlock(char *fname)
{
	char *locker;	/* undolock return string */

	/* unclock and return */
	locker = undolock(fname);
	if (locker == NULL)
		return TRUE;

	/* report the error and come back */
	lckerror(locker);
	return FALSE;
}

void lckerror(char *errstr)
{
	char obuf[NSTRING];

	strcpy(obuf, errstr);
	strcat(obuf, " - ");
	strcat(obuf, strerror(errno));
	mlwrite(obuf);
}

#endif
