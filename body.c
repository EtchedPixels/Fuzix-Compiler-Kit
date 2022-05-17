#include <stdio.h>
#include "compiler.h"

void statement_block(unsigned brack);
static void statement(void);

static unsigned next_tag;
static unsigned func_tag;
static unsigned break_tag;
static unsigned cont_tag;
static unsigned switch_tag;
static unsigned switch_count;
static unsigned switch_type;
static unsigned func_type;

/* C keyword statements */
static void if_statement(void)
{
	unsigned tag = next_tag++;
	header(H_IF, tag, 0);
	next_token();
	bracketed_expression(1);
	statement_block(0);
	if (token == T_ELSE) {
		next_token();
		header(H_ELSE, tag, 0);
		statement_block(0);
		footer(H_IF, tag, 1);
	} else
		footer(H_IF, tag, 0);
}

static void while_statement(void)
{
	unsigned oldbrk = break_tag;
	unsigned oldcont = cont_tag;

	break_tag = next_tag++;
	cont_tag = next_tag++;

	next_token();
	header(H_WHILE, cont_tag, break_tag);
	bracketed_expression(1);
	statement_block(0);
	footer(H_WHILE, cont_tag, break_tag);

	break_tag = oldbrk;
	cont_tag = oldcont;
}

static void do_statement(void)
{
	unsigned oldbrk = break_tag;
	unsigned oldcont = cont_tag;

	break_tag = next_tag++;
	cont_tag = next_tag++;

	next_token();
	header(H_DO, cont_tag, break_tag);
	statement_block(0);
	require(T_WHILE);
	header(H_WHILE, cont_tag, break_tag);
	bracketed_expression(1);
	require(T_SEMICOLON);
	footer(H_DO, cont_tag, break_tag);

	break_tag = oldbrk;
	cont_tag = oldcont;
}

static void for_statement(void)
{
	unsigned oldbrk = break_tag;
	unsigned oldcont = cont_tag;

	break_tag = next_tag++;
	cont_tag = next_tag++;

	next_token();
	header(H_FOR, cont_tag, break_tag);
	require(T_LPAREN);
	expression_or_null(0, 1);
	require(T_SEMICOLON);
	expression_or_null(1, 0);
	require(T_SEMICOLON);
	expression_or_null(0, 1);
	require(T_RPAREN);
	statement_block(0);
	footer(H_FOR, cont_tag, break_tag);

	break_tag = oldbrk;
	cont_tag = oldcont;
}

static void return_statement(void)
{
	next_token();
	header(H_RETURN, func_tag, 0);
	expression_typed(func_type);
	footer(H_RETURN, func_tag, 0);
	need_semicolon();
}

static void break_statement(void)
{
	next_token();
	if (break_tag == 0)
		error("break outside of block");
	header(H_BREAK, break_tag, 0);
	need_semicolon();
}

static void continue_statement(void)
{
	next_token();
	if (cont_tag == 0)
		error("continue outside of block");
	header(H_CONTINUE, cont_tag, 0);
	need_semicolon();
}

static void switch_statement(void)
{
	unsigned oldbrk = break_tag;
	unsigned oldswt = switch_tag;
	unsigned oldswc = switch_count;
	unsigned oldswtype = switch_type;
	unsigned long *swptr;

	switch_tag = next_tag++;
	break_tag = next_tag++;
	switch_count = 0;

	next_token();
	header(H_SWITCH, switch_tag, break_tag);
	switch_type = bracketed_expression(0);

	/* Only integral types */
	if (!IS_INTARITH(switch_type)) {
		error("bad type");
		switch_type = CINT;
	}

	swptr = switch_alloc();

	statement_block(0);
	footer(H_SWITCH, switch_tag, break_tag);

	switch_done(switch_tag, swptr, switch_type);

	switch_type = oldswtype;
	break_tag = oldbrk;
	switch_tag = oldswt;
	switch_count = oldswc;
}

static void case_statement(void)
{
	struct node *n;
	if (switch_tag == 0)
		error("case outside of switch");
	next_token();
	n = expression_tree(0);
	if (!is_constant(n))
		error("not constant");
	else {
		switch_add_node(n->value);
	}
	free_tree(n);
	header(H_CASE, switch_tag, switch_count);
	require(T_COLON);
	switch_count++;
	statement_block(0);
}

static void default_statement(void)
{
	if (switch_tag == 0)
		error("default outside of switch");
	header(H_DEFAULT, switch_tag, 0);
	next_token();
	require(T_COLON);
	statement_block(0);
}

static void typedef_statement(void)
{
	/* TODO */
}

static void goto_statement(void)
{
	unsigned n;
	next_token();
	if ((n = symname()) == 0)
		error("label required");
	/* We will work out if the label existed later */
	use_label(n);
	header(H_GOTO, n, 0);
	need_semicolon();
}

/*
 *	C statements.
 *
 *	This can be a declaration, in which case it starts with a token that
 *	describes storage properties, a keyword, a name followed by a colon
 *	(which is a label), or an expression. A null expression is also allowed.
 */
static void statement(void)
{
	/* It's valid to have a {} block */
	if (token == T_RCURLY)
		return;

#if 0	/* C99 for later if we want it */
	declaration_block();
#endif
	/* Check for keywords */
	switch (token) {
	case T_IF:
		if_statement();
		break;
	case T_WHILE:
		while_statement();
		break;
	case T_SWITCH:
		switch_statement();
		break;
	case T_DO:
		do_statement();
		break;
	case T_FOR:
		for_statement();
		break;
	case T_RETURN:
		return_statement();
		break;
	case T_BREAK:
		break_statement();
		break;
	case T_CONTINUE:
		continue_statement();
		break;
	case T_GOTO:
		goto_statement();
		break;
	case T_CASE:
		case_statement();
		break;
	case T_DEFAULT:
		default_statement();
		break;
	case T_TYPEDEF:
		typedef_statement();
		break;
	case T_SEMICOLON:
		next_token();
		break;
	default:
		/* We could write this not to push back a token but it's
		   actually much cleaner to push back */
		while (token >= T_SYMBOL) {
			unsigned name = token;
			next_token();
			if (token == T_COLON) {
				next_token();
				/* We found a label */
				add_label(name);
				header(H_LABEL, name, 0);
			} else {
				push_token(name);
				break;
			}
		}
		/* It is valid to follow a label with just ; */
		if (token == T_SEMICOLON)
			next_token();
		else {
			struct node *n = expression_tree(1);
			/* A statement top node need not worry about
			   generating the correct result */
			n->flags |= NORETURN;
			write_tree(n);
		}
		break;
	}
}

static void declaration_block(void)
{
	while (is_modifier() || is_storage_word() || is_type_word() ||
			is_typedef()) {
		declaration(S_AUTO);
	}
}

/*
 *	Either a statement or a sequence of statements enclosed in { }. In
 *	some cases the sequence is mandatory (eg a function) so we pass in
 *	need_brack to tell us what to do.
 */
void statement_block(unsigned need_brack)
{
	struct symbol *ltop;
	if (token == T_EOF) {
		fatal("unexpected EOF");
		return;
	}
	if (token != T_LCURLY) {
		if (need_brack)
			require(T_LCURLY);
		statement();
		return;
	}
	next_token();
	ltop = mark_local_symbols();
	/* declarations */
	declaration_block();
	while (token != T_RCURLY) {
		/* statements */
		statement_block(0);
	}
	pop_local_symbols(ltop);
	next_token();
}

/*
 *	We have parsed the declaration part of a function and found it
 *	is followed by a body. Set up the headersfor the backend and turn
 *	the contents into expressions and headers.
 */
void function_body(unsigned st, unsigned name, unsigned type)
{
	/* This makes me sad, but there isn't a nice way to work out
	   the frame size ahead of time */
	unsigned long hrw;

	if (st == S_AUTO || st == S_EXTERN)
		error("invalid storage class");
	func_tag = next_tag++;
	func_type = type;
	header(H_FUNCTION, func_tag, name);
	hrw = mark_header();
	header(H_FRAME, 0, name);

	init_labels();

	statement_block(1);
	footer(H_FUNCTION, func_tag, name);

	rewrite_header(hrw, H_FRAME, frame_size(), name);
	check_labels();
}
