/*
 *	C primary objects
 */

#include <stddef.h>
#include <stdio.h>
#include "compiler.h"

static struct node *badsizeof(void)
{
	error("bad sizeof");
	return make_constant(1, UINT);
}

/*
 *	sizeof() is a strange C thing that is sort of
 *	a function call but magic.
 */
struct node *get_sizeof(void)
{
	unsigned name;
	unsigned type;
	struct node *n, *r;

	/* Bracketing is required for sizeof unlike return */
	require(T_LPAREN);

	/* We wille eventually need to count typedefs as type_word */
	if (is_type_word() || is_typedef()) {
		type = type_name_parse(S_AUTO, get_type(), &name);
		if (type == UNKNOWN || name)
			return badsizeof();
		require(T_RPAREN);
		return make_constant(type_sizeof(type), UINT);
	}
	/* We can just allow sizeof on any expression */
	n = expression_tree(0);
	r = make_constant(type_sizeof(n->type), UINT);
	free_tree(n);
	require(T_RPAREN);
	return r;
}

/*
 *	The tokenizer has already done the basic conversion work for us and
 *	labelled the type produced. We just need to turn it into a name
 *	and for strings deal with the fact that a string is actually a
 *	literal holding the address of the characters.
 */
struct node *constant_node(void)
{
	struct node *n;
	unsigned label;
	unsigned t;

	/* Strings are special */
	label = quoted_string(NULL);
	if (label) {
		/* We have a temporary name */
		n = make_label(label);
		n->type = PTRTO + CCHAR;	/* PTR to CHAR */
		return n;
	}
	/* Numeric */
	switch (token) {
	case T_INTVAL:
		t = CINT;
		break;
	case T_LONGVAL:
		t = CLONG;
		break;
	case T_UINTVAL:
		t = UINT;
		break;
	case T_ULONGVAL:
		t = ULONG;
		break;
	case T_FLOATVAL:
		t = FLOAT;
		break;
	default:
		error("invalid value");
		t = CINT;
		break;
	}
	n = make_constant(token_value, t);
	next_token();
	return n;
}

/*
 *	A C language primary. This can be one of several things
 *
 *	1.	Another expression in brackets, in which case we recurse
 *	2.	sizeof() - basically a magic constant.
 *	3.	A name
 *	4.	A constant
 */
struct node *primary(void)
{
	struct node *l;
	unsigned func = 0;
	unsigned name;
	unsigned p = 0;

	/* Expression case first.. a bracketed expression is a primary */
	if (match(T_LPAREN)) {
		l = hier1();
		require(T_RPAREN);
		return l;
	}
	/* C magic - sizeof */
	if (match(T_SIZEOF))
		return get_sizeof();
	/* Names or types */
	name = symname();
	if (token == T_LPAREN)
		func = 1;
	if (name) {
		struct symbol *sym = find_symbol(name);
		/* Weird case you can call a function you've not declared. This
		   makes it int f() */
		if (func && sym == NULL) {
			unsigned tf = func_symbol_type(CINT, &p);
			sym = update_symbol_by_name(name, S_EXTERN, tf);
		}
		/* You can't size fields and structs by field/struct name without 
		   the type specifier */
		if (sym == NULL) {
			/* Enum... */
			if (find_constant(name, &p) == 0)
				error("unknown symbol");
			return make_constant(p, CINT);
		}
		/* Primary can be followed by operators and the caller handles those */
		return make_symbol(sym);
	}
	/* Not a name - constants or strings */
	return constant_node();
}
