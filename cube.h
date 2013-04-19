/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#ifndef	_cube_h_
#define	_cube_h_

/* The width, height and number of cells in the current grid.
 */
extern int width, height, gridsize;

/* The contents of the current grid.
 */
extern char *grid;

/* The lists of each cell's neighboring cells.
 */
extern short (*neighbors)[9];

/* Randomly initialize the contents of the grid.
 */
extern void shakecube(void);

/* Locate a word within the grid, returning a gridsized array that
 * contains a number for each cell that the word covers, the number
 * being a one-based index for the letter in the word that is
 * there. The array belongs to this module and is reused with each
 * call to the function. If the word cannot be found in the grid, NULL
 * is returned.
 */
extern char *findwordingrid(char const *word);

#endif
