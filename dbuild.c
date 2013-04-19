/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdarg.h>
#include	"genutil.h"
#include	"dict.h"
#include	"dbuild.h"

/* An arc in an uncompressed dictionary graph.
 */
typedef struct treearc treearc;
struct treearc {
    arc		arc;		/* the essential arc data		*/
    treearc    *sibling;	/* the next arc attached to this node	*/
    treearc    *child;		/* the first child node			*/
};

/* A node in an uncompressed dictionary graph.
 */
typedef struct treenode treenode;
struct treenode {
    arc	       *arcs;		/* the arcs leading from this node	*/
    arc	       *parcs;		/* a new, equivalent set of arcs	*/
    long	group;		/* this node's partition group number	*/
    treenode   *nextingroup;	/* the next node in the same partition	*/
    int		arccount: 8,	/* the count of arcs at this node	*/
		wordend	: 1,	/* TRUE if a word ends at this node	*/
		grouped	: 1;	/* TRUE if this node is in a partition	*/
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

/* Letter frequencies for the dictionary.
 */
static long frequencies[SIZE_ALPHABET];

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

/* Macros for abbreviating file I/O and handling errors.
 */
#define	openfile(name, mode)						\
    ((currfp = fopen((currfilename = (name)), (mode))) ?		\
	TRUE : (perror(currfilename), exit(EXIT_FAILURE)))
#define readobj(obj)							\
    (fread(&(obj), sizeof(obj), 1, currfp) == 1 ?			\
	TRUE : (perror(currfilename), exit(EXIT_FAILURE)))
#define	readobjs(obj, n)						\
    (fread((obj), sizeof(*(obj)), (n), currfp) == (size_t)(n) ?		\
	TRUE : (perror(currfilename), exit(EXIT_FAILURE)))
#define writeobj(obj)							\
    (fwrite(&(obj), sizeof(obj), 1, currfp) == 1 ?			\
	TRUE : (perror(currfilename), exit(EXIT_FAILURE)))
#define	writeobjs(obj, n)						\
    (fwrite((obj), sizeof(*(obj)), (n), currfp) == (size_t)(n) ?	\
	TRUE : (perror(currfilename), exit(EXIT_FAILURE)))
#define closefile()							\
    ((ferror(currfp) || fclose(currfp)) ?				\
	(perror(currfilename), exit(EXIT_FAILURE)) :			\
	(void)(currfp = NULL, currfilename = NULL))

/* Outputs relatively useless data on stderr.
 */
static void report(char const *format, ...)
{
    if (verbose) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
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
static treearc *grabnewtreearc(char letter, int nodeid)
{
    static treearc *poolptr = NULL;
    static long arcalloced = 0;

    if (arccount == arcalloced) {
	poolptr = xmalloc(arcchunk * sizeof(treearc));
	memset(poolptr, 0, arcchunk * sizeof(treearc));
	arcalloced += arcchunk - 1;
	poolptr->sibling = arcpools;
	poolptr->child = poolptr + arcchunk;
	poolptr->arc.node = 0;
	arcpools = poolptr++;
    }
    poolptr->arc.letter = letter;
    poolptr->arc.node = nodeid;
    poolptr->sibling = poolptr->child = NULL;
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

    for (i = 0 ; i < filecount ; ++i) {
	usestdin = files[i][0] == '-' && files[i][1] == '\0';
	if (usestdin) {
	    currfilename = "standard input";
	    currfp = stdin;
	} else {
	    currfilename = files[i];
	    openfile(files[i], "r");
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
	    for (ltr = word ; *ltr ; ++ltr)
		++frequencies[*ltr - 'a'];
	    for (ltr = word ; *ltr ; ++ltr) {
		while (tarc->arc.letter != *ltr && tarc->sibling)
		    tarc = tarc->sibling;
		if (tarc->arc.letter != *ltr) {
		    if (tarc->arc.letter)
			tarc = tarc->sibling = grabnewtreearc(*ltr, 0);
		    else
			tarc->arc.letter = *ltr;
		}
		if (!tarc->child)
		    break;
		tarc = tarc->child;
	    }
	    while (*ltr++)
		tarc = tarc->child = grabnewtreearc(*ltr, ++nodecount);
	    tarc->arc.end = TRUE;
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

static void computefrequencies(void)
{
    unsigned long total;
    long error, cutoff;
    double f;
    int i;

    total = 0;
    for (i = 0 ; i < (int)(sizeof frequencies / sizeof *frequencies) ; ++i)
	total += frequencies[i];
    cutoff = total / 2;
    error = -cutoff;
    for (i = 0 ; i < (int)(sizeof frequencies / sizeof *frequencies) ; ++i) {
	f = frequencies[i] * 32768.0;
	frequencies[i] = f / total;
	error += f - ((double)frequencies[i] * total) + 0.25;
	while (error >= cutoff) {
	    ++frequencies[i];
	    error -= total;
	}
    }
}

/* Takes an uncompressed dictionary graph and reorganizes it, creating
 * the treenode structures, and making all of a node's arcs contiguous
 * in memory.
 */
static void serializetree(void)
{
    arc tempnode = { 0, 0, 0 };
    arc temparcs[128];
    arc *arcset;
    treearc *arcpool, *tarc;
    treenode *node;
    long n;

    currfilename = "temporary file";
    currfp = tmpfile();
    if (!currfp) {
	perror("Cannot create temporary file");
	exit(EXIT_FAILURE);
    }

    for (arcpool = arcpools ; arcpool ; arcpool = arcpool->sibling) {
	for (tarc = arcpool + 1 ; tarc != arcpool->child ; ++tarc) {
	    if (!tarc->arc.node)
		continue;
	    tempnode.node = tarc->arc.node;
	    tempnode.end = tarc->arc.end;
	    writeobj(tempnode);
	    if (tarc->arc.letter) {
		treearc *ta;
		memset(temparcs, 0, sizeof temparcs);
		n = 0;
		for (ta = tarc ; ta ; ta = ta->sibling, ++n) {
		    temparcs[ta->arc.letter].letter = ta->arc.letter;
		    temparcs[ta->arc.letter].node = ta->child->arc.node;
		}
		writeobj(n);
		for (n = 1 ; n < 128 ; ++n)
		    if (temparcs[n].node)
			writeobj(temparcs[n]);
	    } else {
		n = 0;
		writeobj(n);
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
	   (nodecount + 1) * sizeof(treenode) +
		(nodecount - 1) * sizeof(arc) * 2);

    arcpools = NULL;
    wordtree = xmalloc((nodecount + 2) * sizeof(treenode));
    arcset = arcsetpool = xmalloc((nodecount - 1) * sizeof(arc) * 2);
    wordtreeroot = wordtree + 1;

    for (n = 0 ; n < nodecount ; ++n) {
	readobj(tempnode);
	node = wordtree + tempnode.node;
	readobj(node->arccount);
	node->wordend = tempnode.end;
	node->arcs = arcset;
	arcset += node->arccount;
	node->parcs = arcset;
	arcset += node->arccount;
	if (!node->arccount)
	    continue;
	readobjs(node->arcs, node->arccount);
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
    long i;
    long groupcount, prevgroupcount;

    gchart = xmalloc((nodecount + 1) * sizeof(treenode*));
    wordtree[0].group = 0;
    for (i = 1, node = wordtreeroot ; i <= nodecount ; ++i, ++node) {
	node->group = node->wordend ? 2 : 1;
	node->grouped = FALSE;
	memcpy(node->parcs, node->arcs, node->arccount * sizeof(arc));
    }
    wordtree[nodecount + 1].group = 0;
    groupcount = 2;
    prevgroupcount = 0;
    report("Compressing ... ");

    while (groupcount != prevgroupcount) {
	report("%d ... ", groupcount);
	for (i = 1 ; i <= groupcount ; ++i)
	    gchart[i] = NULL;
	for (node = wordtreeroot ; node->group ; ++node)
	    for (i = 0 ; i < node->arccount ; ++i)
		node->parcs[i].node = wordtree[node->arcs[i].node].group;
	for (--node ; node->group ; --node) {
	    node->nextingroup = gchart[node->group];
	    gchart[node->group] = node;
	}
	prevgroupcount = groupcount;
	groupcount = 0;
	for (node = wordtreeroot ; node->group ; ++node) {
	    if (node->grouped) {
		node->grouped = FALSE;
		continue;
	    }
	    i = node->group;
	    node->group = ++groupcount;
	    for (next = node->nextingroup ; next ; next = next->nextingroup) {
		if (!next->grouped && next->arccount == node->arccount
				   && !memcmp(node->parcs, next->parcs,
					      node->arccount * sizeof(arc))) {
		    next->group = groupcount;
		    next->grouped = TRUE;
		}
	    }
	}
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
    arc lastarc = { 0, TRUE, 0 };
    dictfilehead header = { DICTFILE_SIG };
    long i, n, g;

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
    header.finalstates = g + 1;
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
    header.size = groups[groupcount] + wordtree[xtable[groupcount]].arccount;
    header.size *= sizeof(arc);
    for (node = wordtreeroot ; node->group ; ++node)
	node->group = groups[node->group];
    header.finalstates = groups[header.finalstates];

    report("%lu bytes for the completed dictionary file.\n",
	   sizeof header + header.size);

    openfile(dictfilename, "wb");
    for (i = 0 ; i < (int)(sizeof frequencies / sizeof *frequencies) ; ++i)
	header.freq[i] = frequencies[i];
    writeobj(header);
    for (g = 1 ; g <= groupcount ; ++g) {
	node = wordtree + xtable[g];
	for (i = 0 ; i < node->arccount ; ++i)
	    node->arcs[i].node = wordtree[node->arcs[i].node].group;
	node->arcs[node->arccount - 1].end = TRUE;
	writeobjs(node->arcs, node->arccount);
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
    if (!readwordlists(filecount, files, grabnewtreearc('\0', nodecount)))
	return EXIT_FAILURE;
    computefrequencies();
    serializetree();
    writetreetofile(partitiontree());

    report("Dictionary saved to %s\n", dictfilename);
    return EXIT_SUCCESS;
}
