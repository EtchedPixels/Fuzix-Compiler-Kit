/*
 *	Compiler pass support for the target processor
 */

#include "compiler.h"


unsigned target_alignof(unsigned t, unsigned storage)
{
    return 1;
}

/* Size of primitive types for this target */
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
	return s;
}

/* integer type for a pointer of type t. For most platforms this is trivial
   but strange boxes with word addressing and byte pointers may need help */
unsigned target_ptr_arith(unsigned t)
{
	return CINT;
}

/* Adjust scaling for a pointer of type t. For most systems this is a no-op
   but on machines with things like word addressing it isn't.*/

unsigned target_scale_ptr(unsigned t, unsigned scale)
{
	return scale;
}

/* Scale a pointer offset to byte size */
unsigned target_ptroff_to_byte(unsigned t)
{
	return 1;
}

struct node *target_struct_ref(struct node *n, unsigned type, unsigned off)
{
	n->type = PTRTO + type;
	n = tree(T_PLUS, n, make_constant(off, UINT));
	n->type = type;
	return n;
}

/* Can we remove pointer/int casts for fixed objects */
unsigned target_remove_cast(struct node *l, struct node *r)
{
	return 1;
}

/* Remap any base types for simplicity on the platform */
unsigned target_type_remap(unsigned type)
{
	/* Our double is float */
	if (type == DOUBLE)
		return FLOAT;
	return type;
}

static unsigned rused;

/* Just hand out register pairs for anything byte or word sized */
unsigned target_register(unsigned type, unsigned storage)
{
	/* No long or floats */
	if (type >= CLONG && !PTR(type))
		return 0;
	/* None left ? */
	if (rused == 2)
		return 0;
	rused++;
	if (storage == S_AUTO)
		func_flags |= F_REG(rused);
	else
		arg_flags |= F_REG(rused);
	return rused;
}

void target_reginit(void)
{
	rused = 0;
}
