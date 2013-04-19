#include	<stdlib.h>
#include	<string.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"cube.h"

int width = 0, height = 0, gridsize = 0;
char *grid = NULL;
short (*neighbors)[9] = NULL;

static const char *sixteendice[16] = {
	"ednosw", "aaciot", "acelrs", "ehinps",
	"eefhiy", "elpstu", "acdemp", "gilruw",
	"egkluy", "ahmors", "abilty", "adenvz",
	"bfiorx", "dknotu", "abjmoq", "egintv"
};
static const char *twentyfivedice[25] = {
	"aegmnn", "aafirs", "ensssu", "aaeeee", "aeegum",
	"ceipst", "ddhnot", "adennn", "oontuw", "ccenst",
	"dhlnor", "ooottu", "emottt", "afirsy", "dhhlor",
	"eiiitt", "gorrvw", "aeeeem", "dhlnor", "ceilpt",
	"aaafrs", "fiprsy", "ceiilt", "iprrry", "bqxkjz"
};
static const float letterfreq[26] = {
	 2493,  3140,  4387,  5643,  9524,  9987, 10999, 11734, 14548,
	14613, 14953, 16663, 17562, 19762, 21745, 22683, 22742, 25104,
	28250, 30325, 31399, 31733, 32043, 32141, 32631, 32768
};

static char *gridtemp;
static const char **dice;

#define rnd(N)		    ((int)(((float)rand() * (N)) / (float)RAND_MAX))

#define	addneighbor(n, y, x)						\
    ((void)((x) >= 0 && (x) < width && (y) >= 0 && (y) < height		\
	    && (neighbors[n][++neighbors[n][0]] = (y) * width + (x))))

static void destroy(void)
{
    free(grid);
    free(gridtemp);
    free(neighbors);
}

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
	    int m = rnd(32768);
	    n = 0;
	    while (letterfreq[n] < m)
		++n;
	    grid[i] = 'a' + n;
	}
    }
}

static char *wordcopy(char const *in)
{
    char *out, *ret;

    for (out = ret = xmalloc(strlen(in) + 1) ; *in ; ++in, ++out) {
	*out = *in;
	if (*in == 'q')
	    ++in;
    }
    *out = '\0';
    return ret;
}

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

char *findword(char const *wd)
{
    char *pos, *word;
    int n;
    int found = FALSE;

    memcpy(gridtemp, grid, gridsize);
    word = wordcopy(wd);
    if (!word[1]) {
	pos = strchr(gridtemp, word[0]);
	if (pos) {
	    *pos = 1;
	    found = TRUE;
	}
    } else {
	n = 0;
	while ((pos = memchr(gridtemp + n, word[0], gridsize - n)) != NULL) {
	    *pos = 1;
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
	if (gridtemp[n] >= 'a')
	    gridtemp[n] = 0;
    return gridtemp;
}
