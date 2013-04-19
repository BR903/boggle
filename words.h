/* words.h: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_words_h_
#define	_words_h_

/* The minimum length of words to be found.
 */
extern int minlen;

/* Returns the array of words that can be found in the current game.
 */
extern char const **getfindable(void);

/* Returns the array of words that have already been found in the
 * current game.
 */
extern char const **getfound(void);

/* Marks a word as having being found. If check is nonzero, then the
 * function checks to see if the word can be found. If so, it is
 * removed from the list of finable words. If not, the function
 * returns zero.
 */
extern int acceptword(char const *word, int check);

/* Removes all words from the found wordlist that are not in the
 * findable wordlist, and then removes all words from the findable
 * wordlist that are in the found wordlist.
 */
extern void filterfound(void);

/* Compiles the findable wordlist (and empties the found wordlist).
 */
extern int findall(void);

#endif
