#include	<stdlib.h>
#include	<string.h>
#include	"genutil.h"
#include	"wordlist.h"

static const int listchunk = 64;
static const int poolchunk = 4096;

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

void addwordtolist(wordlist *list, char const *word)
{
    char **pool;
    int n;

    n = strlen(word) + 1;
    if (list->poolptr + n > list->poolstop) {
	pool = (char**)(list->poolstop - poolchunk);
	assert(*pool == NULL);
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

static int wordcmp(const void *w1, const void *w2)
{
    return strcmp(*(char**)w1, *(char**)w2);
}

char **findwordinlist(wordlist const *list, char const *word)
{
    return (char**)bsearch(&word, list->words, list->count,
			   sizeof(char*), wordcmp);
}

void sortwordlist(wordlist *list)
{
    qsort(list->words, list->count, sizeof(char*), wordcmp);
}

void strikewordfromlist(wordlist *list, int idx)
{
    assert(idx >= 0 && idx < list->count);
    memmove(list->words + idx, list->words + idx + 1,
	    (list->count - idx) * sizeof(char*));
    --list->count;
}

void destroywordlist(wordlist *list)
{
    char **pool, **nextpool;

    free(list->words);
    if (list->pools) {
	for (pool = (char**)*(char**)list->pools ; pool ; pool = nextpool) {
	    nextpool = (char**)*pool;
	    free(pool);
	}
    }
}
