/* dcat.c: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <stdlib.h>

/*
 * Definitions copied from dict.h and dict.c
 */

#ifndef DICTFILEPATH
#define DICTFILEPATH		"boggle.dict"
#endif

#define	DICTFILE_SIG		0xCBDF

typedef unsigned long		arc;

#define	getarcletter(arc)	((int)((arc) >> 23) & 0xFF)
#define	getarcend(arc)		((int)((arc) & 0x80000000L))
#define	getarcnext(arc)		((long)((arc) & 0x007FFFFFL))
#define	setarcletter(arc, ltr)	((arc) |= ((ltr) & 0xFFUL) << 23)
#define	setarcend(arc, end)	((arc) |= (end) ? 0x80000000UL : 0)
#define	setarcnext(arc, next)	((arc) |= (next) & 0x007FFFFFUL)

/* A structure containing a complete dictionary.
 */
struct dictionary {
    int	    sizealphabet;
    char    alphabet[256];
    long    finalstates;
    arc	   *root;
};

/* The name of the dictionary file.
 */
static char *dictfilename;

/* Function to read a byte at a time and deal with I/O errors.
 */
static unsigned char readbyte(FILE *fp)
{
    int ch;

    if ((ch = fgetc(fp)) == EOF) {
	perror(dictfilename);
	exit(EXIT_FAILURE);
    }
    return (unsigned char)ch;
}

/* Read in a complete dictionary from the selected file.
 */
int readdictfile(struct dictionary *d)
{
    FILE *fp;
    arc *arcs;
    long arccount, j, index;
    int sizeindex, i;

    if (!(fp = fopen(dictfilename, "rb"))) {
	perror(dictfilename);
	exit(EXIT_FAILURE);
    }
    i = readbyte(fp) << 8;
    i |= readbyte(fp);
    if (i != DICTFILE_SIG)
	goto fail;
    if (!(d->sizealphabet = (int)readbyte(fp)))
	goto fail;
    if (!(sizeindex = (int)readbyte(fp)))
	goto fail;
    for (i = 0 ; i < d->sizealphabet ; ++i) {
	d->alphabet[i] = (char)readbyte(fp);
	readbyte(fp);
	readbyte(fp);
    }

    arccount = 0;
    for (i = 0 ; i < sizeindex ; ++i)
	arccount |= readbyte(fp) << i * 8;
    d->finalstates = 0;
    for (i = 0 ; i < sizeindex ; ++i)
	d->finalstates |= readbyte(fp) << i * 8;
    if (d->finalstates >= arccount)
	goto fail;

    d->root = malloc(arccount * sizeof *d->root);
    if (!d->root) {
	fprintf(stderr, "Insufficient memory.\n");
	exit(EXIT_FAILURE);
    }

    for (j = 0, arcs = d->root ; j < arccount ; ++j, ++arcs) {
	index = 0;
	for (i = 0 ; i < sizeindex ; ++i)
	    index |= readbyte(fp) << i * 8;
	*arcs = 0;
	setarcnext(*arcs, index);
	i = (int)readbyte(fp);
	setarcend(*arcs, i & 0x80);
	i &= 0x7F;
	if (i > d->sizealphabet)
	    goto fail;
	else if (i < d->sizealphabet)
	    setarcletter(*arcs, d->alphabet[i]);
    }

    fclose(fp);
    return 0;

  fail:
    fclose(fp);
    fprintf(stderr, "%s: not a valid dictionary file", dictfilename);
    return -1;
}

/* Recursive function to retrieve the complete contents of the
 * compressed dictionary.
 */
static void retrieveall(struct dictionary *d, long idx, char *word, int len,
			void (*func)(const char*))
{
    arc	*a;

    a = d->root + idx;
    for (;;) {
	if ((word[len] = getarcletter(*a))) {
	    if (getarcnext(*a) >= d->finalstates)
		(*func)(word);
	    retrieveall(d, getarcnext(*a), word, len + 1, func);
	}
	if (getarcend(*a))
	    break;
	++a;
    }
    word[len] = '\0';
}

/* Simple callback for retrieveall().
 */
static void putword(const char *word)
{
    while (*word) {
	putchar(*word);
	if (*word == 'q')
	    putchar('u');
	++word;
    }
    putchar('\n');
}

/* Print out the contents of a dictionary file.
 */
int main(int argc, char *argv[])
{
    char word[256] = "";
    struct dictionary dict;

    if (argc > 2) {
	fprintf(stderr, "Usage: %s [dictfile]\n", argv[0]);
	return EXIT_FAILURE;
    } else if (argc == 2)
	dictfilename = argv[1];
    else
	dictfilename = DICTFILEPATH;

    if (readdictfile(&dict))
	return EXIT_FAILURE;

    retrieveall(&dict, 0, word, 0, putword);

    free(dict.root);
    return EXIT_SUCCESS;
}
