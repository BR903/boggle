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

/* Lists all the words in the given array at the current cursor
 * location, using multiple columns if necessary, with the given
 * heading at the top.
 */
extern void listwords(char const *heading, char const **words);

/* Displays the grid of letters for the current game.
 */
extern void drawgrid(void);

/* Displays just the letters in the grid. If highlighting is not NULL,
 * then for each value in highlighting that is nonzero, the
 * corresponding letter in the grid is highlighted.
 */
extern void drawgridletters(char const *highlighting);

/* Displays brief online help for the special keys during input if
 * show is TRUE, or erases it if show is FALSE.
 */
extern void displayinputhelp(int show);

#endif
