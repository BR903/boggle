/* boggle.c: Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<getopt.h>
#include	"genutil.h"
#include	"dbuild.h"
#include	"batch.h"
#include	"play.h"
#include	"boggle.h"

/* The version info.
 */
static char const *vourzhon =
	"boggle: version " VERSION "\n"
	"Copyright (C) 1999 by Brian Raiter <breadbox@muppetlabs.com>\n"
	"License GPLv2+: GNU GPL version 2 or later.\n"
	"This is free software; you are free to change and redistribute it.\n"
	"There is NO WARRANTY, to the extent permitted by law.\n";

/* The help info.
 */
static char const *yowzitch =
	"Usage: boggle [-45hlpsv] [-t secs] [-w size] [-d dict] [-b size]\n"
	"              [-B board] [-D wordlists]\n"
	"   -4  4x4 grid, 3-letter minimum\n"
	"   -5  5x5 grid, 4-letter minimum (the default)\n"
	"   -b  Specify custom grid size (one digit indicates a square grid)\n"
	"   -d  Specify filename of compiled dictionary\n"
	"   -h  Display this help\n"
	"   -l  Don't use VT100 line-drawing characters\n"
	"   -p  Postpone check for invalid words until after the game ends\n"
	"   -s  Don't display undiscovered words at end of game\n"
	"   -t  Specify time for each round (default is 180, or 3 minutes)\n"
	"   -w  Specify a minimum word size (default is either 4 or 3)\n"
	"   -v  Display version information\n"
	"   -B  Batch mode: takes a wordlist as input and outputs the words\n"
	"       that can be found in the board specified on the command line\n"
	"   -D  Compile a new dictionary from the specified wordlist files;\n"
	"       use -d to specify an alternate destination file.\n";

/* What part of the program to run.
 */
int mode = 0;

/* argv[0].
 */
char const *thisfile;

/* Parse the cmdline arguments and initialize the program. This
 * function returns FALSE to indicate that the program should exit
 * immediately without error.
 */
static int startup(int argc, char *argv[])
{
    char *opts[256];
    int n;

    thisfile = argv[0];

    if (argc == 2) {
	if (!strcmp(argv[1], "--help")) {
	    fputs(yowzitch, stdout);
	    return FALSE;
	} else if (!strcmp(argv[1], "--version")) {
	    fputs(vourzhon, stdout);
	    return FALSE;
	}
    }

    for (n = 0 ; n < (int)(sizeof opts / sizeof *opts) ; ++n)
	opts[n] = NULL;
    for (;;) {
	n = getopt(argc, argv, "45Bb:Dd:hlpst:vw:");
	if (n == ':' || n == '?')
	    expire(NULL);
	else if (n == EOF)
	    break;
	else if (n == 'h') {
	    fputs(yowzitch, stdout);
	    return FALSE;
	} else if (n == 'v') {
	    fputs(vourzhon, stdout);
	    return FALSE;
	} else if (n == 'B') {
	    if (mode)
		expire("Mutually exclusive options specified.");
	    mode = MODE_BATCH;
	} else if (n == 'D') {
	    if (mode)
		expire("Mutually exclusive options specified.");
	    mode = MODE_MAKEDICT;
	}
	opts[n] = optarg ? optarg : "";
    }
    if (!mode)
	mode = MODE_SOLITAIRE;

    return dictinit(opts)
	&& wordsinit(opts)
	&& cubeinit(opts)
	&& timerinit(opts)
	&& scoreinit(opts)
	&& outputinit(opts)
	&& inputinit(opts)
	&& playinit(opts);
}

/* Run the program.
 */
int main(int argc, char *argv[])
{
    if (!startup(argc, argv))
	return EXIT_SUCCESS;

    if (!(mode & MM_ARGS) && optind != argc)
	expire("Unrecongized argument - %s\n", argv[optind]);

    switch (mode) {
      case MODE_MAKEDICT:  return makedictfile(argc - optind, argv + optind);
      case MODE_BATCH:	   return filtergrid(argc - optind, argv + optind);
      case MODE_SOLITAIRE: return playsolitaire();
    }

    return EXIT_FAILURE;
}
