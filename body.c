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
	statement();
	footer(H_FOR, cont_tag, break_tag);

	break_tag = oldbrk;
	cont_tag = oldcont;
}

static void return_statement(void)
{
	next_token();
	header(H_RETURN, func_tag, 0);
	expression_or_null(0, 0);
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

	switch_tag = next_tag++;
	break_tag = next_tag++;
	switch_count = 0;

	next_token();
	header(H_SWITCH, switch_tag, break_tag);
	bracketed_expression(0);
	statement_block(0);
	footer(H_SWITCH, switch_tag, break_tag);

	break_tag = oldbrk;
	switch_tag = oldswt;
	switch_count = oldswc;
}

static void case_statement(void)
{
	if (switch_tag == 0)
		error("case outside of switch");
	header(H_CASE, switch_tag, switch_count++);
	next_token();
	const_expression();
	require(T_COLON);
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
	/* TODO */
	if ((n = symname()) == 0)
		error("label required");
	need_semicolon();
}

static void statement(void)
{
	/* Classic C requires variables at the block start, C99 doesn't. Whilst
	   the C99 approach is ugly allow it */
	if (is_modifier() || is_storage_word() || is_type_word()) {
		declaration(AUTO);
		/* need_semicolon();	CHECK TODO */
		return;
	}
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
		/* TODO labels */
		/* Expressions */
		write_tree(expression_tree(1));
		break;
	}
}

void statement_block(unsigned need_brack)
{
	/* TODO: we need to change the frame allocation to track a max
	   but pop on block exits so we can overlay frames */
	struct symbol *ltop;
	if (token == T_EOF) {
		error("unexpected EOF");
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
	while (token != T_RCURLY) {
		/* declarations */
		/* statements */
		statement_block(0);
	}
	pop_local_symbols(ltop);
	next_token();
}

void function_body(unsigned st, unsigned name, unsigned type)
{
	/* We need to add ourselves to the symbols first as we can self
	   reference */
	struct symbol *sym;
	unsigned n, m;
	/* This makes me sad, but there isn't a nice way to work out
	   the frame size ahead of time */
	off_t hrw;
	if (st == AUTO || st == EXTERN)
		error("invalid storage class");
	sym = update_symbol(name, st, type);
	func_tag = next_tag++;
	header(H_FUNCTION, st, name);
	hrw = mark_header();
	header(H_FRAME, 0, name);
	statement_block(1);
	footer(H_FUNCTION, st, name);
	/* FIXME: need to clean this up and nicely access the max frame
	   offset */
	mark_storage(&n, &m);
	rewrite_header(hrw, H_FRAME, n, name);
	func_tag = 0;
}
