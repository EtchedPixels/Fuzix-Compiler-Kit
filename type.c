#include <stdio.h>
#include "compiler.h"

static unsigned sizetab[16] = {
	/* FIXME - target should supply */
	1, 2, 4, 8,
	1, 2, 4, 8,
	4, 8, 1, 0,		/* A void has no size but a void * is deemed to be 1 */
	0, 0, 0, 0
};

static unsigned primitive_sizeof(unsigned t)
{
	unsigned s = sizetab[(t >> 4) & 0x0F];
	if (s == 0) {
		error("cannot size type");
		s = 1;
	}
	return s;
}

unsigned type_deref(unsigned t)
{
	switch (CLASS(t)) {
	case C_SIMPLE:
		if (PTR(t))
			return --t;
		break;
	default:;
	}
	error("cannot dereference");
	return CINT;
	/* FIXME: arrays, struct etc */
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
#if 0
	if (IS_ARRAY(t)) {
		struct sym *s = symtab + INFO(t);
		unsigned *p = idx_data(s->idx);
		/* FIXME: array depths or 1 ?? */
		if (*p + PTR(t) > 7)
			indirections();
		else
			t = s->type | (*p + PTR(t));
	}
#endif
	return t;
}

#if 0
unsigned type_arraysize(unsigned t)
{
	struct sym *s = symtab + INFO(t);
	unsigned *p = idx_data(s->idx);
	unsigned n = *p++;
	unsigned s = type_sizeof(s->type);
	while (n--)
		s *= *p++;
	return s;
}
#endif

unsigned type_sizeof(unsigned t)
{
	if (PTR(t))
		return PTRSIZE;
	if (IS_SIMPLE(t))
		return primitive_sizeof(t);
#if 0
	if (IS_ARRAY(t)) {
		return type_arraysize(t));
	}
	if (IS_STRUCT(t)) {
		struct sym *s = symtab + INFO(t);
		unsigned *p = idx_data(s->idx);
		return p[1];
	}
#endif
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

	/* FIXME: need an array to pointer helper of some form */
	if (PTR(l) && PTR(r)) {
		if (l != r)
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


static unsigned typeconflict(void) {
	error("type conflict");
	return CINT;
}

unsigned is_modifier(void)
{
	return (token == T_CONST || token == T_VOLATILE);
}

unsigned is_type_word(void)
{
	return (token >= T_CHAR && token <= T_VOID);
}

void skip_modifiers(void) {
	while (is_modifier())
		next_token();
}
static unsigned once_flags;

static void set_once(unsigned bits)
{
	if (once_flags & bits)
		typeconflict();
	once_flags |= bits;
}
/*
 *	Parse all the type blurb up to the symbol name. We don't deal with
 *	typedefs yet and that complicates things as you can have typedefs
 *	that have pointer/arrayness. Also TODO is enum...
 *
 *	For our simpler case it's a lot easier. We are going to hand back
 *	either a struct or union type with some degree of pointer indirection
 *	possible
 *
 *	Stuff following the name is not our problem. So int x[4] is an int
 *	to us. The fact it's any array is the callers fun for now but wants
 *	centralizing somewhere else (especially once we hit typedef)
 */

unsigned get_type(void) {
	unsigned sflag = 0;
	unsigned type = CINT;	/* default */
	unsigned ptr = 0;


	/* Not a type */
	if (!is_modifier() && token != T_STAR && !is_type_word())
		 return UNKNOWN;

	 skip_modifiers();	/* volatile const etc */

	while (match(T_STAR)) {
		skip_modifiers();	/* volatile const etc */
		ptr++;
	}
	if (ptr > 7)
		 error("too many indirections");

	once_flags = 0;

	skip_modifiers();	/* volatile const etc */
#if 0
	if ((sflag = match(T_STRUCT)) || match(T_UNION))
		return structured_type(sflag);
#endif
	while (is_type_word()) {
		switch (token) {
		case T_SHORT:
			set_once(1);
			break;
		case T_LONG:
			/* For now -- long long is for the future */
			set_once(2);
			break;
		case T_UNSIGNED:
			set_once(4);
			break;
		case T_SIGNED:
			set_once(8);
			break;
		case T_CHAR:
			type = CCHAR;
			break;
		case T_INT:
			type = CINT;
			break;
		case T_FLOAT:
			type = FLOAT;
			break;
		case T_DOUBLE:
			type = DOUBLE;
			break;
		}
		next_token();
	}
	/* Now put it all together */

	/* No signed unsigned or short longs */
	if ((once_flags & 3) == 3 || (once_flags & 12) == 12)
		return typeconflict();
	/* No long or short char */
	if (type == CCHAR && (once_flags & 3))
		return typeconflict();
	/* No signed/unsigned float or double */
	if ((type == FLOAT || type == DOUBLE) && (once_flags & 12))
		return typeconflict();
	/* No void modifiers */
	if (type == VOID && once_flags)
		return typeconflict();
	/* long */
	if (type == CINT && (once_flags & 2))
		type = CLONG;
	if (type == FLOAT && (once_flags & 2))
		type = DOUBLE;
	/* short */
	/* FIXME: allow for int being long/short later */
	if (type == CINT && (once_flags & 1))
		type = CINT;
	/* signed/unsigned */
	if (once_flags & 4)
		type |= UNSIGNED;
	/* We don't deal with default unsigned char yet .. */
	if (once_flags & 8)
		type &= ~UNSIGNED;

	while (match(T_STAR)) {
		skip_modifiers();	/* volatile const etc */
		ptr++;
	}

	if (ptr > 7)
		error("too many indirections");
	return type + ptr;
}

/*
 *	Above this lot sits a parser for typedef and declarations
 *	that handles the name (sometimes optional) and trailing array etc
 *
 *	We want to split that out from the current primary handling so we
 *	can use it for typedef.
 *
 *	We don't deal with storage classes at this level
 */

/*
 *	TODO - defer allocation of sym type and build param array then
 *	try and match it to an existing prototype index
 */
static unsigned type_parse_function(unsigned name, unsigned type) {
	/* Function returning the type accumulated so far */
	/* We need an anonymous symbol entry to hang the function description onto */
	struct symbol *sym;
	unsigned an;
	unsigned t;
	/* Our type is function */
	 type = C_FUNCTION /*|symbol_number(sym) */ ;
	/* Parse the bracketed arguments if any and nail them to the
	   symbol. For now just eat them. We need to add them as local symbols
	   and check collisions etc */
	while (token != T_RPAREN) {
		/* FIXME special rule for void and ellipsis and they depend whether
		   it's a function def or header. Also maybe K&R format support ? */
		if (token == T_ELLIPSIS) {
			next_token();
//            function_add_argument(sym, 0, ELLIPSIS);
			break;
		}
		t = type_and_name(&an, 0, CINT);
		if (t == VOID) {
//            function_add_argument(sym, 0, VOID);
			break;
		}
		if (name)
			update_symbol(an, S_ARGUMENT, t);
		if (!match(T_COMMA))
			break;
	}
	require(T_RPAREN);
	fprintf(stderr, "func type %x\n", type);
	return type;
}

static unsigned type_parse_array(unsigned name, unsigned type) {
#if 0
	int v;
	if (!IS_ARRAY(type))
		 make_array(type);
	/* Using number is a hack until we have proper const trees */
	 number(&v);
	 array_add_dimension(type, v);
#endif
	 return CINT;		/* until we write this */
}
/*
 *	Turn a declaration into a type and name, parse anything function
 *	declarations, parse any array dimensions and turn the whole lot into
 *	a final packaged type and name.
 *
 *	FIXME: C permits stuff like struct definition in situ (eg
 *	typedef struct foo { int a; int b; } bar;
 */

unsigned type_and_name(unsigned *np, unsigned need_name,
			   unsigned deftype)
{
	unsigned type = get_type();
	unsigned sym;
	 skip_modifiers();
	if (type == UNKNOWN)
		type = deftype;
	if (type == UNKNOWN)
		return type;
	 sym = symname();
	if (sym == 0 && (need_name))
		error("name expected");
	*np = sym;
	/* We may be a function specification or an array or both. The other
	   post forms (-> and .) are not valid in a declaration */
	while (token == T_LSQUARE || token == T_LPAREN) {
		if (token == T_LSQUARE) {
			next_token();
			type = type_parse_array(*np, type);
			require(T_RSQUARE);
		} else if (token == T_LPAREN) {
			fprintf(stderr, "func check\n");
			next_token();
			/* It's a function description  */
			type = type_parse_function(*np, type);
			fprintf(stderr, "tok now %x\n", token);
			/* Can be an array of functions .. */
		}
	}
	fprintf(stderr, "final type is %x\n", type);
	return type;
}
