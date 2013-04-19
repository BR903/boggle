#ifndef	_cube_h_
#define	_cube_h_

extern int width, height, gridsize;
extern char *grid;
extern short (*neighbors)[9];

extern void shakecube(void);
extern char *findword(char const *word);

#endif
