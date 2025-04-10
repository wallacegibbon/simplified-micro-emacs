#include <ncurses.h>

int main()
{
	initscr();

	/* Disable line buffering */
	raw();

	/* Enable special keys (F1, Arrow keys, etc.) */
	keypad(stdscr, TRUE);

	refresh();

	for (;;) {
		int ch = getch();
		if (ch == 3) break; /* stop on ^C */
		printw(" <%d (0x%X)> ", ch, ch);
		refresh();
	}

	endwin();
	return 0;
}
