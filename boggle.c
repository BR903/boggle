#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	"genutil.h"
#include	"output.h"
#include	"dbuild.h"
#include	"batch.h"
#include	"play.h"
#include	"boggle.h"

static const char *vourzhon = "Boggle version 0.9, written by Brian Raiter\n";
static const char *yowzitch =
	"Usage: boggle [-45hpsv] [-t secs] [-w size] [-d dict] [-b size]\n"
	"              [-B board] [-D wordlists]\n"
	"   -4  4x4 grid, 3-letter minimum\n"
	"   -5  5x5 grid, 4-letter minimum (the default)\n"
	"   -b  Specify custom grid size (one digit indicates a square grid)\n"
	"   -d  Specify filename of compiled dictionary\n"
	"   -h  Display this help\n"
	"   -p  Postpone check for invalid words until after the game ends\n"
	"   -s  Don't display undiscovered words at end of game\n"
	"   -t  Specify time for each round (default is 180, or 3 minutes)\n"
	"   -w  Specify a minimum word size (default is either 4 or 3)\n"
	"   -v  Display version information\n"
	"   -B  Batch mode: takes a wordlist as input and outputs the words\n"
	"       that can be found in the board specified on the command line\n"
	"   -D  Compile a new dictionary from the specified wordlist files;\n"
	"       use -d to specify an alternate destination file.\n";

int mode = MODE_SOLITAIRE;
char const *thisfile;

static int startup(int argc, char *argv[])
{
    char *opts[256];
    int n;

    thisfile = argv[0];

    for (n = 0 ; n < (int)(sizeof opts / sizeof *opts) ; ++n)
	opts[n] = NULL;
    for (;;) {
	n = getopt(argc, argv, "45Bb:Dd:hpst:vw:x");
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
	}
	opts[n] = optarg ? optarg : "";
    }

    return wordsinit(opts)
	&& cubeinit(opts)
	&& timerinit(opts)
	&& scoreinit(opts)
	&& outputinit(opts)
	&& inputinit(opts)
	&& playinit(opts);
}

int main(int argc, char *argv[])
{
    if (!startup(argc, argv))
	return EXIT_SUCCESS;

    if (!(mode & MM_ARGS) && optind != argc) {
	fprintf(stderr, "Unrecongized argument - %s\n", argv[optind]);
	expire(NULL);
    }

    switch (mode) {
      case MODE_MAKEDICT:  return makedictfile(argc - optind, argv + optind);
      case MODE_BATCH:	   return filtergrid(argc - optind, argv + optind);
      case MODE_SOLITAIRE: return playsolitaire();
    }

    return EXIT_FAILURE;
}
