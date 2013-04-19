/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdarg.h>
#include	"genutil.h"
#include	"dict.h"
#include	"dbuild.h"

/* The range of legal characters in our alphabet.
 */
#define	MAX_ALPHABET		256

/* An arc in an uncompressed dictionary graph.
 */
typedef struct treearc treearc;
struct treearc {
    long	    nodeid;	/* a unique identifier			*/
    treearc	   *sibling;	/* the next arc attached to this node	*/
    treearc	   *child;	/* the first child node			*/
    unsigned char   letter;	/* the arc's letter			*/
    char	    wordend;	/* TRUE if at the end of a word		*/
};

/* A node in an uncompressed dictionary graph.
 */
typedef struct treenode treenode;
struct treenode {
    arc		   *arcs;	/* the arcs leading from this node	*/
    arc		   *parcs;	/* a new, equivalent set of arcs	*/
    long	    group;	/* this node's partition group number	*/
    treenode	   *nextingrp;	/* the next node in the same partition	*/
    unsigned char   arccount;	/* the count of arcs at this node	*/
    unsigned char   wordend;	/* TRUE if a word ends at this node	*/
    unsigned char   grouped;	/* TRUE if this node is in a partition	*/
};

/* How many arcs to allocate at a time.
 */
static const int arcchunk = 8192;

/* The total number of arcs and nodes in the dictionary graph.
 */
static long arccount = 0, nodecount = 0;

/* The head of the linked list of arc pools.
 */
static treearc *arcpools = NULL;

/* The block of memory containing the dictionary graph.
 */
static treenode *wordtree = NULL;

/* The root node of the dictionary graph.
 */
static treenode *wordtreeroot = NULL;

/* The head of the linked list of arc set pools.
 */
static void *arcsetpool = NULL;

/* Letter frequencies in the dictionary.
 */
static long frequencies[MAX_ALPHABET];

/* Whether or not to display relatively useless data while compressing
 * the dictionary.
 */
static int verbose = TRUE;

/* The default wordlist if no files are supplied by the user.
 */
static char *defwordlist = "-";

/* Name and file pointer for the file currently open.
 */
static char *currfilename = NULL;
static FILE *currfp = NULL;

/* Macros for handling file errors.
 */
#define	errfile()	(perror(currfilename), exit(EXIT_FAILURE), NULL)
#define closefile()	(ferror(currfp) || fclose(currfp) ? (void*)errfile() :\
					(currfp = NULL, currfilename = NULL))

/* Outputs relatively useless data on stderr.
 */
static void report(char const *format, ...)
{
    if (verbose) {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	fflush(stdout);
    }
}

/* Returns zero if the given word is somehow invalid for use in a
 * game; otherwise, returns the word's length.
 */
static int checkover(char *word)
{
    char *r, *w;

    for (r = w = word ; *w && *w != '\n' ; ++w, ++r) {
	if (!islower(*w))
	    return 0;
	*r = *w;
	if (*w == 'q') {
	    if (w[1] != 'u')
		return 0;
	    else
		++w;
	}
    }
    *r = '\0';
    return r - word;
}

/* Returns an unused arc, allocating more arc pools as needed.
 */
static treearc *grabnewtreearc(unsigned char letter, int nodeid)
{
    static treearc *poolptr = NULL;
    static long arcalloced = 0;

    if (arccount == arcalloced) {
	poolptr = xmalloc(arcchunk * sizeof(treearc));
	memset(poolptr, 0, arcchunk * sizeof(treearc));
	arcalloced += arcchunk - 1;
	poolptr->sibling = arcpools;
	poolptr->child = poolptr + arcchunk;
	arcpools = poolptr++;
    }
    poolptr->sibling = poolptr->child = NULL;
    poolptr->letter = letter;
    poolptr->wordend = FALSE;
    poolptr->nodeid = nodeid;
    ++arccount;
    return poolptr++;
}

/* Reads through all the given files for words, and buids an
 * uncompressed and disorganized dictionary graph, in which the nodes
 * are represented as linked lists of arcs.
 */
static int readwordlists(int filecount, char *files[], treearc *root)
{
    treearc *tarc;
    char word[WORDBUFSIZ];
    unsigned char *ltr;
    int clear = FALSE;
    long count = 0;
    int usestdin;
    int i, n;

    memset(frequencies, 0, sizeof frequencies);
    for (i = 0 ; i < filecount ; ++i) {
	usestdin = files[i][0] == '-' && files[i][1] == '\0';
	if (usestdin) {
	    currfilename = "standard input";
	    currfp = stdin;
	} else {
	    currfilename = files[i];
	    if (!(currfp = fopen(files[i], "r")))
		errfile();
	}
	report("Reading words from %s ...\n", currfilename);
	while (fgets(word, sizeof word, currfp)) {
	    n = checkover(word);
	    if (n == sizeof word - 1) {
		fprintf(stderr, "Overlong word \"%s...\" rejected\n", word);
		clear = TRUE;
		continue;
	    }
	    if (clear || !n) {
		clear = FALSE;
		continue;
	    }
	    tarc = root;
	    for (ltr = (unsigned char*)word ; *ltr ; ++ltr)
		++frequencies[*ltr];
	    for (ltr = (unsigned char*)word ; *ltr ; ++ltr) {
		while (tarc->letter != *ltr && tarc->sibling)
		    tarc = tarc->sibling;
		if (tarc->letter != *ltr) {
		    if (tarc->letter)
			tarc = tarc->sibling = grabnewtreearc(*ltr, 0);
		    else
			tarc->letter = *ltr;
		}
		if (!tarc->child)
		    break;
		tarc = tarc->child;
	    }
	    while (*ltr++) {
		tarc->child = grabnewtreearc(*ltr, ++nodecount);
		tarc = tarc->child;
	    }
	    tarc->wordend = TRUE;
	    ++count;
	}
	if (usestdin) {
	    currfp = NULL;
	    currfilename = NULL;
	} else
	    closefile();
    }
    if (!count) {
	fprintf(stderr, "No words found - nothing to build.\n");
	return 0;
    }

    report("\n%d words, %d nodes, %d arcs\n", count, nodecount, arccount);
    return count;
}

/* Determine which letters are in our alphabet, and compute their
 * frequencies.
 */
static void computefrequencies(void)
{
    unsigned long total;
    long error, cutoff;
    double f;
    int i, n;

    sizealphabet = 0;
    total = 0;
    for (i = 0 ; i < MAX_ALPHABET ; ++i) {
	if (!frequencies[i])
	    continue;
	total += frequencies[i];
	++sizealphabet;
    }
    alphabet = xmalloc(sizealphabet * sizeof *alphabet);
    letterfreq = xmalloc(sizealphabet * sizeof *letterfreq);
    cutoff = total / 2;
    error = -cutoff;
    n = 0;
    for (i = 0 ; i < MAX_ALPHABET ; ++i) {
	if (!frequencies[i])
	    continue;
	alphabet[n] = (char)i;
	f = frequencies[i] * 32768.0;
	letterfreq[n] = f / total;
	error += f - ((double)frequencies[i] * total) + 0.25;
	while (error >= cutoff) {
	    ++letterfreq[n];
	    error -= total;
	}
	++n;
    }
}

/* Takes an uncompressed dictionary graph and reorganizes it, creating
 * the treenode structures, and making all of a node's arcs contiguous
 * in memory.
 */
static void serializetree(void)
{
    arc temparcs[MAX_ALPHABET];
    arc *arcset;
    treearc *arcpool, *tarc, *ta;
    treenode *node;
    unsigned char count;
    long i, n;

    currfilename = "temporary file";
    if (!(currfp = tmpfile()))
	errfile();

    for (arcpool = arcpools ; arcpool ; arcpool = arcpool->sibling) {
	for (tarc = arcpool + 1 ; tarc != arcpool->child ; ++tarc) {
	    if (!tarc->nodeid)
		continue;
	    if (fwrite(&tarc->nodeid, sizeof tarc->nodeid, 1, currfp) != 1)
		errfile();
	    if (fwrite(&tarc->wordend, sizeof tarc->wordend, 1, currfp) != 1)
		errfile();
	    count = 0;
	    if (tarc->letter) {
		memset(temparcs, 0, sizeof temparcs);
		for (ta = tarc ; ta ; ta = ta->sibling, ++count) {
		    n = ta->letter;
		    setarcletter(temparcs[n], n);
		    setarcnext(temparcs[n], ta->child->nodeid);
		}
		if (fwrite(&count, sizeof count, 1, currfp) != 1)
		    errfile();
		for (n = 1 ; n < MAX_ALPHABET ; ++n) {
		    if (temparcs[n])
			if (fwrite(&temparcs[n], sizeof(arc), 1, currfp) != 1)
			    errfile();
		}
	    } else {
		if (fwrite(&count, sizeof count, 1, currfp) != 1)
		    errfile();
	    }
	}
    }

    report("%ld bytes in temporary file.\n", ftell(currfp));

    rewind(currfp);
    for (arcpool = arcpools ; arcpool ; arcpool = tarc) {
	tarc = arcpool->sibling;
	free(arcpool);
    }

    report("Allocating %d bytes for the ordered tree.\n",
	   (nodecount + 2) * sizeof(treenode) +
		(nodecount - 1) * sizeof(arc) * 2);

    arcpools = NULL;
    wordtree = xmalloc((nodecount + 2) * sizeof(treenode));
    arcset = arcsetpool = xmalloc((nodecount - 1) * sizeof(arc) * 2);
    wordtreeroot = wordtree + 1;

    for (i = 0 ; i < nodecount ; ++i) {
	if (fread(&n, sizeof n, 1, currfp) != 1)
	    errfile();
	node = wordtree + n;
	if (fread(&count, sizeof count, 1, currfp) != 1)
	    errfile();
	node->wordend = count ? 1 : 0;
	if (fread(&count, sizeof count, 1, currfp) != 1)
	    errfile();
	node->arcs = arcset;
	arcset += count;
	node->parcs = arcset;
	arcset += count;
	node->arccount = count;
	if (count) {
	    if (fread(node->arcs, count * sizeof(arc), 1, currfp) != 1)
		errfile();
	}
    }

    closefile();
}

/* Create a compression scheme for a dictionary graph by finding the
 * minimum number of partition groups for the nodes, and assigning
 * each node to the appropriate group.
 */
static int partitiontree(void)
{
    treenode **gchart;
    treenode *node, *next;
    long groupcount, prevgroupcount;
    long i;

    gchart = xmalloc((nodecount + 1) * sizeof *gchart);
    wordtree[0].group = 0;
    prevgroupcount = 0;
    groupcount = 0;
    for (i = 1, node = wordtreeroot ; i <= nodecount ; ++i, ++node) {
	node->group = node->arccount * 2;
	if (node->wordend)
	    ++node->group;
	if (node->group > groupcount)
	    groupcount = node->group;
	node->grouped = FALSE;
	memcpy(node->parcs, node->arcs, node->arccount * sizeof(arc));
    }
    wordtree[nodecount + 1].group = 0;
    report("Compressing ... ");

    while (groupcount != prevgroupcount) {
	for (i = 1 ; i <= groupcount ; ++i)
	    gchart[i] = NULL;
	for (node = wordtreeroot ; node->group ; ++node) {
	    for (i = 0 ; i < node->arccount ; ++i)
		setarc(node->parcs[i], getarcletter(node->arcs[i]), FALSE,
		       wordtree[getarcnext(node->arcs[i])].group);
	}
	for (--node ; node->group ; --node) {
	    node->nextingrp = gchart[node->group];
	    gchart[node->group] = node;
	}
	prevgroupcount = groupcount;
	groupcount = 0;
	for (node = wordtreeroot ; node->group ; ++node) {
	    if (node->grouped) {
		node->grouped = FALSE;
		continue;
	    }
	    node->group = ++groupcount;
	    for (next = node->nextingrp ; next ; next = next->nextingrp) {
		if (!next->grouped && !memcmp(node->parcs, next->parcs,
					      node->arccount * sizeof(arc))) {
		    next->group = groupcount;
		    next->grouped = TRUE;
		}
	    }
	}
	report("%d ... ", groupcount - prevgroupcount);
    }

    report("done\n");
    free(gchart);
    return groupcount;
}

/* Write out a compressed dictionary graph, using only one node from
 * each partition group.
 */
static void writetreetofile(int groupcount)
{
    int *xtable, *groups;
    treenode *node;
    arc lastarc;
    long i, n, g;

    lastarc = 0;
    setarcend(lastarc, TRUE);
    xtable = xmalloc((groupcount + 1) * sizeof(int));
    groups = xmalloc((groupcount + 1) * sizeof(int));
    memset(groups, 0, (groupcount + 1) * sizeof(int));
    g = 0;
    for (n = 1, node = wordtreeroot ; n <= nodecount ; ++n, ++node) {
	if (node->wordend)
	    continue;
	i = node->group;
	if (!groups[i]) {
	    groups[i] = ++g;
	    xtable[g] = n;
	}
	node->group = groups[i];
    }
    dictfinalstates = g + 1;
    for (n = 1, node = wordtreeroot ; n <= nodecount ; ++n, ++node) {
	if (!node->wordend)
	    continue;
	i = node->group;
	if (!groups[i]) {
	    if (node->arccount) {
		groups[i] = ++g;
		xtable[g] = n;
	    } else {
		groups[i] = groupcount;
		xtable[groupcount] = n;
		node->arccount = 1;
		node->arcs = &lastarc;
	    }
	}
	node->group = groups[i];
    }

    groups[1] = 0;
    for (g = 1 ; g < groupcount ; ++g)
	groups[g + 1] = groups[g] + wordtree[xtable[g]].arccount;
    dictarccount = groups[groupcount] + wordtree[xtable[groupcount]].arccount;
    for (node = wordtreeroot ; node->group ; ++node)
	node->group = groups[node->group];
   dictfinalstates = groups[dictfinalstates];

    report("%lu bytes for the completed dictionary file.\n",
	   (unsigned long)(dictarccount * sizeof(arc) + fileheadsize()));

    currfilename = dictfilename;
    if (!(currfp = fopen(currfilename, "wb")))
	errfile();
    if (writefilehead(currfp))
	errfile();
    for (g = 1 ; g <= groupcount ; ++g) {
	node = wordtree + xtable[g];
	for (i = 0 ; i < node->arccount ; ++i)
	    setarc(node->arcs[i], getarcletter(node->arcs[i]), FALSE,
		   wordtree[getarcnext(node->arcs[i])].group);
	setarcend(node->arcs[i - 1], TRUE);
	if (writefilenodes(currfp, node->arcs, node->arccount))
	    errfile();
    }
    closefile();
}

/* Compile a compressed dictionary file from the words in the given
 * files.
 */
int makedictfile(int filecount, char *files[])
{
    if (!filecount) {
	filecount = 1;
	files = &defwordlist;
    }

    nodecount = 1;
    if (!readwordlists(filecount, files, grabnewtreearc(0, nodecount)))
	return EXIT_FAILURE;
    computefrequencies();
    serializetree();
    writetreetofile(partitiontree());

    report("Dictionary saved to %s\n", dictfilename);
    return EXIT_SUCCESS;
}
