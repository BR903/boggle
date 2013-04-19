/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#ifndef	_input_h_
#define	_input_h_

/* Gets keystrokes at the very start of the game session, returning
 * TRUE for space, FALSE for ^D, and ignoring anything else.
 */
extern int getstartinput(void);

/* Inputs a word, returning a pointer to a statically-allocated buffer
 * containing the word, or NULL if the user types ^D instead. If
 * enablehelp is TRUE, then help is displayed when ? is pressed.
 */
extern char const *inputword(int enablehelp);

/* Gets keystrokes at the end of each game, returning TRUE for &,
 * FALSE for ^D, dealing with ? by inputting a word and displaying its
 * location on the grid, and ignoring anything else.
 */
extern int getendgameinput(void);

#endif
