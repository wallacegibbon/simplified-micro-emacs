#include "estruct.h"
#include "edef.h"
#include <term.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
	int ch = 0;
	TTopen();
	for (;;) {
		/* For Enter key, `read` get 13, while `getch` get 10 */
		read(0, &ch, 1);
		if (ch == 3)
			break; /* stop on ^C */
		printf("<%02X>", ch);
		fflush(stdout);
	}
	printf("\r\n");
	TTclose();
	return 0;
}
