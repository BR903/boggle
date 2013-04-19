/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#ifndef	_timer_h_
#define	_timer_h_

/* Starts the timer running for t seconds, or for the default length
 * of time if t is zero.
 */
extern void starttimer(int t);

/* Updates the timer display and returns FALSE if time has run out.
 */
extern int runtimer(void);

/* Pauses the timer if pause is TRUE, or starts it running again if
 * pause is FALSE.
 */
extern void pausetimer(int pause);

/* Turns off the timer, returning the amount of time still left (in
 * clocks).
 */
extern int stoptimer(void);

#endif
