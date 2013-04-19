/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdlib.h>
#include	<string.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"words.h"
#include	"cube.h"

/* Obtain a random number between 0 and N - 1.
 */
#define rnd(N)		    ((int)(((float)rand() * (N)) / (float)RAND_MAX))

/* The width, height and number of cells in the current grid.
 */
int width = 0, height = 0, gridsize = 0;

/* The contents of the current grid.
 */
char *grid = NULL;

/* The lists of each cell's neighboring cells. neighbors[n][0] is the
 * count of neighbors for the nth cell, and neighbors[n][1] through
 * neighbors[n][neighbors[n][0]] contain the actual indices (in no
 * particular order).
 */
short (*neighbors)[9] = NULL;

/* The dice to use for grids of size 4x4 and 5x5.
 */
static char const *sixteendice[16] = {
	"ednosw", "aaciot", "acelrs", "ehinps",
	"eefhiy", "elpstu", "acdemp", "gilruw",
	"egkluy", "ahmors", "abilty", "adenvz",
	"bfiorx", "dknotu", "abjmoq", "egintv"
};
static char const *twentyfivedice[25] = {
	"aegmnn", "aafirs", "ensssu", "aaeeee", "aeegum",
	"ceipst", "ddhnot", "adennn", "oontuw", "ccenst",
	"dhlnor", "ooottu", "emottt", "afirsy", "dhhlor",
	"eiiitt", "gorrvw", "aeeeem", "dhlnor", "ceilpt",
	"aaafrs", "fiprsy", "ceiilt", "iprrry", "bqxkjz"
};

/* A gridsized scratch buffer.
 */
static char *gridtemp;

/* A pointer to the dice array, if dice are being used.
 */
static char const **dice;

/* Macro for adding the nth cell to the list of neighbors for the cell
 * at (x,y) if it exists.
 */
#define	addneighbor(n, y, x)						\
    ((void)((x) >= 0 && (x) < width && (y) >= 0 && (y) < height		\
	    && (neighbors[n][++neighbors[n][0]] = (y) * width + (x))))

/* Exit function for this module.
 */
static void destroy(void)
{
    free(grid);
    free(gridtemp);
    free(neighbors);
}

/* The initialization function for this module. Parses the cmdline
 * options -4 and -b (-5 is ignored, being the default), and
 * allocates and/or initializes static variables.
 */
int cubeinit(char *opts[])
{
    int	i, n, x, y;

    if (opts['D'])
	return TRUE;
    atexit(destroy);

    if (opts['b']) {
	n = opts['b'][0] - '0';
	if (n < 2 || n > 9)
	    expire("Invalid board dimension");
	width = n;
	n = opts['b'][1];
	if (n) {
	    i = 2;
	    if (n == 'X' || n == 'x' || n == ',') {
		n = opts['b'][2];
		i = 3;
	    }
	    n -= '0';
	    if (n < 2 || n > 9 || opts['b'][i])
		expire("Invalid board dimensions");
	    height = n;
	} else
	    height = width;
	dice = NULL;
    } else if (opts['4']) {
	width = height = 4;
	dice = sixteendice;
    } else {
	width = height = 5;
	dice = twentyfivedice;
    }

    gridsize = width * height;
    grid = xmalloc(gridsize);
    gridtemp = xmalloc(gridsize);

    neighbors = xmalloc(gridsize * 9 * sizeof(short));
    for (y = 0 ; y < height ; ++y) {
	for (x = 0 ; x < width ; ++x) {
	    n = y * width + x;
	    neighbors[n][0] = 0;
	    addneighbor(n, y - 1, x - 1);
	    addneighbor(n, y - 1, x);
	    addneighbor(n, y - 1, x + 1);
	    addneighbor(n, y, x - 1);
	    addneighbor(n, y, x + 1);
	    addneighbor(n, y + 1, x - 1);
	    addneighbor(n, y + 1, x);
	    addneighbor(n, y + 1, x + 1);
	}
    }

    if (opts['B'])
	mode = MODE_BATCH;

    return TRUE;
}

static int letterindex(char letter)
{
    int i;

    for (i = 0 ; i < SIZE_ALPHABET ; ++i)
	if (ALPHABET[i] == letter)
	    return i;
    return 0;
}

/* Fill the grid with randomly selected letters. If the grid's
 * dimensions are either 4x4 or a 5x5, then dice are used. Otherwise,
 * letters are selected based on the letterfreq table.
 */
void shakecube(void)
{
    int i, n;

    if (dice) {
	for (i = 0 ; i < gridsize ; ++i)
	    gridtemp[i] = dice[i][rnd(6)];
	for (i = 0 ; i < gridsize ; ++i) {
	    n = rnd(gridsize - i);
	    grid[i] = gridtemp[n];
	    gridtemp[n] = gridtemp[gridsize - i - 1];
	}
    } else {
	for (i = 0 ; i < gridsize ; ++i) {
	    int m = rnd(freqdenom);
	    n = -1;
	    do
		m -= letterfreq[++n];
	    while (m > 0);
	    grid[i] = ALPHABET[n];
	}
    }
}

/* Return a newly allocated buffer with the given word copied into it,
 * with any occurrences of "qu" shortened to "q".
 */
static char *wordcopy(char const *in)
{
    char *out, *ret;

    for (out = ret = xmalloc(strlen(in) + 1) ; *in ; ++in, ++out) {
	*out = *in;
	if (*in == 'q') {
	    ++in;
	    if (*in != 'u') {
		free(ret);
		return NULL;
	    }
	}
    }
    *out = '\0';
    return ret;
}

/* A recursive function that does the real work for findwordingrid().
 * Returns TRUE if word can be found by starting at pos. The function
 * assumes that the first letter of word has already been verified.
 */
static int auxfindword(int pos, char *word)
{
    int i, n;

    for (i = 1; i <= neighbors[pos][0] ; ++i) {
	n = neighbors[pos][i];
	if (gridtemp[n] == word[0]) {
	    gridtemp[n] = gridtemp[pos] + 1;
	    if (!word[1] || auxfindword(n, word + 1))
		return TRUE;
	    gridtemp[n] = word[0];
	}
    }
    return FALSE;
}

/* Locate a word within the grid. If the word is not to be found in
 * the current grid, return NULL. Otherwise, return gridtemp, filled
 * out with 1-based indices indicating which letter in the word
 * appears at that cell.
 */
char *findwordingrid(char const *wd)
{
    char *pos, *word;
    int n;
    int found = FALSE;

    for (n = 0 ; n < gridsize ; ++n)
	gridtemp[n] = letterindex(grid[n]) + 1;
    if (!(word = wordcopy(wd)))
	return NULL;
    for (n = 0 ; word[n] ; ++n)
	word[n] = letterindex(word[n]) + 1;
    if (!word[1]) {
	pos = strchr(gridtemp, word[0]);
	if (pos) {
	    *pos = SIZE_ALPHABET + 1;
	    found = TRUE;
	}
    } else {
	n = 0;
	while ((pos = memchr(gridtemp + n, word[0], gridsize - n)) != NULL) {
	    *pos = SIZE_ALPHABET + 1;
	    n = pos - gridtemp;
	    if (auxfindword(n, word + 1)) {
		found = TRUE;
		break;
	    }
	    *pos = word[0];
	    ++n;
	}
    }

    free(word);
    if (!found)
	return NULL;
    for (n = 0 ; n < gridsize ; ++n)
	if (gridtemp[n] <= SIZE_ALPHABET)
	    gridtemp[n] = 0;
	else
	    gridtemp[n] -= SIZE_ALPHABET;
    return gridtemp;
}
