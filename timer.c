/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#include	<stdlib.h>
#include	<time.h>
#include	<signal.h>
#include	<curses.h>
#include	"boggle.h"
#include	"genutil.h"
#include	"output.h"
#include	"timer.h"

#ifdef SIGTSTP
/* Saved signal configurations.
 */
static struct sigaction pausing, prevtstp;
#endif	/* SIGTSTP */

/* How long to run the timer by default.
 */
static int runtime = 180;

/* When the timer was started.
 */
static time_t timestart = -1;

/* How many seconds to run timer.
 */
static int timetorun = 0;

#ifdef SIGTSTP
/* Signal handler for SIGTSTP. Suspends the timer (if running) and
 * curses' control of the screen, restores the original SIGTSTP handler,
 * and re-raises the signal, allowing curses to handle it. When the
 * program is restarted, replaces the signal handler and restarts curses
 * and the timer.
 */
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
#endif	/* SIGTSTP */

/* The initialization function for this module. Parses the cmdline
 * option -t and seeds the random number generator.
 */
int timerinit(char *opts[])
{
#ifdef SIGTSTP
    pausing.sa_handler = onpause;
    sigemptyset(&pausing.sa_mask);
    pausing.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &pausing, &prevtstp);
#endif	/* SIGTSTP */

    srand((unsigned)time(NULL));

    if (opts['t']) {
	runtime = atoi(opts['t']);
	if (runtime <= 0)
	    expire("Invalid time value");
    }

    return TRUE;
}

/* Starts the timer.
 */
void starttimer(int t)
{
    halfdelay(10);
    timetorun = t ? t : runtime;
    timestart = time(NULL);
}

/* If the timer is running, updates the time remaining on the screen
 * and returns FALSE if time has run out.
 */
int runtimer(void)
{
    int	y, x, t;

    if (timestart < 0)
	return TRUE;
    t = timetorun - difftime(time(NULL), timestart);
    if (t < 0)
	return FALSE;
    getyx(stdscr, y, x);
    movetostatus(FALSE);
    printw("%2d:%02d", t / 60, t % 60);
    move(y, x);
    refresh();
    return TRUE;
}

/* Pauses the timer if pause is TRUE, or starts it running again if
 * pause is FALSE.
 */
void pausetimer(int pause)
{
    static int timeleft = 0;

    if (pause) {
	if (timestart < 0) {
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

/* Turns off the timer, returning the amount of time still left.
 */
int stoptimer(void)
{
    int t;

    t = timetorun - difftime(time(NULL), timestart);
    timestart = -1;
    timetorun = 0;
    cbreak();
    return t;
}
