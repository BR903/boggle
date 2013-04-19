#ifndef	_wordlist_h_
#define	_wordlist_h_

typedef	struct wordlist {
    char      **words;
    int		count;
    int		alloced;
    char       *pools;
    char       *poolptr;
    char       *poolstop;
} wordlist;

#define EMPTYWORDLIST	{ NULL, 0, 0, NULL, NULL, NULL }

extern void initwordlist(wordlist *list);
extern void addwordtolist(wordlist *list, char const *word);
extern char **findwordinlist(wordlist const *list, char const *word);
extern void strikewordfromlist(wordlist *list, int idx);
extern void sortwordlist(wordlist *list);
extern void destroywordlist(wordlist *list);

#define emptywordlist(list)	(destroywordlist(list), initwordlist(list))

#endif
