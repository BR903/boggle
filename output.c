/* output.c: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#define	_POSIX_SOURCE
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdarg.h>
#include	<signal.h>
#include	<curses.h>
#include	"boggle.h"
#include	"words.h"
#include	"genutil.h"
#include	"cube.h"
#include	"output.h"

/* The text of the online help.
 */
static char const *inputhelp[] = { "<----------------------------------->",
				   "SPC or RET . . enter current word",
				   "^U or ^W . . . erase current word",
				   "BKSP . . . . . erase letter",
				   "^T . . . . . . transpose letters",
				   "^L or ^R . . . redraw screen",
				   "^D . . . . . . quit current game",
				   "?  . . . . . . display help"
};

/* The top left coordinates of the area where wordlists are displayed.
 */
static int ywords = 0, xwords = 0;

/* The top left coordinates of the area where the player's status is
 * displayed.
 */
static int ystatus = 0, xstatus = 0;

/* The curses attribute to use for highlighting.
 */
static int highlightattr = A_STANDOUT;

/* Whether or not to show the unfound words at the end of each game.
 */
static int ego = 0;

/* Whether or not to use ACS characters to draw the board.
 */
static int noacs = 0;

/* Exit function for the module.
 */
static void destroy(void)
{
    int y, x;

    if (isendwin())
	return;

    getmaxyx(stdscr, y, x);
    move(y - 1, 0);
    clrtoeol();
    refresh();
    endwin();
}

#ifdef SIGWINCH
/* Signal handler for SIGWINCH.
 */
static void onresize(int sig)
{
    if (sig == SIGWINCH) {
	endwin();
	clearok(stdscr, TRUE);
	refresh();
    }
}
#endif /* SIGWINCH */

/* The initialization function for this module. Calculates the various
 * screen coordinates and initializes curses, and parses the cmdline
 * option -o.
 */
int outputinit(char *opts[])
{
    struct sigaction act;
    int y, x;

    ego = opts['o'] != NULL;
    noacs = opts['l'] != NULL;

    xwords = 0;
    ywords = height * 2 + 1;
    xstatus = width * 4 + 4;
    ystatus = 0;

    if (!(mode & MM_CURSES))
	return TRUE;

    if (!initscr())
	exit(EXIT_FAILURE);
    nonl();
    noecho();
    cbreak();
    getmaxyx(stdscr, y, x);
    if (xstatus + 1 >= x || ywords + 1 >= y) {
	endwin();
	fputs("Screen too small\n", stderr);
	exit(EXIT_FAILURE);
    }

    setscrreg(ywords, y - 1);
    highlightattr = termattrs() & A_BOLD ? A_BOLD : A_STANDOUT;

    atexit(destroy);

#ifdef SIGWINCH
    act.sa_handler = onresize;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGWINCH, &act, NULL);
#endif /* SIGWINCH */

    return TRUE;
}

/* Moves the cursor to the top left of the area where words are
 * displayed, and erase the area if clear is TRUE.
 */
void movetowords(int clear)
{
    move(ywords, xwords);
    if (clear)
	clrtobot();
}

/* Moves the cursor to the top left of the area where the game status
 * is displayed, and erase the area if clear is TRUE.
 */
void movetostatus(int clear)
{
    if (clear) {
	int y;
	for (y = ystatus ; y < ywords ; ++y) {
	    move(y, xstatus);
	    clrtoeol();
	}
    }
    move(ystatus, xstatus);
}

/* Displays a line of text (using printf-style formatting) and move
 * the cursor down one line, using its current x-coordinate as the
 * left margin.
 */
void addline(char const *fmt, ...)
{
    char buf[256];
    int y, x, n, m;
    va_list args;

    getmaxyx(stdscr, n, m);
    getyx(stdscr, y, x);
    if (x < m) {
	va_start(args, fmt);
	vsprintf(buf, (char*)fmt, args);
	va_end(args);
	addnstr(buf, m - x);
    }
    if (y < n - 1)
	++y;
    move(y, x);
}

/* Lists words in the given array. The current cursor location gives
 * the upper left of the area to use, which is assumed to extend to
 * the edges of the screen. The supplied heading is displayed first,
 * then the words are displayed in a column; if the bottom of the
 * screen is reached, then another column is begun.  (The longest word
 * in the list sets the width of the columns.) If the right edge of
 * the screen is reached, an ellipsis is displayed in the bottom right
 * corner. The offset argument indicates how many columns to skip
 * before displaying.
 */
static int listwords(char const *heading, char const **list, int offset)
{
    char const **word;
    int y, x, ymin, ymax, xmax, ycol;
    int lenmax, minextend, n;
    int finished = TRUE;

    getyx(stdscr, ymin, x);
    ++ymin;
    getmaxyx(stdscr, ymax, xmax);
    ycol = ymax - ywords - 1;

    minextend = strlen(heading) + 2;
    if (offset) {
	minextend += 4;
	addstr("... ");
    }
    addline(heading);

    for (word = list + offset * ycol ; *word ; x += lenmax + 1) {
	lenmax = 0;
	for (y = 0 ; y < ycol ; ++y) {
	    if (!word[y])
		break;
	    n = strlen(word[y]);
	    if (n > lenmax)
		lenmax = n;
	}
	if (x + lenmax > xmax) {
	    finished = FALSE;
	    mvaddstr(ymin - 1, xmax - 3, "...");
	    break;
	}

	for (y = ymin ; y < ymax && *word ; ++y, ++word)
	    mvaddstr(y, x, *word);
    }

    if (minextend > x)
	x = minextend;
    if (x >= xmax)
	x = xmax - 1;
    move(ymin - 1, x);

    return finished;
}

/* Displays the letters in the grid. If highlighting is not NULL, then
 * it gives an gridsized array of boolean values. For each element
 * that is positive, the corresponding letter is displayed with
 * highlighting.
 */
static void drawgridletters(int highlighted)
{
    int	ypos, xpos, x, n;
    int ch, attr;

    xpos = 2;
    ypos = 1;
    for (n = x = 0 ; n < gridsize ; ++n, ++x) {
	if (x == width) {
	    x = 0;
	    xpos = 2;
	    ypos += 2;
	}
	if (n == highlighted)
	    attr = highlightattr;
	else
	    attr = A_NORMAL;
	ch = toupper(grid[n]);
	mvaddch(ypos, xpos, attr | ch);
	if (ch == 'Q')
	    addch(attr | 'u');
	xpos += 4;
    }
}

/* Displays the current grid.
 */
void drawgrid(void)
{
    int y, x;
    int	left = 0, right = width * 4, top = 0, bottom = height * 2;

    if (noacs) {
	mvaddch(top, left, '+');
	for (x = left + 1 ; x < right ; ++x)
	    addch('-');
	addch('+');
	for (y = top + 1 ; y < bottom ; ++y) {
	    mvaddch(y, left, '|');
	    mvaddch(y, right, '|');
	}
	mvaddch(bottom, left, '+');
	for (x = left + 1 ; x < right ; ++x)
	    addch('-');
	addch('+');
	for (y = top + 1 ; y < bottom ; ++y) {
	    if ((y ^ top) & 1) {
		for (x = left + 4 ; x < right ; x += 4)
		    mvaddch(y, x, '|');
		mvaddch(y, x, '|');
	    } else {
		mvaddstr(y, left + 1, "---");
		for (x = left + 4 ; x < right ; x += 4)
		    addstr(" ---");
	    }
	}
    } else {
	mvaddch(top, left, ACS_ULCORNER);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	mvaddch(top, right, ACS_URCORNER);
	mvaddch(bottom, left, ACS_LLCORNER);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	mvaddch(bottom, right, ACS_LRCORNER);
	for (x = left + 4 ; x < right ; x += 4) {
	    mvaddch(top, x, ACS_TTEE);
	    addch(ACS_HLINE);
	    addch(ACS_HLINE);
	    addch(ACS_HLINE);
	    mvaddch(top + 1, x, ACS_VLINE);
	    mvaddch(bottom, x, ACS_BTEE);
	    addch(ACS_HLINE);
	    addch(ACS_HLINE);
	    addch(ACS_HLINE);
	}
	mvaddch(top + 1, left, ACS_VLINE);
	mvaddch(top + 1, right, ACS_VLINE);
	for (y = top + 2 ; y < bottom ; y += 2) {
	    mvaddch(y, left, ACS_LTEE);
	    addch(ACS_HLINE);
	    addch(ACS_HLINE);
	    addch(ACS_HLINE);
	    mvaddch(y + 1, left, ACS_VLINE);
	    mvaddch(y, right, ACS_RTEE);
	    mvaddch(y + 1, right, ACS_VLINE);
	}
	for (x = left + 4 ; x < right ; x += 4) {
	    for (y = top + 2 ; y < bottom ; y += 2) {
		mvaddch(y, x, ACS_PLUS);
		addch(ACS_HLINE);
		addch(ACS_HLINE);
		addch(ACS_HLINE);
		mvaddch(y + 1, x, ACS_VLINE);
	    }
	}
    }

    drawgridletters(-1);

}

/* Sets up the screen for the start of a game.
 */
void displaygamestart(void)
{
    clear();
    drawgrid();
    movetowords(FALSE);
    addch('\n');
    refresh();
}

/* Displays brief online help for the special keys during input if
 * show is TRUE, or erases it if show is FALSE.
 */
void displayinputhelp(int show)
{
    int y, x, xleft;
    int n;

    getmaxyx(stdscr, y, x);
    xleft = x - strlen(inputhelp[0]);
    if (xleft < xstatus) {
	beep();
	return;
    }
    getyx(stdscr, y, x);
    for (n = 1 ; n < (int)(sizeof inputhelp / sizeof *inputhelp) ; ++n) {
	move(n, xleft);
	if (show)
	    addstr(inputhelp[n]);
	else
	    clrtoeol();
    }
    move(y, x);
    refresh();
}

/* Draw the board, the help, and the two wordlists at the end of a
 * round. Returns zero if the end of screen was reached before the end
 * of the wordlist.
 */
int doendgameoutput(int y, int x, int offset, int highlighted)
{
    int f = TRUE;

    drawgridletters(highlighted);
    movetowords(TRUE);
    listwords("Your words:", getfound(), 0);
    if (!ego)
	f = listwords("Other words that were present:", getfindable(), offset);
    move(y, x);
    if (!offset && f)
	addline("^D: quit  &: new game  ?: find word");
    else
	addline("^D: quit  &: new game  ?: find word  -+: scroll");
    clrtoeol();
    return f;
}
