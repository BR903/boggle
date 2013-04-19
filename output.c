#include	<stdlib.h>
#include	<ctype.h>
#include	<stdarg.h>
#include	<signal.h>
#include	<ncurses.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"cube.h"
#include	"input.h"
#include	"output.h"

static const char *inputhelp[] = { "<--------------------------------------->",
				   "SPC or RET . . enter current word",
				   "^U or ^W . . . erase current word",
				   "BKSP . . . . . erase last letter",
				   "^T . . . . . . transpose last letters",
				   "^L or ^R . . . redraw screen",
				   "^D . . . . . . quit current game",
				   "?  . . . . . . display help"
};

static int ywords = 0, xwords = 0, ystatus = 0, xstatus = 0;

static int highlightattr = A_STANDOUT;

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

static void onresize(int sig)
{
    if (sig == SIGWINCH) {
	endwin();
	clearok(stdscr, TRUE);
	refresh();
    }
}

int outputinit(char *opts[])
{
    struct sigaction act;
    int y, x;

    xwords = 0;
    ywords = height * 2 + 1;
    xstatus = width * 4 + 4;
    ystatus = 0;

    if (!(mode & MM_CURSES))
	return TRUE;

    initscr();
    nonl();
    noecho();
    cbreak();
    getmaxyx(stdscr, y, x);
    setscrreg(ywords, y - 1);
    highlightattr = termattrs() & A_BOLD ? A_BOLD : A_STANDOUT;

    atexit(destroy);
    act.sa_handler = onresize;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGWINCH, &act, NULL);

    return TRUE;
    (void)opts;
}

void movetowords(int clear)
{
    move(ywords, xwords);
    if (clear)
	clrtobot();
}

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

void addline(char const *fmt, ...)
{
    int y, x;
    va_list args;

    va_start(args, fmt);
    getyx(stdscr, y, x);
    vw_printw(stdscr, (char*)fmt, args);
    move(y + 1, x);
    va_end(args);
}

void listwords(char const *heading, char const **list)
{
    char const **word;
    int y, x, ymin, ymax, xmax;
    int lenmax, minextend;
    int skipped = FALSE;

    minextend = strlen(heading) + 2;
    addline(heading);
    getyx(stdscr, ymin, x);
    getmaxyx(stdscr, ymax, xmax);

    y = ymin;
    lenmax = 0;
    for (word = list ; *word ; ++word) {
	int n;
	if (y == ymax) {
	    if (x + lenmax + 1 >= xmax) {
		skipped = TRUE;
		break;
	    }
	    x += lenmax + 1;
	    y = ymin;
	    lenmax = 0;
	}
	n = strlen(*word);
	if (x + n < xmax)
	    mvaddstr(y++, x, *word);
	else
	    skipped = TRUE;
	if (n > lenmax)
	    lenmax = n;
    }
    if (skipped) {
	if (x + 3 >= xmax)
	    mvaddstr(ymax - 1, xmax - 3, "...");
	else if (y == ymax) {
	    mvaddstr(ymax - 1, x, "...");
	    clrtoeol();
	} else
	    mvaddstr(y, x, "...");
    }

    if (lenmax)
	x += lenmax + 2;
    if (minextend > x)
	x = minextend;
    if (x >= xmax)
	x = xmax - 1;
    move(ymin - 1, x);
}

void drawgridletters(char const *highlighting)
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
	if (highlighting && highlighting[n])
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

void drawgrid(void)
{
    int y, x;
    int	left = 0, right = width * 4, top = 0, bottom = height * 2;

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

    drawgridletters(NULL);

}

void displayinputhelp(int show)
{
    int y, x, xleft;
    int n;

    getmaxyx(stdscr, y, x);
    xleft = x - strlen(inputhelp[0]);
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
