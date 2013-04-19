#ifndef	_score_h_
#define	_score_h_

typedef	struct scores {
    int	    ncount, ncount1;
    int	    nscore, nscore1;
    int	    dcount, dcount1;
    int	    dscore, dscore1;
    int	    gamecount;
} scores;

#define	EMPTYSCORES	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }

extern void reportwords(void);
extern void reportscore(void);

#endif
