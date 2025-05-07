#include "estruct.h"
#include "usage.h"
#include <stdlib.h>

int xmkstemp(char *template)
{
	int fd = mkstemp(template);
	if (fd < 0)
		die("Unable to create temporary file");
	return fd;
}

void *xmalloc(size_t size)
{
	void *ret = malloc(size);
	if (!ret)
		die("Out of memory");
	return ret;
}
