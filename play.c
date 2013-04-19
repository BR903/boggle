/* play.c: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include	<stdlib.h>
#include	<string.h>
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

	displaygamestart();
	addtocolumn(-1);
	starttimer(0);
	while (runtimer()) {
	    char const *input = inputword(TRUE);
	    if (!input || !*input)
		break;
	    addtocolumn(acceptword(input, feedback) ? strlen(input) : 0);
	}
	stoptimer();

	if (!feedback)
	    filterfound();
	scoregame();
	reportscore();
	if (!doendgameinputloop())
	    break;
    }

    return EXIT_SUCCESS;
}
