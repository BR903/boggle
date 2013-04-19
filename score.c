#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"words.h"
#include	"output.h"
#include	"score.h"

#define	PLURAL(n)	(n), ((n) == 1 ? "" : "s")

static const int wordscores[] = { 0, 1, 1, 1, 1, 2, 3, 5, 11 };
static const int maxwordscores = sizeof wordscores / sizeof *wordscores;

static int ego = 0;

static int ncount1, nscore1, dcount1, dscore1;
static int ncount = 0, nscore = 0, dcount = 0, dscore = 0;
static int gamecount = 0;

int scoreinit(char *opts[])
{
    ego = opts['o'] != NULL;
    return TRUE;
}

void reportwords(void)
{
    char const **found;
    char const **findable;
    int	i, n;

    found = getfound();
    findable = getfindable();
    nscore1 = 0;
    for (i = 0 ; found[i] ; ++i) {
	n = strlen(found[i]);
	if (n > maxwordscores)
	    n = maxwordscores;
	nscore1 += wordscores[n];
    }
    ncount1 = i;
    dscore1 = nscore1;
    for (i = 0 ; findable[i] ; ++i) {
	n = strlen(findable[i]);
	if (n > maxwordscores)
	    n = maxwordscores;
	dscore1 += wordscores[n];
    }
    dcount1 = ncount1 + i;

    ncount += ncount1;
    dcount += dcount1;
    nscore += nscore1;
    dscore += dscore1;
    ++gamecount;

    movetowords(TRUE);
    listwords("Your words:", found);
    if (!ego)
	listwords("Other words that were present:", findable);
}

void reportscore(void)
{
    movetostatus(TRUE);
    addline("%d word%s, scoring %d point%s", PLURAL(ncount1), PLURAL(nscore1));
    if (!ego) {
	addline("(of a possible %d scoring %d point%s)",
		dcount1, PLURAL(dscore1));
	addline("%.2f%% over %d game%s (%.2f%% by score)",
		((100.0 * ncount) / dcount), PLURAL(gamecount),
		((100.0 * nscore) / dscore));
    }
}
