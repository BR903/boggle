/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"words.h"
#include	"output.h"
#include	"score.h"

/* Macros to automatically handle plurals inside printf argument lists.
 */
#define WORDS(n)	(n), ((n) == 1 ? "word" : "words")
#define POINTS(n)	(n), ((n) == 1 ? "point" : "points")
#define GAMES(n)	(n), ((n) == 1 ? "game" : "games")

/* A structure that holds all the different statistics kept on the
 * game(s) being played. The paired elements are the statistic for the
 * most recent game, and the running total for all the games played
 * in this session, respectively.
 */
typedef	struct scores {
    int		gamecount;		/* number of games played	*/
    int		ncount1, ncount;	/* number of found words	*/
    int		nscore1, nscore;	/* points for the found words	*/
    int		dcount1, dcount;	/* number of words present	*/
    int		dscore1, dscore;	/* points for the words present	*/
} scores;

/* How many points the various word lengths are worth.
 */
static const int wordscores[] = { 0, 0, 1, 1, 1, 2, 3, 5, 11 };
static const int maxwordscores = sizeof wordscores / sizeof *wordscores;

/* The player's statistics, initially all zero.
 */
static scores scoring;

/* Whether or not to show the unfound words at the end of each game.
 */
static int ego = 0;

/* The initialization function for this module.
 */
int scoreinit(char *opts[])
{
    ego = opts['o'] != NULL;
    return TRUE;
}

/* Display all the words that were found (and the ones that were not
 * found, unless -o was used), and computer the player's current
 * statistics.
 */
void scoregame(void)
{
    char const **found;
    char const **findable;
    int	i, n;

    found = getfound();
    findable = getfindable();
    scoring.nscore1 = 0;
    for (i = 0 ; found[i] ; ++i) {
	n = strlen(found[i]);
	if (n >= maxwordscores)
	    n = maxwordscores - 1;
	scoring.nscore1 += wordscores[n];
    }
    scoring.ncount1 = i;
    scoring.dscore1 = scoring.nscore1;
    for (i = 0 ; findable[i] ; ++i) {
	n = strlen(findable[i]);
	if (n >= maxwordscores)
	    n = maxwordscores - 1;
	scoring.dscore1 += wordscores[n];
    }
    scoring.dcount1 = scoring.ncount1 + i;

    scoring.ncount += scoring.ncount1;
    scoring.dcount += scoring.dcount1;
    scoring.nscore += scoring.nscore1;
    scoring.dscore += scoring.dscore1;
    ++scoring.gamecount;
}

/* Display the current statistics.
 */
void reportscore(void)
{
    movetostatus(TRUE);
    addline("%d %s, scoring %d %s",
	    WORDS(scoring.ncount1), POINTS(scoring.nscore1));
    if (!ego && scoring.dcount1 && scoring.dscore1)
	addline("out of %d %s (%.2f%%) for %d %s (%.2f%%)",
		WORDS(scoring.dcount1),
		(100.0 * scoring.ncount1) / scoring.dcount1,
		POINTS(scoring.dscore1),
		(100.0 * scoring.nscore1) / scoring.dscore1);
    if (scoring.gamecount > 1) {
	addline("%d %s for %d %s over %d games",
		WORDS(scoring.ncount), POINTS(scoring.nscore),
		scoring.gamecount);
	if (!ego && scoring.dcount && scoring.dscore)
	    addline("out of %d %s (%.2f%%) for %d %s (%.2f%%)",
		    WORDS(scoring.dcount),
		    (100.0 * scoring.ncount) / scoring.dcount,
		    POINTS(scoring.dscore),
		    (100.0 * scoring.nscore) / scoring.dscore);
    }
}
