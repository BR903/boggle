/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdlib.h>
#include	<curses.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"words.h"
#include	"cube.h"
#include	"timer.h"
#include	"score.h"
#include	"input.h"
#include	"output.h"
#include	"play.h"

/* Whether or not to check words as they are entered.
 */
static int feedback = TRUE;

/* The initialization function for this module. Parses the cmdline
 * option -p.
 */
int playinit(char *opts[])
{
    feedback = opts['p'] == NULL;
    return TRUE;
}

/* Play the solitaire game.
 */
int playsolitaire(void)
{
    if (!getstartinput())
	return EXIT_SUCCESS;

    for (;;) {
	do
	    shakecube();
	while (!findall());

	clear();
	drawgrid();
	movetowords(FALSE);
	scrollok(stdscr, TRUE);
	addch('\n');
	refresh();
	starttimer(0);
	while (runtimer()) {
	    char const *input = inputword(TRUE);
	    if (!input || !*input)
		break;
	    addch(acceptword(input, feedback) ? '\n' : '\r');
	    clrtoeol();
	    refresh();
	}
	stoptimer();
	scrollok(stdscr, FALSE);

	if (!feedback)
	    filterfound();
	reportwords();
	reportscore();
	refresh();
	if (!getendgameinput())
	    break;
    }

    return EXIT_SUCCESS;
}
