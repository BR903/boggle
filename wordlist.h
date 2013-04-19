/* wordlist.h: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_wordlist_h_
#define	_wordlist_h_

/* A structure for managing a variable-length array of strings. The
 * design is for a collection of strings that are very short, for
 * which it would be wasteful to malloc each separately, and for which
 * the emphasis is on building up the collection and then organizing
 * it, rather than being able to efficiently alter the collection or
 * the strings. "Pools" of memory are allocated, and into these the
 * strings are crammed back-to-back. Each pool contains, at top, a
 * pointer to the next pool.
 */
typedef	struct wordlist {
    char      **words;		/* the array of string pointers		*/
    int		count;		/* the size of the array		*/
    int		alloced;	/* the allocated size of the array	*/
    char       *pools;		/* the first allocated pool		*/
    char       *poolptr;	/* the beginning of the latest pool	*/
    char       *poolstop;	/* the end of the latest pool		*/
} wordlist;

/* Initializes a wordlist structure and allocates the first pool.
 */
extern void initwordlist(wordlist *list);

/* Appends a new word to the end of a wordlist.
 */
extern void addwordtolist(wordlist *list, char const *word);

/* Locates a word in a sorted wordlist, returning a pointer to the
 * word's string pointer within the array.
 */
extern char **findwordinlist(wordlist const *list, char const *word);

/* Removes the word at the given position from a wordlist.
 */
extern void strikewordfromlist(wordlist *list, int idx);

/* Sorts a wordlist.
 */
extern void sortwordlist(wordlist *list);

/* Frees all memory associated with a wordlist.
 */
extern void destroywordlist(wordlist *list);

/* Makes an already-initialized wordlist empty again.
 */
#define emptywordlist(list)	(destroywordlist(list), initwordlist(list))

#endif
