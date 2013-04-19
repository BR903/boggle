/* (C) 1999 Brian Raiter (under the terms of the GPL) */

#ifndef _boggle_h_
#define _boggle_h_

/* The "modes" are the different "programs" that the executable can run. */

#define	MM_ARGS		0x8000		/* the mode takes extra args	*/
#define	MM_CURSES	0x4000		/* the mode is interactive	*/

#define	MODE_BATCH	(1 | MM_ARGS)	/* filtering a wordlist		*/
#define	MODE_MAKEDICT	(2 | MM_ARGS)	/* the dictionary compiler	*/
#define	MODE_SOLITAIRE	(3 | MM_CURSES)	/* actually playing the game	*/

/* The selected mode.
 */
extern int mode;

/* argv[0].
 */
extern char const *thisfile;

/* The many initialization functions. Each one is passed a pointer to
 * the array of command-line options. A function can return FALSE to
 * cause the program to abort.
 */
extern int wordsinit(char*[]);
extern int cubeinit(char*[]);
extern int scoreinit(char*[]);
extern int timerinit(char*[]);
extern int outputinit(char*[]);
extern int inputinit(char*[]);
extern int playinit(char*[]);

#endif
