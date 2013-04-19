/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"cube.h"
#include	"wordlist.h"
#include	"dict.h"
#include	"words.h"

/* Default dictionary pathname.
 */
char *dictfilename = NULL;

/* Minimum word length for the current game session.
 */
int minlen = 0;

/* Letter frequencies for the current dictionary.
 */
int letterfreq[SIZE_ALPHABET];
long freqdenom;

/* Pointer to the compressed dictionary.
 */
static arc *dictionary = NULL;

/* The index of the first final state in the compressed dictionary.
 */
static long finalstates = 0;

/* The list of words that can be found in the current grid.
 */
static wordlist findable;

/* The list of words that have been found in the current game.
 */
static wordlist found;

/* Reads in a compressed dictionary file.
 */
static int readdictfile(FILE *fp)
{
    dictfilehead header;
    int i;

    if (fread(&header, sizeof header, 1, fp) != 1)
	return FALSE;
    if (header.sig != DICTFILE_SIG)
	return FALSE;
    freqdenom = 0;
    for (i = 0 ; i < (int)(sizeof letterfreq / sizeof *letterfreq) ; ++i) {
	letterfreq[i] = (int)header.freq[i];
	freqdenom += (long)header.freq[i];
    }
    finalstates = (long)header.finalstates;
    dictionary = xmalloc(header.size);
    if (fread(dictionary, 1, header.size, fp) != (size_t)header.size)
	return FALSE;
    return TRUE;
}

/* Exit function for this module.
 */
static void destroy(void)
{
    free(dictionary);
    free(dictfilename);
    destroywordlist(&findable);
    destroywordlist(&found);
}

/* The initialization function for this module. Parses the cmdline
 * options -w, -4, and -d, reads the compressed dictionary file into
 * memory, and initializes static variables.
 */
int wordsinit(char *opts[])
{
    FILE *fp;

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
	dictfilename = xmalloc(sizeof DICTFILEPATH);
	strcpy(dictfilename, DICTFILEPATH);
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
    errno = 0;
    if (!readdictfile(fp)) {
	if (errno) {
	    perror(dictfilename);
	    expire(NULL);
	} else
	    expire("%s: not a valid dictionary file", dictfilename);
    }
    fclose(fp);

    initwordlist(&findable);
    initwordlist(&found);

    return TRUE;
}

/* Returns the finable list of words.
 */
char const **getfindable(void)
{
    return (char const**)findable.words;
}

/* Returns the found list of words.
 */
char const **getfound(void)
{
    return (char const**)found.words;
}

/* Accepts a word to be added to the found wordlist. If check is TRUE,
 * then it only accepts the word if it is currently in the findable
 * wordlist, in which case it is then removed.
 */
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

/* Removes all words from the found wordlist that are not in the
 * findable wordlist, and then removes all words from the findable
 * wordlist that are in the found wordlist.
 */
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

/* A recursive function that does the real work for findall(). The
 * function is called with the prefix of a word in the dictionary
 * having already been located on the grid. gd is a modifiable copy of
 * the grid, with the cells making up the found prefix replaced with
 * '.', pos is the cell that the prefix ends on, idx is the index into
 * the compressed dictionary for the end of the prefix, word is the
 * prefix itself, and len is the length of the prefix. For each
 * neighbor of pos, the function call itself recursively if the
 * current prefix plus the letter at the neighboring cell is also a
 * valid prefix. Whenever a complete word is found this way, the word
 * is added to the findable wordlist.
 */
void auxfindall(char *gd, int pos, long idx, char *word, int len)
{
    arc *node;
    int ltr;
    int i, n;
    char saved;

    node = dictionary + idx;
    for (;;) {
	ltr = node->letter;
	if (ltr == gd[pos]) {
	    idx = node->node;
	    break;
	} else if (ltr > gd[pos] || node->end)
	    return;
	++node;
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

/* Compiles the findable wordlist based on the current grid and the
 * compressed dictionary, returning the number of findable words, and
 * empties the found wordlist.
 */
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
