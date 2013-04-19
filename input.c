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
char const *inputword(int enablehelp)
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
		--len;
		mvaddch(y, x + len, ' ');
		move(y, x + len);
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
 * can be found on the grid).
 */
int getendgameinput(void)
{
    int y, x, ch;
    char *highlit = NULL;

    addline("^D: quit  &: new game  ?: find word");
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
	if (ch == CTRL('D'))
	    return FALSE;
	if (ch == '&')
	    return TRUE;
	if (ch == '?') {
	    char const *input;
	    addstr("Word to find: ");
	    refresh();
	    input = inputword(FALSE);
	    if (input && (highlit = findwordingrid(input)))
		drawgridletters(highlit);
	    else
		beep();
	}
    }
}
