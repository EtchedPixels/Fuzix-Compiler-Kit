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


unsigned get_type(void) {
	unsigned sflag = 0;
	unsigned type;


	/* Not a type */
	if (!is_modifier() && !is_type_word())
		 return UNKNOWN;

	skip_modifiers();	/* volatile const etc */

	if ((sflag = match(T_STRUCT)) || match(T_UNION))
		type = structured_type(sflag);
	else
		type = base_type();
	return type;
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
		if (an) {
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
	if (tn == tplt + 1)
		*tn++ = ELLIPSIS;
	*tplt = tn - tplt - 1;
	type = func_symbol_type(type, tplt);
	return type;
}

static unsigned type_parse_array(unsigned type) {
	int n = const_int_expression();
	if (n < 1) {
		error("bad size");
		n = 1;
	}
	if (!IS_ARRAY(type))
		 type = make_array(type);
	array_add_dimension(type, n);
	return type;
}

/* Recursively walk any *(*(*(*x))) bits */

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

static unsigned type_name_parse(unsigned type, unsigned *name)
{
	unsigned ptr = declarator(name);
	if (ptr > 7)
		indirections();
	/* We may be a function specification or an array or both. The other
	   post forms (-> and .) are not valid in a declaration */
	while (token == T_LSQUARE || token == T_LPAREN) {
		if (token == T_LSQUARE) {
			next_token();
			type = type_parse_array(type);
			require(T_RSQUARE);
		} else if (token == T_LPAREN) {
			next_token();
			/* It's a function description  */
			type = type_parse_function(*name, type);
			/* Can be an array of functions .. */
		}
	}
	return type + ptr;
}

unsigned type_and_name(unsigned *name, unsigned nn, unsigned deftype)
{
	unsigned type = get_type();
	if (type == UNKNOWN)
		type = deftype;
	if (type == UNKNOWN)
		return type;
	type = type_name_parse(type, name);
	if (nn && *name == 0) {
		error("name required");
		junk();
	}
	return type;
}

void type_iterator(unsigned storage, unsigned deftype, unsigned info,
		   unsigned (*handler)(unsigned storage, unsigned type, unsigned name, unsigned info))
{
	unsigned name;
	unsigned utype;
	unsigned type;

	type = get_type();
	if (type == UNKNOWN)
		type = deftype;
	if (type == UNKNOWN)
		return;

//	while (is_modifier() || is_type_word() || token >= T_SYMBOL || token == T_STAR) {
	while (token != T_SEMICOLON) {
		utype = type_name_parse(type, &name);
		if (handler(storage, utype, name, info) == 0)
			return;
		if (!match(T_COMMA))
			break;
	}
	need_semicolon();
}
