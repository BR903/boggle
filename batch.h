/* batch.h: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_batch_h_
#define	_batch_h_

/* Filter stdin to pass through only the lines that have words that
 * can be found in the grid given by concatenating the string
 * arguments together.
 */
extern int filtergrid(int count, char *pieces[]);

#endif
