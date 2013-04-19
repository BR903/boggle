/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<curses.h>
#include	"genutil.h"

/* Wrapper for malloc that aborts upon failure.
 */
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

/* Wrapper for realloc that aborts upon failure.
 */
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

/* Display a message on stderr and abort.
 */
void expire(char const *format, ...)
{
    if (format) {
	va_list	args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputc('\n', stderr);
    }
    fputs("Use boggle -h to view help.\n", stderr);
    exit(EXIT_FAILURE);
}
