#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"cube.h"
#include	"wordlist.h"
#include	"words.h"

static const char defdictfile[] = "boggle.dict";

char *dictfilename = NULL;
int minlen = 0;

int letterfreq[26];

static unsigned *dictionary = NULL;
static int finalstates = 0;

static wordlist findable;
static wordlist found;

static void destroy(void)
{
    free(dictionary);
    free(dictfilename);
    destroywordlist(&findable);
    destroywordlist(&found);
}

int wordsinit(char *opts[])
{
    FILE *fp;
    int n;
    unsigned short sig;

    if (opts['w']) {
	minlen = atoi(opts['w']);
	if (minlen < 2)
	    expire("Invalid minimum word size");
    } else if (opts['4'])
	minlen = 3;
    else
	minlen = 4;

    if (opts['B'])
	return TRUE;
    atexit(destroy);

    if (opts['d']) {
	dictfilename = xmalloc(strlen(opts['d']) + 1);
	strcpy(dictfilename, opts['d']);
    } else {
	char *basename;
	basename = strrchr(thisfile, DIRSEPCHAR);
	if (basename)
	    n = basename - thisfile + 1;
	else
	    n = 0;
	dictfilename = xmalloc(n + sizeof defdictfile);
	memcpy(dictfilename, thisfile, n);
	strcpy(dictfilename + n, defdictfile);
    }

    if (opts['D']) {
	mode = MODE_MAKEDICT;
	return TRUE;
    }

    fp = fopen(dictfilename, "rb");
    if (!fp) {
	perror(dictfilename);
	expire(NULL);
    }
    if (fread(&sig, 1, 2, fp) != 2) {
	perror(dictfilename);
	expire(NULL);
    }
    if (sig != 0xCBDF) {
	fputs(dictfilename, stderr);
	expire(": not a valid dictionary file");
    }
    if (fread(letterfreq, 4, 26, fp) != 26 || fread(&finalstates, 4, 1, fp) != 1
					   || fread(&n, 4, 1, fp) != 1) {
	fputs(dictfilename, stderr);
	expire(": not a valid dictionary file");
    }
    dictionary = xmalloc(n);
    if ((int)fread(dictionary, 1, n, fp) != n) {
	perror(dictfilename);
	expire(NULL);
    }
    fclose(fp);

    initwordlist(&findable);
    initwordlist(&found);

    return TRUE;
}

char const **getfindable(void)
{
    return (char const**)findable.words;
}

char const **getfound(void)
{
    return (char const**)found.words;
}

int acceptword(char const *word, int check)
{
    char **pos;

    if (check) {
	pos = findwordinlist(&findable, word);
	if (!pos)
	    return FALSE;
	strikewordfromlist(&findable, pos - findable.words);
    }
    addwordtolist(&found, word);
    return TRUE;
}

void filterfound(void)
{
    int n;

    sortwordlist(&found);
    n = 1;
    while (n < found.count) {
	if (strcmp(found.words[n - 1], found.words[n]))
	    ++n;
	else
	    strikewordfromlist(&found, n);
    }
    n = 0;
    while (n < found.count) {
	char **pos = findwordinlist(&findable, found.words[n]);
	if (pos) {
	    strikewordfromlist(&findable, pos - findable.words);
	    ++n;
	} else
	    strikewordfromlist(&found, n);
    }
}

void auxfindall(char *gd, int pos, int idx, char *word, int len)
{
    unsigned node;
    int ltr;
    int i, n;
    char saved;

    for (;;) {
	node = dictionary[idx];
	ltr = node & 0xFF;
	if (ltr == gd[pos]) {
	    idx = node >> 9;
	    break;
	} else if (ltr > gd[pos] || node & 0x0100)
	    return;
	++idx;
    }
    saved = word[len++] = gd[pos];
    if (gd[pos] == 'q')
	word[len++] = 'u';
    gd[pos] = '.';
    if (len >= minlen && idx >= finalstates) {
	word[len] = '\0';
	addwordtolist(&findable, word);
    }
    for (i = 1 ; i <= neighbors[pos][0] ; ++i) {
	n = neighbors[pos][i];
	if (gd[n] != '.')
	    auxfindall(gd, n, idx, word, len);
    }
    gd[pos] = saved;
}

int findall(void)
{
    char *gd, *word;
    int n;

    emptywordlist(&findable);
    emptywordlist(&found);

    gd = xmalloc(gridsize);
    memcpy(gd, grid, gridsize);
    word = xmalloc(WORDBUFSIZ);
    for (n = 0 ; n < gridsize ; ++n)
	auxfindall(gd, n, 0, word, 0);
    free(word);

    sortwordlist(&findable);
    n = 1;
    while (n < findable.count) {
	if (strcmp(findable.words[n - 1], findable.words[n]))
	    ++n;
	else
	    strikewordfromlist(&findable, n);
    }

    return findable.count;
}
