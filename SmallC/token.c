/*
 *	C tokenizer. Do our best not to store stuff temporarily
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "defs.h"
#include "data.h"

struct token tok;		/* Live token */
static struct token prev;	/* For push */
static uint8_t pushed;
static char sym_name[33];	/* Symbol name */

enum tok_t {
    /* Keywords */
    TOK_AUTO,
    TOK_BREAK,
    TOK_CASE,
    TOK_CONTINUE,
    TOK_DEFAULT,
    TOK_DO,
    TOK_ELSE,
    TOK_EXTERN,
    TOK_FOR,
    TOK_GOTO,
    TOK_IF,
    TOK_RETURN,
    TOK_SIZEOF,
    TOK_SWITCH,
    TOK_TYPEDEF,
    TOK_UNION,
    TOK_VOLATILE,
    TOK_WHILE,

    /* Keywords that are type modifiers */    
    TOK_CONST,
    TOK_REGISTER,
    TOK_SIGNED,
    TOK_STATIC,
    TOK_UNSIGNED,

    /* Keywords that are types */
    TOK_CHAR,
    TOK_DOUBLE,
    TOK_ENUM,
    TOK_FLOAT,
    TOK_INT,
    TOK_LONG,
    TOK_SHORT,
    TOK_STRUCT,
    TOK_VOID

    /* Operators */
    TOK_PLUS,
    TOK_MINUS,
    TOK_LT,
    TOK_GT,
    TOK_AND,
    TOK_OR,
    TOK_XOR,
    TOK_TILDE,
    TOK_MUL,
    TOK_DIV,
    TOK_NOT,
    TOK_SEMI,
    TOK_DOT,
    TOK_LPAR,
    TOK_RPAR,
    TOK_LSQUARE,
    TOK_RSQUARE,
    TOK_LBRA,
    TOK_RBRA,
    TOK_COLON,
    TOK_COMMA.
    
    TOK_PLUSPLUS,
    TOK_MINUSMINUS,
    TOK_LSHIFT,
    TOK_RSHIFT,
    TOK_LAND,
    TOK_LOR,
    TOK_LXOR,

    TOK_PLUSEQ,
    TOK_MINUSEQ,
    TOK_LTEQ,
    TOK_GTEQ,
    TOK_ANDEQ,
    TOK_OREQ,
    TOK_XOREQ,
    TOK_TILDEEQ,
    TOK_MULEQ,
    TOK_DIVEQ,
    TOK_NOTEQ,
    
    /* The three byte specials */
    TOK_LSHIFTEQ,
    TOK_RSHIFTEQ,

    /* The oddities */
    TOK_MINUSGT,
    TOK_DOTDOTDOT,

    /* Everything else */
    TOK_STRING,
    TOK_SYMNAME,
    TOK_VALUE
    
};

/* Caution required - we have ambiguous symbols like * and [] which may
   be type modifiers (eg char * const, or int x[]) */
#define TOKEN_KEYWORD(x)	((x) <= TOK_WHILE)
#define TOKEN_TYPEMOD(x)	((x) >= TOK_CONST && (x) <= TOK_UNSIGNED)
#define TOKEN_TYPE(x)		((x) >= CHAR && (x) <= VOID)

    

static const char onetokens[]={
    "+-<>&|^"				/* Doubling symbols first */
    "~*/!"				/* Valid with = */
    ";.()[]{}:,"			/* Only in single form */
};

static uint8_t doubles[] = {
    TOK_PLUSPLUS,
    TOK_MINUSMINUS,
    TOK_LSHIFT,
    TOK_RSHIFT,
    TOK_LAND,
    TOK_LOR,
    TOK_LXOR,
};

static uint8_t equals[] = {
    TOK_PLUSEQ,
    TOK_MINUSEQ,
    TOK_LTEQ,,
    TOK_GTEQ,
    TOK_ANDEQ,
    TOK_ORQEQ,
    TOK_XOREQ,
    TOK_TILDEEQ,
    TOK_MULEQ,
    TOK_DIVEQ,
    TOK_NOTEQ
};

uint16_t next_token(void)
{
    char *s = sym_name;
    uint8_t c;

    if (pushed) {
        memcpy(&tok, &prev, sizeof(tok));
        pushed = 0;
        return tok.token;
    }
retry:
    while(*inptr && isspace(*inptr))
        inptr++;
    if (!*inptr) {
        tok.token = TOK_EOF;
        return TOK_EOF;
    }
    /* We have a clear divide betwen _[a-z][A-Z] keywords and symbols
       and other tokens */
    if (issymchar(t)) {
    }
    /* Quoting */
    if (*inptr == '\'') {
    }
    /* Strings */
    if (*inptr == '"') {
    }
    /* Numbers */
    if (isdigit(*inptr)) {
        if (*inptr == '0') {
            inptr++;
            if (*inptr == 'x' || *inptr == 'X')
                return hextoken(p);
            return octtoken(p);
        }
        return dectoken(p);
    }
    /* See if this is a valid leading token code */
    t = strchr(onetokens, *inptr);
    if (t == NULL) {
        error("unexpected character");
        inptr++;
        goto retry;
    }
    /* Get the token number */
    c = t - onetokens;

    if (c == TOK_DOT && memcmp(inptr, "...", 3) == 0) {
        tok.token = TOK_DOTDOTDOT;
        tok.type = TOK_SPECIAL;
    }
    /* Cover two byte codes like == and ++ */
    if (*inptr == inptr[1] && c < sizeof(doubles) && doubles[c]) {
        inptr += 2;
        c = doubles[c];
        /* The special case three byte tokens */
        if (*inptr == '=') {
            if (c == TOK_LT) {
                inptr++;
                c = TOK_RSHIFTEQ;
            }
            else if (c == TOK_GT) {
                inptr++;
                c = TOK_RSHIFTEQ;
            }
        }
    }
    else
        inptr++;
    /* >= <= &= etc */
    if (*inptr == '=' &&  c < sizeof(equals) && equals[c]) {
        t.token = equals[c];
        inptr++;
    }
    if (c == TOK_MINUS && *inptr == '>') {
        c = TOK_MINUSGT;
        inptr++;
    }
    tok.token = c;
    tok.type = toktype[c];
    tok.data = 0;
    return tok.token;
}
