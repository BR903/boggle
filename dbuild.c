#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdarg.h>
#include	<unistd.h>
#include	<time.h>
#include	"genutil.h"
#include	"words.h"
#include	"dbuild.h"

typedef	struct arc {
    int	letter	: 8,
	wordend	: 1,
    	node	: 23;
} arc;

typedef struct treearc treearc;
struct treearc {
    arc		arc;
    treearc    *sibling;
    treearc    *child;
};

typedef struct treenode treenode;
struct treenode {
    arc	       *arcs;
    arc	       *parcs;
    int		group;
    treenode   *nextingroup;
    int		arccount: 8,
		wordend	: 1,
		grouped	: 1;
};

static const int arcchunk = 8192;

static treearc *arctreeroot = NULL;
static treearc *arcpools = NULL;
static int arccount = 0;

static treenode *wordtree = NULL;
static treenode *wordtreeroot = NULL;
static int nodecount = 0;
static void *arcsetpool = NULL;

static int frequencies[26];

static int verbose = TRUE;

static char *defwordlist = "/usr/dict/words";

static char *currfilename = NULL;
static FILE *currfp = NULL;

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

static void report(char const *format, ...)
{
    if (verbose) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
    }
}

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

static treearc *grabnewtreearc(char letter, int nodeid)
{
    static treearc *poolptr = NULL;
    static int arcalloced = 0;

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

static int readwordlists(int filecount, char *files[])
{
    treearc *tarc;
    char word[83];
    char *ltr;
    int clear = FALSE;
    int count = 0;
    int i, n;

    for (i = 0 ; i < filecount ; ++i) {
	report("Reading words from %s ...\n", files[i]);
	currfilename = files[i];
	openfile(files[i], "r");
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
	    tarc = arctreeroot;
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
	    tarc->arc.wordend = TRUE;
	    ++count;
	}
	closefile();
    }
    if (!count) {
	fprintf(stderr, "No words found - nothing to build.\n");
	return 0;
    }

    report("\n%d words\n", count);
    return count;
}

static void serializetree(void)
{
    arc tempnode = { 0, 0, 0 };
    arc temparcs[128];
    arc *arcset;
    treearc *arcpool, *tarc;
    treenode *node;
    int n;

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
	    tempnode.wordend = tarc->arc.wordend;
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
	node->wordend = tempnode.wordend;
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

static int partitiongroups(void)
{
    treenode *node, *other;
    int i;
    int newgroupcount = 0;

    for (node = wordtreeroot ; node->group ; ++node)
	for (i = 0 ; i < node->arccount ; ++i)
	    node->parcs[i].node = wordtree[node->arcs[i].node].group;

    for (node = wordtreeroot ; node->group ; ++node) {
	if (node->grouped) {
	    node->grouped = 0;
	    continue;
	}
	i = node->group;
	node->group = ++newgroupcount;
	for (other = node + 1 ; other->group ; ++other) {
	    if (!other->grouped
			&& other->group == i
			&& other->arccount == node->arccount
			&& !memcmp(node->parcs, other->parcs,
				   node->arccount * sizeof(arc))) {
		other->group = newgroupcount;
		other->grouped = TRUE;
	    }
	}
    }

    return newgroupcount;
}

void compresstree(void)
{
    int count, newcount;
    int n;
    time_t t, t0;

    for (n = 1 ; n <= nodecount ; ++n) {
	wordtree[n].group = wordtree[n].wordend ? 2 : 1;
	wordtree[n].grouped = 0;
	memcpy(wordtree[n].parcs, wordtree[n].arcs,
				  wordtree[n].arccount * sizeof(arc));
    }
    wordtree[nodecount + 1].group = 0;

    t0 = time(NULL);
    report("Compressing ... ");
    count = 2;
    newcount = partitiongroups();
    while (newcount != count) {
	t = time(NULL);
	report("%ds:", t - t0);
	t0 = t;
	report("%d ... ", newcount);
	count = newcount;
	newcount = partitiongroups();
    }
    report("done\n");
}

static int partitiontree(void)
{
    treenode **gchart;
    treenode *node, *next;
    int i;
    int groupcount, prevgroupcount;

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

static void writetreetofile(int groupcount)
{
    int *xtable, *groups;
    treenode *node;
    arc lastarc = { 0, TRUE, 0 };
    unsigned short sig = 0xCBDF;
    int firstwordend, dictsize;
    int i, n, g;

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
    firstwordend = g + 1;
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
    dictsize = groups[groupcount] + wordtree[xtable[groupcount]].arccount;
    dictsize *= sizeof(arc);
    for (node = wordtreeroot ; node->group ; ++node)
	node->group = groups[node->group];
    firstwordend = groups[firstwordend];

    report("First final node: %d\n", firstwordend);
    report("%d bytes for the completed dictionary file.\n", dictsize + 10);

    openfile(dictfilename, "wb");
    writeobj(sig);
    writeobjs(frequencies, 26);
    writeobj(firstwordend);
    writeobj(dictsize);
    for (g = 1 ; g <= groupcount ; ++g) {
	node = wordtree + xtable[g];
	for (i = 0 ; i < node->arccount ; ++i)
	    node->arcs[i].node = wordtree[node->arcs[i].node].group;
	node->arcs[node->arccount - 1].wordend = TRUE;
	writeobjs(node->arcs, node->arccount);
    }
    closefile();
}

int makedictfile(int filecount, char *files[])
{
    int n;

    arctreeroot = grabnewtreearc('\0', 1);
    nodecount = 1;

    if (filecount)
	n = readwordlists(filecount, files);
    else
	n = readwordlists(1, &defwordlist);
    if (!n)
	return EXIT_FAILURE;
    report("%d nodes\n%d arcs\n", nodecount, arccount);

    serializetree();
    n = partitiontree();
    writetreetofile(n);

    report("Dictionary saved to %s\n", dictfilename);
    return EXIT_SUCCESS;
}
