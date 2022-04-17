
#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"

/*
 *	Read and match against the token stream. Need to move from stdio
 *	eventually
 */

unsigned line_num;

unsigned token_value;
unsigned token;

unsigned tokbyte(void)
{
	unsigned c = getchar();
	if (c == EOF) {
		error("corrupt stream");
		exit(1);
	}
	return c;
}

void next_token(void)
{
	int c = getchar();
	if (c == EOF) {
		token = T_EOF;
//        printf("*** EOF\n");
		return;
	}
	token = c;
	c = getchar();
	if (c == EOF) {
		token = T_EOF;
		return;
	}
	token |= (c << 8);

	if (token == T_LINE) {
		line_num = tokbyte();
		line_num |= tokbyte() << 8;
		next_token();
		return;
	}

	if (token == T_INTVAL || token == T_LONGVAL || token == T_UINTVAL
	    || token == T_ULONGVAL) {
		token_value = tokbyte();
		token_value |= tokbyte() << 8;
		/* Throw the upper word for now */
		tokbyte();
		tokbyte();
	}
}


void junk(void)
{
	while (token != T_EOF && token != T_SEMICOLON)
		next_token();
	next_token();
}

unsigned match(unsigned t)
{
	if (t == token) {
		next_token();
		return 1;
	}
	return 0;
}

void need_semicolon(void)
{
	if (!match(T_SEMICOLON)) {
		error("missing semicolon");
		junk();
	}
}

void require(unsigned t)
{
	if (!match(t))
		errorc(t, "expected");
}

unsigned symname(void)
{
	unsigned t;
	if (token < T_SYMBOL)
		return 0;
	t = token;
	next_token();
	return t;
}

/*
 *	This is ugly and we need to review how we handle it
 */

static unsigned string_tag;

unsigned quoted_string(int *len)
{
	unsigned c;
	unsigned l = 0;
	unsigned label = ++string_tag;

	if (token != T_STRING)
		return 0;

	header(H_STRING, label, 0);

	while ((c = tokbyte()) != 0) {
		if (c == 255)
			c = tokbyte();
//TODO        output_number(c);
		l++;
	}
	l++;
//TODO    output_number(0);

	footer(H_STRING, label, l);

	next_token();
	if (token != T_STRING_END)
		error("bad token stream");
	next_token();

	if (len)
		*len = l;
	return label;
}
