/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#ifndef	_dict_h_
#define	_dict_h_

#include	"genutil.h"

/* Default dictionary pathname if none is specified at compile-time.
 */
#ifndef DICTFILEPATH
#define	DICTFILEPATH		"boggle.dict"
#endif

/* The first four bytes of a compressed dictionary file.
 */
#define	DICTFILE_SIG		0xCBDF

/* Sized integer types.
 */
typedef unsigned short	uint2bytes;
typedef	unsigned long	uint4bytes;

/* An arc in the dictionary graph.
 */
typedef	struct arc {
    uint4bytes	letter	: 8,	/* the letter attached to this arc	*/
		end	: 1,	/* indicates the last arc in the node	*/
	    	node	: 23;	/* index of the node this arc points to	*/
} arc;

/* The dictionary file header.
 */
typedef	struct dictfilehead {
    uint4bytes	sig;			/* equal to DICTFILE_SIG	*/
    uint2bytes	freq[SIZE_ALPHABET];	/* letter frequency ratios	*/
    uint4bytes	finalstates;		/* index of first end node	*/
    uint4bytes	size;			/* size of dictionary proper	*/
} dictfilehead;

/* The pathname for the compressed dictionary.
 */
extern char *dictfilename;

#endif
