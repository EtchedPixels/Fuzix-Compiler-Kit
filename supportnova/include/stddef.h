#ifndef _STDDEF_H
#define	_STDDEF_H

#include <stdint.h>

#define	NULL ((void *)0)

/* This works for the Nova because we explicitly turn pointers into
   bytepointers when casting to integer types */
#define	offsetof(type, ident)	((size_t) &((type *)0)->ident)

#endif
