/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#ifndef	_batch_h_
#define	_batch_h_

/* Filter stdin to pass through only the lines that have words that
 * can be found in the grid given by concatenating the string
 * arguments together.
 */
extern int filtergrid(int count, char *pieces[]);

#endif
