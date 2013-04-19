/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#ifndef	_output_h_
#define	_output_h_

/* Moves the cursor to the top left of the area where words are
 * displayed, and erase the area if clear is TRUE.
 */
extern void movetowords(int clear);

/* Moves the cursor to the top left of the area where the game status
 * is displayed, and erase the area if clear is TRUE.
 */
extern void movetostatus(int clear);

/* Displays a line of text (using printf-style formatting) and move the
 * cursor down one line, using its current x-coordinate as the left
 * margin.
 */
extern void addline(char const *fmt, ...);

/* Displays the grid of letters for the current game.
 */
extern void drawgrid(void);

/* Sets up the screen for the start of a game.
 */
extern void displaygamestart(void);

/* Displays brief online help for the special keys during input if
 * show is TRUE, or erases it if show is FALSE.
 */
extern void displayinputhelp(int show);

/* Draw the board, the help, and the two wordlists at the end of a
 * round. highlit, if not NULL, indicates a set of letters in the
 * board to highlight. Offset indicates the number of columns to skip
 * over in the list of findable words. The return value is zero if the
 * end of screen was reached before the end of the findable wordlist.
 */
extern int doendgameoutput(int y, int x, char *highlit, int offset);

#endif
