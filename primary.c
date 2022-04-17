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
			return make_constant(1);
		}
		return make_constant(type_sizeof(sym->type));
	}
	/* Not a name.. should be a type */
	/* TODO */
	return make_constant(2);
}

struct node *constant_node(void)
{
	struct node *n;
	unsigned label;

	/* Strings are special */
	label = quoted_string(NULL);
	if (label) {
		/* We have a temporary name */
		n = make_label(label);
		n->type = CCHAR + 1;	/* PTR to CHAR */
		return n;
	}
	/* Numeric */
	n = make_constant(token_value);

	switch (token) {
	case T_INTVAL:
		n->type = CINT;
		break;
	case T_LONGVAL:
		n->type = CLONG;
		break;
	case T_UINTVAL:
		n->type = UINT;
		break;
	case T_ULONGVAL:
		n->type = ULONG;
		break;
	}
	next_token();
	return n;
}

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
		if (func && sym == NULL)
			/* Weird case you can call a function you've not declared */
			sym = update_symbol(name, S_EXTERN, C_FUNCTION	/*TODO | int() func */
			    );
		/* You can't size fields and structs by field/struct name without 
		   the type specifier */
		if (sym == NULL || sym->storage > S_EXTDEF) {
			fprintf(stderr, "Couldn't find %u %p\n", name, (void *)sym);
			error("unknown symbol");
			return make_constant(0);
		}
		/* Primary can be followed by operators and the caller handles those */
		return make_symbol(sym);
	}
	/* Not a name - constants or strings */
	return constant_node();
}
