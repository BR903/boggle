/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdio.h>
#include	<string.h>
#include	"genutil.h"
#include	"cube.h"
#include	"words.h"
#include	"batch.h"

/* Filter stdin according to the given grid. The strings in the function
 * arguments are pasted together to make a grid, aborting if the pasted
 * string is of the incorrect size. Then, lines from stdin are read, and
 * only the lines that contain a single word that can be found in the
 * given grid are echoed to stdout.
 */
int filtergrid(int count, char *pieces[])
{
    char buf[WORDBUFSIZ];
    int skipnext;
    int i, m, n = 0;

    if (!count)
	expire("No letters specified.");
    for (i = 0 ; i < count ; ++i) {
	m = strlen(pieces[i]);
	if (n + m > gridsize)
	    expire("Excessive number of letters supplied.");
	memcpy(grid + n, pieces[i], m);
	n += m;
    }
    if (n != gridsize)
	expire("Not enough letters supplied.");

    skipnext = FALSE;
    while (fgets(buf, sizeof buf, stdin)) {
	n = strlen(buf) - 1;
	if (buf[n] == '\n') {
	    if (skipnext) {
		skipnext = FALSE;
		continue;
	    }
	    buf[n] = '\0';
	    if (n >= minlen && findwordingrid(buf, NULL))
		puts(buf);
	} else
	    skipnext = TRUE;
    }

    return ferror(stdin);
}
