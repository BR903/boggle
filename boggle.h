#ifndef _boggle_h_
#define _boggle_h_

#define	MM_ARGS		0x8000
#define	MM_CURSES	0x4000

#define	MODE_BATCH	(1 | MM_ARGS)
#define	MODE_MAKEDICT	(2 | MM_ARGS)
#define	MODE_SOLITAIRE	(3 | MM_CURSES)

extern int mode;
extern char const *thisfile;

extern int wordsinit(char*[]);
extern int cubeinit(char*[]);
extern int scoreinit(char*[]);
extern int timerinit(char*[]);
extern int outputinit(char*[]);
extern int inputinit(char*[]);
extern int playinit(char*[]);

#endif
