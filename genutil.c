#include	<stdio.h>
#include	<stdlib.h>
#include	<ncurses.h>
#include	"genutil.h"

void *xmalloc(size_t size)
{
    void *mem;

    mem = malloc(size);
    if (!mem) {
	if (!isendwin())
	    endwin();
	fputs("Out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }
    return mem;
}

void *xrealloc(void *mem, size_t size)
{
    mem = realloc(mem, size);
    if (!mem) {
	if (!isendwin())
	    endwin();
	fputs("Out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }
    return mem;
}

void expire(char const *msg)
{
    if (msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
    }
    fputs("Use boggle -h to view help.\n", stderr);
    exit(EXIT_FAILURE);
}
