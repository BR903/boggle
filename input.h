/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#ifndef	_input_h_
#define	_input_h_

/* Gets keystrokes at the very start of the game session, returning
 * TRUE for space, FALSE for ^D, and ignoring anything else.
 */
extern int getstartinput(void);

/* Move the cursor to successive lines with a dynamically-sized set of
 * columns. If size is negative, the current cursor position is used as
 * the top of the first column. If size is zero, the current line is
 * erased. Otherwise, the cursor is moved to the next line, or to the
 * top of the next column if the current column is full.
 */
void addtocolumn(int size);

/* Inputs a word, returning a pointer to a statically-allocated buffer
 * containing the word, or NULL if the user types ^D instead. If
 * enablehelp is TRUE, then help is displayed when ? is pressed.
 */
extern char *inputword(int enablehelp);

/* Gets keystrokes at the end of each game, returning TRUE for &,
 * FALSE for ^D, shifting the wordlist display on - and +/=, dealing
 * with ? by inputting a word and displaying its location on the grid,
 * and ignoring anything else.
 */
extern int doendgameinputloop(void);

#endif
