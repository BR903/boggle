#ifndef	_words_h_
#define	_words_h_

extern char *dictfilename;
extern int minlen;

extern char const **getfindable(void);
extern char const **getfound(void);
extern int acceptword(char const *word, int check);
extern void filterfound(void);
extern int findall(void);

#endif
