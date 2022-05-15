#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"

/*
 *	Read and match against the token stream. Need to move from stdio
 *	eventually
 */

#define NO_TOKEN	0xFFFF		/* An unused value */

char filename[16];

unsigned line_num;

unsigned long token_value;
unsigned token;
unsigned last_token = NO_TOKEN;

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
	int c;

	/* Handle pushed back tokens */
	if (last_token != NO_TOKEN) {
		token = last_token;
		last_token = NO_TOKEN;
		return;
	}

	c = getchar();
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
		char *p = filename;

		line_num = tokbyte();
		line_num |= tokbyte() << 8;

		if (line_num & 0x8000) {
			line_num &= 0x7FFF;
			for (c = 0; c < 16; c++) {
				*p = tokbyte();
				if (*p == 0)
					break;
				p++;
			}
		}
		next_token();
		return;
	}

	if (token == T_INTVAL || token == T_LONGVAL || token == T_UINTVAL
	    || token == T_ULONGVAL || token == T_FLOATVAL) {
		token_value = tokbyte();
		token_value |= tokbyte() << 8;
		token_value |= tokbyte() << 16;
		token_value |= tokbyte() << 24;
	}
}

/*
 * You can only push back one token and it must not have attached data. This
 * works out fine because we only ever need to push back a name when processing
 *  labels
 */
void push_token(unsigned t)
{
	last_token = token;
	token = t;
}

/*
 *	Try and move on a bit so that we don't generate a wall of errors for
 *	a single mistake
 */
void junk(void)
{
	while (token != T_EOF && token != T_SEMICOLON)
		next_token();
	next_token();
}

/*
 *	If the token is the one expected then consume it and return 1, if not
 *	do not consume it and leave 0. This lets us write things like
 *
 *	if (match(T_STAR)) { ... }
 */
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

/* This can only be used if the token is a single character token. That turns
   out to be sufficient for C so there is no need for anything fancy here */
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
		write(1, &c, 1);
		l++;
	}
	l++;
	write(1, &c, 1);

	footer(H_STRING, label, l);

	next_token();
	if (token != T_STRING_END)
		error("bad token stream");
	next_token();

	if (len)
		*len = l;
	return label;
}
