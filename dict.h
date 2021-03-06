/* dict.h: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_dict_h_
#define	_dict_h_

#include	<stdio.h>
#include	"genutil.h"

/* Default dictionary pathname if none is specified at compile-time.
 */
#ifndef DICTFILEPATH
#define	DICTFILEPATH		"boggle.dict"
#endif

/* An arc in the dictionary graph.
 */
typedef unsigned long		arc;

/* Macros to access and change a dictionary arc.
 */
#define	getarcletter(arc)		((int)((arc) >> 23) & 0xFF)
#define	getarcend(arc)			(((arc) & 0x80000000L) != 0)
#define	getarcnext(arc)			((long)((arc) & 0x007FFFFFL))

#define	setarcletter(arc, ltr)		((arc) |= ((ltr) & 0xFFUL) << 23)
#define	setarcend(arc, end)		((arc) |= (end) ? 0x80000000UL : 0)
#define	setarcnext(arc, next)		((arc) |= (next) & 0x007FFFFFUL)

#define	setarc(arc, ltr, end, next)	((arc) = ((next) & 0x007FFFFFUL)    \
					       | (((ltr) & 0xFFUL) << 23)   \
					       | ((end) ? 0x80000000UL : 0))

/* The pathname for the compressed dictionary.
 */
extern char *dictfilename;

/* The alphabet.
 */
extern char *alphabet;
extern int sizealphabet;

/* Letter frequencies for the current dictionary.
 */
extern int *letterfreq;
extern long freqdenom;

/* Pointer to the compressed dictionary.
 */
extern arc *dictionary;

/* The total number of arcs in the dictionary.
 */
extern long dictarccount;

/* The index of the first final state in the compressed dictionary.
 */
extern long dictfinalstates;

/* Read a dictionary file into memory. Returns zero on success.
 */
extern int readdictfile(void);

/* Write the dictionary file header out to the given file.
 */
extern int writefilehead(FILE *fp);

/* Write the given number of arcs out to the given file.
 */
extern int writefilenodes(FILE *fp, arc const *nodes, long count);

/* Return the size of the dictionary file.
 */
extern long dictfilesize(void);

#endif
