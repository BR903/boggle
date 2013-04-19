#include	<stdio.h>
#include	<string.h>
#include	"genutil.h"
#include	"cube.h"
#include	"words.h"
#include	"batch.h"

int filtergrid(int count, char *pieces[])
{
    char buf[WORDBUFSIZ];
    int skipnext;
    int i, m, n;

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
	    if (n >= minlen && findword(buf))
		puts(buf);
	} else
	    skipnext = TRUE;
    }

    return ferror(stdin);
}
