/* cube.h: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
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

/* Locate a word within the current grid. The return value is the
 * length of the word, or zero if the word cannot be found. If the
 * word is found, and positions is not NULL, then positions is set to
 * point to an array of char values. Each value in the array gives the
 * position of that letter on the grid for the given word. (The buffer
 * must be freed by the caller.)
 */
extern int findwordingrid(char const *word, char **positions);

#endif
