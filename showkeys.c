#include <ncurses.h>
#include <unistd.h>

int main()
{
	int ch = 0;

	initscr();
	raw();	/* Disable line buffering */
	keypad(stdscr, TRUE); /* Enable special keys (F1, Arrow keys, etc.) */
	refresh();

	for (;;) {
		/* Micro Emacs use `read`, which get 13 for Enter */
		/* If we use `getch`, we will get 10 for Enter */
		read(0, &ch, 1);
		if (ch == 3)
			break; /* stop on ^C */

		printw(" <%d (0x%X)> ", ch, ch);
		refresh();
	}

	endwin();
	return 0;
}
