#include	<stdlib.h>
#include	<ncurses.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"words.h"
#include	"cube.h"
#include	"timer.h"
#include	"score.h"
#include	"input.h"
#include	"output.h"
#include	"play.h"

static int feedback = TRUE;

int playinit(char *opts[])
{
    feedback = opts['p'] == NULL;
    return TRUE;
}

int playsolitaire(void)
{
    addstr("Type SPC to begin.... ");
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
	    char const *input = inputword();
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
	addline("^D: quit  &: new game  ESC: find word");
	refresh();
	if (!getendgameinput())
	    break;
    }

    return EXIT_SUCCESS;
}
