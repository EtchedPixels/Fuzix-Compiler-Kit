/*
 *	Compiler pass support for the target processor
 */

#include "compiler.h"

static unsigned u_free;

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

/* We can work with byte sized arguments as bytes, in fact it's rather
   cheaper to do so */
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

/* Remap any base types for simplicity on the platform */

unsigned target_type_remap(unsigned type)
{
	/* Our double is float */
	if (type == DOUBLE)
		return FLOAT;
	return type;
}

/* For the 6809 we can use U as a register variable. For the other processors
   we have the option of using elements of direct page. This is marginally faster
   than ,X and avoids some TSX reloads. That needs looking at later to see if it is
   worthwhile */

static unsigned ralloc(unsigned storage, unsigned n)
{
	/* Tell the backend what is allocated */
	if (storage == S_AUTO)
		func_flags |= F_REG(n);
	else
		arg_flags |= F_REG(n);
	return n;
}

unsigned target_register(unsigned type, unsigned storage)
{
	if (target_sizeof(type) == 2 && cputype == 6809 && u_free == 1) {
		u_free = 0;
		return ralloc(storage, 1);
	}
	return 0;
}

void target_reginit(void)
{
	u_free = 1;
}
