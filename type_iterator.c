/*
 *	Handle all the int a, char *b[433], *x() stuff
 */

#include <stdio.h>
#include "compiler.h"

unsigned is_modifier(void)
{
	return (token == T_CONST || token == T_VOLATILE);
}

unsigned is_type_word(void)
{
	return (token >= T_CHAR && token <= T_VOID);
}

/* This one is more expensive so use it last when possible */
struct symbol *is_typedef(void)
{
	return find_symbol_by_class(token, S_TYPEDEF);
}

void skip_modifiers(void)
{
	while (is_modifier())
		next_token();
}

static unsigned typeconflict(void) {
	error("type conflict");
	return CINT;
}

/* Structures */
static unsigned structured_type(unsigned sflag)
{
	struct symbol *sym;
	unsigned name = symname();

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
			set_once(16);
			type = CCHAR;
			break;
		case T_INT:
			set_once(16);
			type = CINT;
			break;
		case T_FLOAT:
			set_once(16);
			type = FLOAT;
			break;
		case T_DOUBLE:
			set_once(16);
			type = DOUBLE;
			break;
		case T_VOID:
			set_once(16);
			type = VOID;
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
	/* No signed/unsigned/short float or double */
	if ((type == FLOAT || type == DOUBLE) && (once_flags & 13))
		return typeconflict();
	/* No void modifiers */
	if (type == VOID && (once_flags & 15))
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

unsigned get_type(void)
{
	unsigned sflag = 0;
	struct symbol *sym;
	unsigned type;

	skip_modifiers();

	if (match(T_ENUM))
		type = enum_body();
	else if ((sflag = match(T_STRUCT)) || match(T_UNION))
		type = structured_type(sflag);
	else if (is_type_word())
		type = base_type();
	else {
		/* Check for typedef */
		sym = find_symbol_by_class(token, S_TYPEDEF);
		if (sym == NULL)
			return UNKNOWN;
		next_token();
		type = sym->type;
	}
	skip_modifiers();
	return type;
}

/*
 *	Parse an ANSI C style function (we don't do K&R at all)
 *
 *	We build a vector of type descriptors for the arguments and then
 *	build a type from that. All functions with the same argument pattern
 *	have the same type code. That saves us a ton of space and also means
 *	we can compare function pointer equivalence trivially
 *
 *	ptr tells us if this is a pointer declaration and therefore must
 *	not have a body. We will need that once we move the body parsing
 *	here.
 */


static unsigned type_parse_function(struct symbol *fsym, unsigned storage, unsigned type, unsigned ptr)
{
	/* Function returning the type accumulated so far */
	/* We need an anonymous symbol entry to hang the function description onto */
	struct symbol *sym;
	unsigned an;
	unsigned t;
	unsigned tplt[33];	/* max 32 typed arguments */
	unsigned *tn = tplt + 1;

	/* Parse the bracketed arguments if any and nail them to the
	   symbol. */
	while (token != T_RPAREN) {
		/* TODO: consider K&R support */
		if (tn == tplt + 33)
			fatal("too many arguments");
		if (token == T_ELLIPSIS) {
			next_token();
			*tn++ = ELLIPSIS;
			break;
		}
		t = get_type();
		if (t == UNKNOWN)
			t = CINT;
		t = type_name_parse(S_ARGUMENT, t, &an);
		if (t == VOID) {
			*tn++ = VOID;
			break;
		}
		/* Arrays pass the pointer */
		t = type_canonical(t);
		if (!PTR(t) && (IS_STRUCT(t) || IS_FUNCTION(t))) {
			error("cannot pass objects");
			t = CINT;
		}
		if (an) {
			sym = update_symbol_by_name(an, S_ARGUMENT, t);
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
	if (tn == tplt + 1)
		*tn++ = ELLIPSIS;
	*tplt = tn - tplt - 1;
	type = func_symbol_type(type, tplt);
	if (!ptr) {
		/* Must do this first as a function may reference itself */
		update_symbol(fsym, fsym->name, storage, type);
		if (token == T_LCURLY) {
			unsigned argsave, locsave;
			struct symbol *ltop;

			if (fsym->flags & INITIALIZED)
				error("duplicate function");
			if (storage == S_AUTO || storage == S_NONE)
				error("function not allowed");
			if (storage == S_EXTDEF)
				header(H_EXPORT, fsym->name, 0);
			ltop = mark_local_symbols();
			mark_storage(&argsave, &locsave);
			function_body(storage, fsym->name, type);
			pop_local_symbols(ltop);
			pop_storage(&argsave, &locsave);
			fsym->flags |= INITIALIZED;
		}
	}
	return type;
}

static unsigned type_parse_array(unsigned storage, unsigned type, unsigned ptr)
{
	int n;

	/* Arguments can have [] form but it's just an alias of * and
	   a syntactic quirk. We need to handle it here because of the
	   way our type matching is handled and to allow [] */
	if (token == T_RSQUARE) {
		if (storage == S_ARGUMENT || ptr)
			return type + 1;
		error("size required");
	}
	n = const_int_expression();
	if (n < 1) {
		error("bad size");
		n = 1;
	}
	if (!IS_ARRAY(type))
		type = make_array(type);
	array_add_dimension(type, n);
	/* TODO: handle the nptr case correctly */
	return type + 1;
}

/* Recursively walk any *(*(*(*x))) bits

	char *x()	function returning char
	char (*x)()	pointer to function returning char
 */

static unsigned declarator(unsigned *name)
{
	unsigned ptr = 0;
	*name = 0;

	skip_modifiers();
	while (match(T_STAR)) {
		skip_modifiers();
		ptr++;
	}
	skip_modifiers();
	if (token >= T_SYMBOL) {	/* It's a name */
		*name = token;
		next_token();
		return ptr;
	}
	if (token == T_LPAREN) {
		next_token();
		ptr += declarator(name);
		require(T_RPAREN);
	}
	return ptr;
}

unsigned type_name_parse(unsigned storage, unsigned type, unsigned *name)
{
	unsigned ptr = 0, nptr;
	struct symbol *sym = NULL, *ltop;

	/* Finish the type */
	skip_modifiers();
	while(match(T_STAR)) {
		skip_modifiers();
		ptr++;
	}

	/* Check if the name is a pointer */
	nptr = declarator(name);
	if (nptr + ptr > 7)
		indirections();

	/* Add the pointer depth to the base type */
	type += ptr;

	/* Reserve a symbol slot if needed */
	if (*name && storage != S_NONE)
		sym = update_symbol_by_name(*name, storage, C_ANY);

	/* All the symbols within the declaration below are local to the
	   declaration if not static/global */
	ltop = mark_local_symbols();
	/* We may be a function specification or an array or both. The other
	   post forms (-> and .) are not valid in a declaration */
	while (token == T_LSQUARE || token == T_LPAREN) {
		if (token == T_LSQUARE) {
			next_token();
			type = type_parse_array(storage, type, nptr);
			require(T_RSQUARE);
		} else if (token == T_LPAREN) {
			next_token();
			/* It's a function description  */
			type = type_parse_function(sym, storage, type, nptr);
			/* Can be an array of functions .. */
		}
		/* FIXME: stop nonsense like func()[]() */
	}
	pop_local_symbols(ltop);
	/* Add any pointer element attached to the name */
	if (PTR(type) + nptr > 7)
		indirections();
	else
		type += nptr;
	if (sym)
		update_symbol(sym, *name, storage, type);
	return type;
}
