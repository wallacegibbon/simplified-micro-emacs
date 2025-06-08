#include "estruct.h"
#include "edef.h"
#include <stdio.h>

int main(void)
{
	int ch;
	TTopen();
	for (;;) {
		ch = TTgetc();
		if (ch == 3 /* Ctrl + C */)
			break;
		printf("<%02X>", ch);
		fflush(stdout);
	}
	TTclose();
	printf("\r\n");
	return 0;
}
