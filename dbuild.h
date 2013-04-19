/* dbuild.h: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_dbuild_h_
#define	_dbuild_h_

/* Build a compressed dictionary file that contains the words listed
 * in the given files.
 */
extern int makedictfile(int filecount, char *files[]);

#endif
