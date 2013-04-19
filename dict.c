/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"dict.h"

/* The magic number for a compressed dictionary file.
 */
#define	DICTFILE_SIG		0xCBDF

/* The size of an index into the file's arc array in bytes.
 */
static int sizenodeindex;

/* Default dictionary pathname.
 */
char *dictfilename = NULL;

/* The alphabet.
 */
char *alphabet = NULL;
int sizealphabet = 0;

/* Letter frequencies for the current dictionary.
 */
int *letterfreq = NULL;
long freqdenom = 0;

/* Pointer to the compressed dictionary.
 */
arc *dictionary = NULL;

/* The total number of arcs in the dictionary.
 */
long dictarccount = 0;

/* The index of the first final state in the compressed dictionary.
 */
long dictfinalstates = 0;

/* Exit function for this module.
 */
static void destroy(void)
{
    free(alphabet);
    free(letterfreq);
    free(dictionary);
    free(dictfilename);
}

/* Function to read a byte at a time and deal with I/O errors.
 */
static unsigned char readbyte(FILE *fp)
{
    int n;

    errno = 0;
    if ((n = fgetc(fp)) == EOF) {
	if (errno)
	    expire("%s: %s", dictfilename, strerror(errno));
	else
	    expire("%s: not a valid dictionary file.", dictfilename);
    }
    return (unsigned char)n;
}

/* Function to write a byte at a time and deal with I/O errors.
 */
static void writebyte(FILE *fp, int n)
{
    errno = 0;
    if (fputc((unsigned char)n, fp) == EOF) {
	if (errno)
	    expire("%s: %s", dictfilename, strerror(errno));
	else
	    expire("%s: couldn't write to file.", dictfilename);
    }
}

/* Read in a complete dictionary from the selected file.
 */
int readdictfile(void)
{
    FILE *fp;
    arc *arcs;
    long j, temp;
    int i;

    if (!(fp = fopen(dictfilename, "rb")))
	expire("%s: %s", dictfilename, strerror(errno));
    i = readbyte(fp) << 8;
    i |= readbyte(fp);
    if (i != DICTFILE_SIG)
	goto fail;
    if (!(sizealphabet = (int)readbyte(fp)))
	goto fail;
    if (!(sizenodeindex = (int)readbyte(fp)))
	goto fail;
    alphabet = xmalloc(sizealphabet * sizeof *alphabet);
    letterfreq = xmalloc(sizealphabet * sizeof *letterfreq);
    freqdenom = 0;
    for (i = 0 ; i < sizealphabet ; ++i) {
	alphabet[i] = (char)readbyte(fp);
	letterfreq[i] = readbyte(fp);
	letterfreq[i] |= readbyte(fp) << 8;
	freqdenom += (long)letterfreq[i];
    }
    dictarccount = 0;
    for (i = 0 ; i < sizenodeindex ; ++i)
	dictarccount |= readbyte(fp) << i * 8;
    dictfinalstates = 0;
    for (i = 0 ; i < sizenodeindex ; ++i)
	dictfinalstates |= readbyte(fp) << i * 8;
    if (dictfinalstates >= dictarccount)
	goto fail;
    dictionary = xmalloc(dictarccount * sizeof *dictionary);
    for (j = 0, arcs = dictionary ; j < dictarccount ; ++j, ++arcs) {
	temp = 0;
	for (i = 0 ; i < sizenodeindex ; ++i)
	    temp |= readbyte(fp) << i * 8;
	i = (int)readbyte(fp);
	*arcs = 0;
	setarcnext(*arcs, temp);
	setarcend(*arcs, i & 0x80);
	i &= 0x7F;
	if (i > sizealphabet)
	    goto fail;
	else if (i < sizealphabet)
	    setarcletter(*arcs, alphabet[i]);
    }

    fclose(fp);
    return 0;

  fail:
    fclose(fp);
    sizealphabet = 0;
    dictarccount = 0;
    expire("%s: not a valid dictionary file", dictfilename);
    return -1;
}

/* The initialization function for this module. Parses the cmdline
 * option -d and reads the compressed dictionary file into memory.
 */
int dictinit(char *opts[])
{
    if (mode == MODE_BATCH)
	return TRUE;

    atexit(destroy);

    if (opts['d']) {
	dictfilename = xmalloc(strlen(opts['d']) + 1);
	strcpy(dictfilename, opts['d']);
    } else {
	dictfilename = xmalloc(sizeof DICTFILEPATH);
	strcpy(dictfilename, DICTFILEPATH);
    }

    if (mode == MODE_MAKEDICT)
	return TRUE;

    readdictfile();
    return TRUE;
}

/* Write a dictionary file header out to the given file.
 */
int writefilehead(FILE *fp)
{
    long temp;
    int i;

    sizenodeindex = 0;
    for (temp = 0 ; temp < dictarccount ; temp = (temp << 8) | 0xFF)
	++sizenodeindex;

    writebyte(fp, (DICTFILE_SIG >> 8) & 0xFF);
    writebyte(fp, DICTFILE_SIG & 0xFF);
    writebyte(fp, sizealphabet);
    writebyte(fp, sizenodeindex);
    for (i = 0 ; i < sizealphabet ; ++i) {
	writebyte(fp, alphabet[i]);
	writebyte(fp, letterfreq[i] & 0xFF);
	writebyte(fp, (letterfreq[i] >> 8) & 0xFF);
    }
    temp = dictarccount;
    for (i = 0 ; i < sizenodeindex ; ++i) {
	writebyte(fp, temp & 0xFF);
	temp >>= 8;
    }
    temp = dictfinalstates;
    for (i = 0 ; i < sizenodeindex ; ++i) {
	writebyte(fp, temp & 0xFF);
	temp >>= 8;
    }

    return 0;
}

/* Write the given number of arcs out to the given file.
 */
int writefilenodes(FILE *fp, arc const *arcs, long count)
{
    arc const *arc;
    unsigned long temp;
    long j;
    int i, ch;

    for (j = 0, arc = arcs ; j < count ; ++j, ++arc) {
	temp = getarcnext(*arc);
	for (i = 0 ; i < sizenodeindex ; ++i) {
	    writebyte(fp, temp & 0xFF);
	    temp >>= 8;
	}
	ch = getarcletter(*arc);
	if (ch) {
	    for (i = 0 ; i < sizealphabet ; ++i)
		if (ch == alphabet[i])
		    break;
	    if (i == sizealphabet)
		return -1;
	} else
	    i = sizealphabet;
	if (getarcend(*arc))
	    i |= 0x80;
	writebyte(fp, i);
    }

    return 0;
}

int fileheadsize(void)
{
    long temp;

    sizenodeindex = 0;
    for (temp = 0 ; temp < dictarccount ; temp = (temp << 8) | 0xFF)
	++sizenodeindex;

    return 2 + 1 + 1 + sizealphabet * 3 + sizenodeindex * 2;
}
