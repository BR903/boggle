/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<curses.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"output.h"
#include	"timer.h"
#include	"cube.h"
#include	"input.h"

/* Macro for representing control characters.
 */
#ifdef CTRL
#undef CTRL
#endif
#define	CTRL(c)	(((c) - '@') & 0x7F)

/* User-set values for the backspace and line-erase keys.
 */
static int backspacekey = CTRL('?');
static int linekillkey = CTRL('U');

/* The initialization function for this module.
 */
int inputinit(char *opts[])
{
    if (mode & MM_CURSES) {
	backspacekey = erasechar();
	linekillkey = killchar();
    }
    return opts != NULL;
}

/* Move the cursor to the next line, or to the next column if at the
 * bottom of the screen. If size is zero, erase the current line. If
 * size is negative, use the current cursor position as the start of
 * the first column.
 */
void addtocolumn(int size)
{
    static int ycol = 0, xcol = 0, coltop = 0, colwidth = 0;
    int y, x;

    if (size < 0) {
	getyx(stdscr, ycol, xcol);
	coltop = ycol;
	colwidth = 0;
    } else if (size == 0) {
	move(ycol, xcol);
	clrtoeol();
	refresh();
    } else {
	if (colwidth < size)
	    colwidth = size;
	++ycol;
	getmaxyx(stdscr, y, x);
	if (ycol >= y) {
	    ycol = coltop;
	    xcol += colwidth + 1;
	    colwidth = 0;
	}
	move(ycol, xcol);
	refresh();
    }
}

/* Get a keystroke at the very beginning of the session, returning
 * FALSE if the keystore is ^D.
 */
int getstartinput(void)
{
    addline("Type SPC to begin....");
    for (;;) {
	int ch = getch();
	if (ch == CTRL('D'))
	    return FALSE;
	else if (ch == ' ')
	    return TRUE;
    }
}

/* Inputs a word, returning a pointer to a private buffer containing
 * the word, or NULL if the user types ^D. All the special keystrokes
 * are handled and uppercase letters are automatically lowered.
 */
char *inputword(int enablehelp)
{
    static char input[WORDBUFSIZ];
    int len, max;
    int done, stop;
    int helpdisplay = FALSE;
    int	y, x, ch;

    len = 0;
    done = stop = FALSE;
    getmaxyx(stdscr, y, max);
    getyx(stdscr, y, x);
    max -= x + 1;
    if (max >= (int)(sizeof input))
	max = sizeof input - 1;
    clrtoeol();

    while (!done && runtimer()) {
	ch = getch();
	if (ch == ERR)
	    continue;
	if (ch & ~0x7F) {
	    beep();
	} else if (isalpha(ch)) {
	    ch = tolower(ch);
	    if (len < max) {
		input[len++] = ch;
		echochar(ch);
	    } else
		beep();
	} else {
	    if (ch == backspacekey)
		ch = CTRL('?');
	    else if (ch == linekillkey)
		ch = CTRL('U');
	    switch (ch) {
	      case ' ':
	      case '\r':
	      case '\n':
		done = TRUE;
		break;
	      case CTRL('H'):
	      case CTRL('?'):
		if (len) {
		    --len;
		    mvaddch(y, x + len, ' ');
		    move(y, x + len);
		} else
		    beep();
		break;
	      case CTRL('U'):
	      case CTRL('W'):
		len = 0;
		move(y, x);
		clrtoeol();
		break;
	      case CTRL('T'):
		if (len > 1) {
		    ch = input[len - 2];
		    input[len - 2] = input[len - 1];
		    input[len - 1] = ch;
		    mvaddnstr(y, x, input, len);
		} else
		    beep();
		break;
	      case CTRL('L'):
	      case CTRL('R'):
		clearok(stdscr, TRUE);
		refresh();
		helpdisplay = FALSE;
		break;
	      case '?':
		if (enablehelp) {
		    helpdisplay = !helpdisplay;
		    displayinputhelp(helpdisplay);
		} else
		    beep();
		break;
	      case CTRL('D'):
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

/* Gets keystrokes at the end of each game, returning TRUE for & and
 * FALSE for ^D. If ? is pressed, the function stops to input a word
 * and then redraws the grid with that word highlighted (presuming it
 * can be found on the grid). The keys - and =/+ causes the wordlist
 * to be scrolled horizontally, if it cannot be fully displayed on the
 * screen.
 */
int doendgameinputloop(void)
{
    char const *input;
    char *highlighting = NULL;
    int highlightlen = 0;
    int index = 0;
    int offset, atend;
    int y, x;
    int ch;

    getyx(stdscr, y, x);
    offset = 0;
    for (;;) {
	atend = doendgameoutput(y, x, offset,
				highlighting ? highlighting[index] : -1);
	refresh();
	ch = getch();
	if (highlighting) {
	    halfdelay(7);
	    ++index;
	    if (index >= highlightlen || ch != ERR) {
		free(highlighting);
		highlighting = NULL;
		cbreak();
	    }
	}
	switch (ch) {
	  case CTRL('D'):
	    return FALSE;
	  case '&':
	    return TRUE;
	  case '-':
	    if (offset > 0)
		--offset;
	    break;
	  case '+':
	  case '=':	/* Unshifted plus on various QWERTY-based keyboards */
	  case '*':
	  case '1':
	  case '3':
	  case '4':
	    if (!atend)
		++offset;
	    break;
	  case '?':
	    addstr("Word to find: ");
	    refresh();
	    if (!(input = inputword(FALSE)))
		break;
	    if (!(highlightlen = findwordingrid(input, &highlighting))) {
		beep();
		break;
	    }
	    halfdelay(10);
	    index = 0;
	    break;
	}
    }
}
