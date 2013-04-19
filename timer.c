#include	<stdlib.h>
#include	<time.h>
#include	<signal.h>
#include	<ncurses.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"output.h"
#include	"timer.h"

static int runtime = 180;
static time_t timelimit = 0;

static struct sigaction pausing, prevtstp;

void onpause(int sig)
{
    sigset_t mask, prevmask;

    pausetimer(TRUE);
    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_UNBLOCK, &mask, &prevmask);
    sigaction(sig, &prevtstp, NULL);
    endwin();
    raise(sig);
    clearok(stdscr, TRUE);
    sigaction(sig, &pausing, NULL);
    sigprocmask(SIG_SETMASK, &prevmask, NULL);
    pausetimer(FALSE);
    refresh();
}

int timerinit(char *opts[])
{
    pausing.sa_handler = onpause;
    sigemptyset(&pausing.sa_mask);
    pausing.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &pausing, &prevtstp);

    srand((unsigned)time(NULL));

    if (opts['t']) {
	runtime = atoi(opts['t']);
	if (runtime <= 0)
	    expire("Invalid time value");
    }

    return TRUE;
}

void starttimer(int t)
{
    halfdelay(10);
    timelimit = time(NULL) + (t ? t : runtime);
}

int runtimer(void)
{
    int	y, x, t;

    if (!timelimit)
	return TRUE;
    t = timelimit - time(NULL);
    if (t < 0)
	return FALSE;
    getyx(stdscr, y, x);
    movetostatus(FALSE);
    printw("%2d:%02d", t / 60, t % 60);
    move(y, x);
    refresh();
    return TRUE;
}

void pausetimer(int pause)
{
    static int timeleft = 0;

    if (pause) {
	if (timelimit) {
	    timeleft = stoptimer();
	    if (timeleft <= 0)
		timeleft = 1;
	}
    } else {
	if (timeleft) {
	    starttimer(timeleft);
	    timeleft = 0;
	}
    }
}

int stoptimer(void)
{
    int t;

    t = timelimit - time(NULL);
    timelimit = 0;
    cbreak();
    return t;
}
