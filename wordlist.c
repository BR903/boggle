/* wordlist.c: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include	<stdlib.h>
#include	<string.h>
#include	"genutil.h"
#include	"wordlist.h"

/* How much to increase the array of string pointers by each time
 * space runs out.
 */
static const int listchunk = 64;

/* The size of each pool.
 */
static const int poolchunk = 4096;

/* Initializes a wordlist structure. Allocates the first pool and
 * sets the list's size to zero.
 */
void initwordlist(wordlist *list)
{
    list->words = xmalloc(listchunk * sizeof(char*));
    list->pools = xmalloc(poolchunk);
    list->alloced = listchunk;
    *((char**)list->pools) = NULL;
    list->poolptr = list->pools + sizeof(char*);
    list->poolstop = list->pools + poolchunk;
    list->count = 0;
    list->words[0] = NULL;
}

/* Appends a word to the end of a wordlist, (re)allocating memory as
 * necessary.
 */
void addwordtolist(wordlist *list, char const *word)
{
    char **pool;
    int n;

    n = strlen(word) + 1;
    if (list->poolptr + n > list->poolstop) {
	pool = (char**)(list->poolstop - poolchunk);
	*pool = xmalloc(poolchunk);
	*(char**)*pool = NULL;
	list->poolptr = *pool + sizeof(char*);
	list->poolstop = *pool + poolchunk;
    }
    list->words[list->count++] = list->poolptr;
    if (list->count == list->alloced) {
	list->alloced += listchunk;
	list->words = xrealloc(list->words, list->alloced * sizeof(char*));
    }
    list->words[list->count] = NULL;
    memcpy(list->poolptr, word, n);
    list->poolptr += n;
}

/* Removes a word from a list. Note that the string itself is not
 * removed from the pools.
 */
void strikewordfromlist(wordlist *list, int idx)
{
    memmove(list->words + idx, list->words + idx + 1,
	    ((list->count--) - idx) * sizeof(char*));
}

/* Frees all memory associated with a wordlist.
 */
void destroywordlist(wordlist *list)
{
    char **pool, **nextpool;

    free(list->words);
    if (list->pools) {
	for (pool = (char**)*(char**)list->pools ; pool ; pool = nextpool) {
	    nextpool = (char**)*pool;
	    free(pool);
	}
	free(list->pools);
    }
}

/* A comparison callback function.
 */
static int wordcmp(void const *w1, void const *w2)
{
    return strcmp(*(char**)w1, *(char**)w2);
}

/* Sorts the words in a wordlist.
 */
void sortwordlist(wordlist *list)
{
    qsort(list->words, list->count, sizeof(char*), wordcmp);
}

/* Returns the index for a word in a wordlist.
 */
char **findwordinlist(wordlist const *list, char const *word)
{
    return (char**)bsearch(&word, list->words, list->count,
			   sizeof(char*), wordcmp);
}
