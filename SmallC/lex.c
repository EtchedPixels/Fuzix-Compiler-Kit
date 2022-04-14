/*
 *	Read and match against the token stream
 */
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "data.h"

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

    if (token == T_INTVAL || token == T_LONGVAL || token == T_UINTVAL ||
        token == T_ULONGVAL) {
        token_value = tokbyte();
        token_value |= tokbyte() << 8;
        /* Throw the upper word for now */
        tokbyte();
        tokbyte();
    }
}

/**
 * semicolon enforcer
 * called whenever syntax requires a semicolon
 */
void need_semicolon(void) {
    if (!match (T_SEMICOLON)) {
        error ("missing semicolon");
        /* Try and recover by eating until some kind of structural element */
        while(token != T_EOF && token != T_RCURLY && token != T_SEMICOLON)
            next_token();
    }
}

void junk(void) {
    next_token();
}

int endst(void) {
    if (token == T_EOF)
        return 1;
    if (match(T_SEMICOLON))
        return 1;
    return 0;
}

/**
 * enforces bracket
 * @param str
 * @return 
 */
void needbrack(unsigned brack) {
    if (!match (brack)) {
        error ("missing bracket");
//        output_string (str);
        newline ();
    }
}

/**
 * looks for a match between a token and the current token in
 * the input line. It skips over the token and returns true if a match occurs
 * otherwise it retains the current position in the input line and returns false
 * @param lit
 * @return 
 */
int match(unsigned t) {
    if (t == token) {
        next_token();
        return 1;
    }
    return 0;
}

