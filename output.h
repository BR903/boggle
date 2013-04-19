#ifndef	_output_h_
#define	_output_h_

extern void movetowords(int clear);
extern void movetostatus(int clear);
extern void addline(char const *fmt, ...);
extern void listwords(char const *heading, char const **words);
extern void drawgrid(void);
extern void drawgridletters(char const *highlighting);
extern void displayinputhelp(int show);

#endif
