/*
 *	C primary objects
 */

#include <stddef.h>
#include <stdio.h>
#include "compiler.h"

struct node *get_sizeof(void)
{
	unsigned name;
	require(T_LPAREN);
	/* Names or types */
	name = symname();
	if (name) {
		/* This chunk is wrong... we should parse a full type description 
		   FIXME: it's legal to sizeof(x[4]) or sizeof(x->y); */
		struct symbol *sym = find_symbol(name);
		/* You can't size fields and structs by field/struct name without 
		   the type specifier */
		require(T_RPAREN);
		if (sym == NULL || sym->storage > S_EXTDEF) {
			error("unknown symbol");
			return make_constant(1, UINT);
		}
		return make_constant(type_sizeof(sym->type), UINT);
	}
	/* Not a name.. should be a type */
	/* TODO */
	return make_constant(2, UINT);
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
		n->type = CCHAR + 1;	/* PTR to CHAR */
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
			unsigned p = 0;
			unsigned tf = func_symbol_type(CINT, &p);
			sym = update_symbol(name, S_EXTERN, tf);
		}
		/* You can't size fields and structs by field/struct name without 
		   the type specifier */
		if (sym == NULL || sym->storage > S_EXTDEF) {
			fprintf(stderr, "Couldn't find %u %p\n", name, (void *)sym);
			error("unknown symbol");
			return make_constant(0, UINT);
		}
		/* Primary can be followed by operators and the caller handles those */
		return make_symbol(sym);
	}
	/* Not a name - constants or strings */
	return constant_node();
}
