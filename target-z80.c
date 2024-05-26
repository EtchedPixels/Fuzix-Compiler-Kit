/*
 *	Compiler pass support for the target processor
 */

#include "compiler.h"


unsigned target_alignof(unsigned t, unsigned storage)
{
    /* Arguments are stacked as words on 8080 */
    if (storage == S_ARGUMENT)
	return 2;
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
	if (s == 1)
		return 2;
	return s;
}

/* integer type for a pointer of type t. For most platforms this is trivial
   but strange boxes with word addressing and byte pointers may need help */
unsigned target_ptr_arith(unsigned t)
{
	return CINT;
}

/* Scale a pointer offset to byte size */
unsigned target_ptroff_to_byte(unsigned t)
{
	return 1;
}

/* Adjust scaling for a pointer of type t. For most systems this is a no-op
   but on machines with things like word addressing it isn't.*/

unsigned target_scale_ptr(unsigned t, unsigned scale)
{
	return scale;
}

struct node *target_struct_ref(struct node *n, unsigned type, unsigned off)
{
	n->type = PTRTO + type;
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

static unsigned bc_free;
static unsigned ix_free;
static unsigned iy_free;

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
	if (type >= CLONG && !PTR(type))
		return 0;
	/* For now only use IX/IY for pointers */
	if (PTR(type)) {
		if (ix_free) {
			ix_free = 0;
			return ralloc(storage, 2);
		}
		if (iy_free) {
			iy_free = 0;
			return ralloc(storage, 3);
		}
	}
	/* BC is good for 8 and 16 bit maths or byte pointers, but prefer ix/iy for pointers */
	if (bc_free == 0)
		return 0;
	if (PTR(type) == 0 || (PTR(type) == 1 && type < CSHORT)) {
		bc_free = 0;
		return ralloc(storage, 1);
	}
	return 0;
}

void target_reginit(void)
{
	bc_free = 1;
	if (!(cpufeat & 2))	/* --no-ix */
		ix_free = 1;
	if (!(cpufeat & 4))	/* --no-iy */
		iy_free = 1;
}
