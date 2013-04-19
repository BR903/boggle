#ifndef	_genutil_h_
#define	_genutil_h_

#ifndef TRUE
#define TRUE	1
#define	FALSE	0
#endif

#ifndef	DIRSEPCHAR
#define	DIRSEPCHAR	'/'
#endif

#define	WORDBUFSIZ	99

extern void *xmalloc(size_t size);
extern void *xrealloc(void *mem, size_t size);
extern void expire(char const *msg);

#define	auxassert(Z,L)  ((void)((Z) || (expire("CRASH " __FILE__ ":" #L), 0)))
#define assert(Z)	(auxassert(Z, __LINE__))

#endif
