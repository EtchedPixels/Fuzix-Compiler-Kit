#ifndef _STDARG_H
#define	_STDARG_H

/* We really don't want to work in bytepointers here */
typedef unsigned *va_list;

/* The compiler isn't smart enough to turn this into inczr but it'll generate
   ok code */
#define __vasz(x)		((sizeof(x)+1) >> 1)
/* Point to the start of the last fixed argument *.
#define va_start(ap, parmN)	(ap = (va_list)&parmN)
/* For any type we move back the size of the dereferenced object (as the
   stack grows up. Do this in words to avoid bytepointer costs */
#define va_arg(ap, type)	(*((type *)(ap -= __vasz(type))))
/* As ever nothing to do */
#define va_end(ap)

#endif
