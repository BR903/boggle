#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<ncurses.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"output.h"
#include	"timer.h"
#include	"cube.h"
#include	"input.h"

#define	ctl(c)	(((c) - '@') & 0x7F)

static int backspacekey = ctl('?');
static int linekillkey = ctl('U');

int inputinit(char *opts[])
{
    if (mode & MM_CURSES) {
	backspacekey = erasechar();
	linekillkey = killchar();
    }
    return opts != NULL;
}

int getstartinput(void)
{
    for (;;) {
	int ch = getch();
	if (ch == ctl('D'))
	    return FALSE;
	else if (ch == ' ')
	    return TRUE;
    }
}

char const *inputword(void)
{
    static char input[WORDBUFSIZ];
    int len;
    int done, stop;
    int helpdisplay = FALSE;
    int	y, x, ch;

    len = 0;
    done = stop = FALSE;
    getyx(stdscr, y, x);
    clrtoeol();

    while (!done && runtimer()) {
	ch = getch();
	if (ch == ERR)
	    continue;
	if (ch & ~0x7F) {
	    beep();
	} else if (isalpha(ch)) {
	    ch = tolower(ch);
	    input[len++] = ch;
	    echochar(ch);
	} else {
	    if (ch == backspacekey)
		ch = ctl('?');
	    else if (ch == linekillkey)
		ch = ctl('U');
	    switch (ch) {
	      case ' ':
	      case '\r':
	      case '\n':
		done = TRUE;
		break;
	      case ctl('H'):
	      case ctl('?'):
		--len;
		mvaddch(y, x + len, ' ');
		move(y, x + len);
		break;
	      case ctl('U'):
	      case ctl('W'):
		len = 0;
		move(y, x);
		clrtoeol();
		break;
	      case ctl('T'):
		if (len > 1) {
		    ch = input[len - 2];
		    input[len - 2] = input[len - 1];
		    input[len - 1] = ch;
		    mvaddnstr(y, x, input, len);
		} else
		    beep();
		break;
	      case ctl('L'):
	      case ctl('R'):
		clearok(stdscr, TRUE);
		refresh();
		helpdisplay = FALSE;
		break;
	      case '?':
		helpdisplay = !helpdisplay;
		displayinputhelp(helpdisplay);
		break;
	      case ctl('D'):
		done = stop = TRUE;
		break;
	      default:
		beep();
		break;
	    }
	    if (done) {
		if (helpdisplay)
		    displayinputhelp(FALSE);
		if (stop ? len != 0 : len == 0) {
		    done = stop = FALSE;
		    beep();
		}
	    }
	}
    }

    if (stop)
	return NULL;
    input[len] = '\0';
    return input;
}

int getendgameinput(void)
{
    int y, x, ch;
    char *highlit = NULL;

    getyx(stdscr, y, x);
    for (;;) {
	move(y, x);
	clrtoeol();
	ch = getch();
	if (highlit) {
	    highlit = NULL;
	    drawgridletters(NULL);
	    move(y, x);
	    clrtoeol();
	    refresh();
	}
	if (ch == ctl('D'))
	    return FALSE;
	if (ch == '&')
	    return TRUE;
	if (ch == ctl('[')) {
	    char const *input;
	    addstr("Word to find: ");
	    refresh();
	    input = inputword();
	    if (input && (highlit = findword(input)))
		drawgridletters(highlit);
	    else
		beep();
	}
    }
}
