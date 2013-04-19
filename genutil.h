/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#ifndef	_genutil_h_
#define	_genutil_h_

#ifndef TRUE
#define TRUE	1
#define	FALSE	0
#endif

/* Our alphabet.
 */
#define	ALPHABET		"abcdefghijklmnopqrstuvwxyz"
#define	SIZE_ALPHABET		((int)sizeof(ALPHABET) - 1)

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
