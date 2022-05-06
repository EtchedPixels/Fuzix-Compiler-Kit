#include <stdio.h>
#include "compiler.h"


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

void skip_modifiers(void)
{
	while (is_modifier())
		next_token();
}

/* Structures */
static unsigned structured_type(unsigned sflag)
{
	struct symbol *sym;
	unsigned name = symname();

	/* Our func/sym names partly clash for now like an old school compiler. We
	   can address that later FIXME */
	sym = update_struct(name, sflag);
	if (sym == NULL) {
		error("not a struct");
		junk();
		return CINT;
	}
	/* Now see if we have a struct declaration here */
	if (token == T_LCURLY) {
		struct_declaration(sym);
	}
	/* Encode the struct. The caller deals with any pointer, array etc
	   notation */
	return type_of_struct(sym);
}

static unsigned once_flags;

static void set_once(unsigned bits)
{
	if (once_flags & bits)
		typeconflict();
	once_flags |= bits;
}

static unsigned base_type(void)
{
	once_flags = 0;
	unsigned type = CINT;

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
	return type;
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
	unsigned type;
	unsigned ptr = 0;


	/* Not a type */
	if (!is_modifier() && token != T_STAR && !is_type_word())
		 return UNKNOWN;

	skip_modifiers();	/* volatile const etc */

	/* Check - is this after only (ie is **int** legal) TODO */
	while (match(T_STAR)) {
		skip_modifiers();	/* volatile const etc */
		ptr++;
	}
	if (ptr > 7)
		 error("too many indirections");

	skip_modifiers();	/* volatile const etc */

	if ((sflag = match(T_STRUCT)) || match(T_UNION))
		type = structured_type(sflag);
	else
		type = base_type();

	while (match(T_STAR)) {
		skip_modifiers();	/* volatile const etc */
		ptr++;
	}

	if (ptr > 7)
		error("too many indirections");
	return type + ptr;
}

/*
 *	Parse an ANSI C style function header (we don't do K&R at all)
 *
 *	We build a vector of type descriptors for the arguments and then
 *	build a type from that. All functions with the same argument pattern
 *	have the same type code. That saves us a ton of space and also means
 *	we can compare function pointer equivalence trivially
 */
static unsigned type_parse_function(unsigned name, unsigned type) {
	/* Function returning the type accumulated so far */
	/* We need an anonymous symbol entry to hang the function description onto */
	struct symbol *sym;
	unsigned an;
	unsigned t;
	unsigned tplt[33];	/* max 32 typed arguments */
	unsigned *tn = tplt + 1;

	/* Parse the bracketed arguments if any and nail them to the
	   symbol. For now just eat them. We need to add them as local symbols
	   and check collisions etc */
	while (token != T_RPAREN) {
		/* FIXME special rule for void and ellipsis and they depend whether
		   it's a function def or header. Also maybe K&R format support ? */

		if (tn == tplt + 33)
			fatal("too many arguments");
		if (token == T_ELLIPSIS) {
			next_token();
			*tn++ = ELLIPSIS;
			break;
		}
		t = type_and_name(&an, 0, CINT);
		if (t == VOID) {
			*tn++ = VOID;
			break;
		}
		/* Arrays pass the pointer */
		t = type_canonical(t);
		if (IS_STRUCT(t)) {
			error("cannot pass structures");
			t = CINT;
		}
		if (name) {
			fprintf(stderr, "argument symbol %x\n", name);
			sym = update_symbol(an, S_ARGUMENT, t);
			sym->offset = assign_storage(t, S_ARGUMENT);
			*tn++ = t;
		} else {
			assign_storage(t, S_ARGUMENT);
			*tn++ = t;
		}
		if (!match(T_COMMA))
			break;
	}
	require(T_RPAREN);
	/* A zero length comes from () and means 'whatever' so semantically
	   for us at least 8) it's an ellipsis only */
	if (tn == tplt)
		*tn++ = ELLIPSIS;
	*tplt = tn - tplt;
	type = func_symbol_type(tplt);
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
