/* genutil.h: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_genutil_h_
#define	_genutil_h_

#ifndef TRUE
#define TRUE	1
#define	FALSE	0
#endif

/* The maximum size needed for word buffers.
 */
#define	WORDBUFSIZ	82

/* Wrapper for malloc that aborts upon failure.
 */
extern void *xmalloc(size_t size);

/* Wrapper for realloc that aborts upon failure.
 */
extern void *xrealloc(void *mem, size_t size);

/* Display a message on stderr and abort.
 */
extern void expire(char const *msg, ...);

#endif
