#include <stdio.h>
#include "compiler.h"


unsigned type_deref(unsigned t)
{
	if (PTR(t))
		return --t;
	/* Arrays complicate matters because you can do
		*x on an array object */
	t = type_canonical(t);
	if (PTR(t))
		return --t;
	error("cannot dereference");
	return CINT;
}

unsigned type_ptr(unsigned t)
{
	if (PTR(t) == 7)
		indirections();
	else
		t++;
	return t;
}

/*
 *	Handle array v type pointers.
 */
unsigned type_canonical(unsigned t)
{
	if (IS_ARRAY(t)) {
		struct symbol *s = symbol_ref(t);
		unsigned *p = s->idx;
		/* FIXME: array depths or 1 ?? */
		if (*p + PTR(t) > 7)
			indirections();
		else
			t = s->type | (*p + PTR(t));
	}
	return t;
}

unsigned type_arraysize(unsigned t)
{
	struct symbol *sym = symbol_ref(t);
	unsigned *p = sym->idx;
	unsigned n = *p++;
	unsigned s = type_sizeof(sym->type);
	while (n--)
		s *= *p++;
	return s;
}

unsigned type_sizeof(unsigned t)
{
	if (PTR(t) || IS_SIMPLE(t))
		return target_sizeof(t);
	if (IS_ARRAY(t)) {
		return type_arraysize(t);
	}
	if (IS_STRUCT(t)) {
		struct symbol *s = symbol_ref(t);
		unsigned *p = s->idx;
		if (s->flags & INITIALIZED)
			return p[1];
		error("struct/union not declared");
		return 1;
	}
	/* Umm.. help ?? */
	error("can't size type");
	return 1;
}

unsigned type_ptrscale(unsigned t) {
	/* FIXME arrays - scaling rule ? */
	t = type_canonical(t);
	if (!PTR(t)) {
		error("not a pointer");
		return 1;
	}
	return type_sizeof(type_deref(t));
}

unsigned type_scale(unsigned t) {
	t = type_canonical(t);
	if (!PTR(t))
		return 1;
	return type_sizeof(type_deref(t));
}

/* lvalue conversion is handled by caller */
unsigned type_addrof(unsigned t) {
	if (PTR(t))
		return t - 1;
	error("cannot take address");
	return VOID + 1;
}

unsigned type_ptrscale_binop(unsigned op, unsigned l, unsigned r,
			     unsigned *div) {
	*div = 1;

	if (PTR(l) && PTR(r)) {
		/* FIXME: allow for sign differences as warning. Want
		   a generic ptrcompare() */
		if (type_canonical(l) != type_canonical(r))
			error("pointer type mismatch");
		if (op == T_MINUS)
			return type_ptrscale(r);
		else {
			error("invalid pointer difference");
			return 1;
		}
	}
	*div = 0;
	if (PTR(l) && IS_ARITH(r))
		return type_ptrscale(l);
	if (PTR(r) && IS_ARITH(l))
		return type_ptrscale(r);
	if (IS_ARITH(l) && IS_ARITH(r))
		return 1;
	error("invalid types");
	return 1;
}

int type_pointermatch(struct node *l, struct node *r)
{
    unsigned lt = l->type;
    unsigned rt = r->type;
    /* The C zero case */
    if (is_constant_zero(l) && PTR(rt))
        return 1;
    if (is_constant_zero(r) && PTR(lt))
        return 1;
    /* Not pointers */
    if (!PTR(lt) || !PTR(rt))
        return 0;
    /* Same depth and type */
    if (lt == rt)
        return 1;
    /* void * is fine */
    if (BASE_TYPE(lt) == VOID)
        return 1;
    if (BASE_TYPE(rt) == VOID)
        return 1;
    /* sign errors */
    if (BASE_TYPE(lt) < FLOAT && (lt ^ rt) == UNSIGNED) {
        warning("sign mismatch");
        return 1;
    }
    return 0;
}

