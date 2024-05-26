/*
 *	Compiler pass support for the target processor
 */

#include "compiler.h"


/* Everything is words */
unsigned target_alignof(unsigned t, unsigned storage)
{
	return 2;
}

/* Size of primitive types for this target */
/* These are byte sizes but this is a word machine */
static unsigned sizetab[16] = {
	1, 2, 4, 8,		/* char, short, long, longlong */
	1, 2, 4, 8,		/* unsigned forms */
	4, 8, 0, 0,		/* float, double, void, unused.. */
	0, 0, 0, 0		/* unused */
};

unsigned target_sizeof(unsigned t)
{
	unsigned s;

	if (PTR(t))
		return 2;

	s = sizetab[(t >> 4) & 0x0F];
	if (s == 0) {
		error("cannot size type");
		s = 1;
	}
	return s;
}

unsigned target_argsize(unsigned t)
{
	unsigned s = target_sizeof(t);
	if (s == 1)
		return 2;
	return s;
}

/* integer type for a pointer of type t. For most platforms this is trivial
   but strange boxes with word addressing and byte pointers may need help */
unsigned target_ptr_arith(unsigned t)
{
	/* We are word addressed but our byte pointers fit 16bits as our
	   addressing is 32K words with an indirection bit */
	return CINT;
}

/* Adjust scaling for a pointer of type t. For most systems this is a no-op
   but on machines with things like word addressing it isn't.*/

unsigned target_scale_ptr(unsigned t, unsigned scale)
{
	/* Get the type being pointed at */
	t = type_deref(t);
	/* Char is a byte pointer and scales by bytes */
	if (t == CCHAR || t == UCHAR)
		return scale;
	/* Words are word pointers and thus scale by 1, long by 2 */
	return scale / 2;
}

/* Scale a pointer offset to byte size */
unsigned target_ptroff_to_byte(unsigned t)
{
	t = type_deref(t);
	if (t == CCHAR || t == UCHAR || t == VOID)
		return 1;
	return 2;
}

/* Generate correct scaling for a struct field refence. This can cause
   a pointer byte change, plus offsets are always in bytes so must be fixed
   up */

struct node *target_struct_ref(struct node *n, unsigned type, unsigned off)
{
	/* The input node is a reference to the structure and is a word pointer */
	if (type == CCHAR || type == UCHAR || type == VOID)
		n = make_cast(n, PTRTO + type);
	else {
		n->type = PTRTO + type;
		if (off & 1)
			fatal("structalign");
		off /= 2;
	}
	n = tree(T_PLUS, n, make_constant(off, UINT));
	n->type = type;
	return n;
}

/* Remap any base types for simplicity on the platform */

unsigned target_type_remap(unsigned type)
{
	/* Our double is float */
	if (type == DOUBLE)
		return FLOAT;
	return type;
}

/* We could put some things in page 0 but it's not clear this has
   any value versus n,3 */
unsigned target_register(unsigned type, unsigned storage)
{
	return 0;
}

void target_reginit(void)
{
}
