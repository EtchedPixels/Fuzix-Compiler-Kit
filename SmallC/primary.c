/*
 * File primary.c: 2.4 (84/11/27,16:26:07)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

struct node *primary(void) {
    unsigned sname;
    struct node *l;
    int     num[1], symbol_table_idx, offset;
    SYMBOL *symbol;
    unsigned size;
    unsigned scale = 1;

    if (match (T_LPAREN)) {
        l = hier1 ();
        needbrack (T_RPAREN);
        return (l);
    }
    if (match(T_SIZEOF)) {
        needbrack(T_LPAREN);
        if (match(T_INT))
            size = INTSIZE;
        else if (match(T_CHAR))
            size = 1;
        /* TODO: sizeof struct foo, local static etc */
        else if ((sname = symname()) != 0) {
            if (((symbol_table_idx = find_locale(sname)) > -1) ||
                ((symbol_table_idx = find_global(sname)) > -1)) {
                symbol = &symbol_table[symbol_table_idx];
                if (symbol->storage == LSTATIC)
                    error("sizeof local static");
                offset = symbol->offset;
                /* FIXME: double check which depth of indirection */
                scale = type_ptrscale(symbol->type);
#if 0
                if ((symbol->type & CINT) ||
                    (symbol->identity == POINTER))
                    offset *= INTSIZE;
                else if (symbol->type == STRUCT)
                    offset *= tag_table[symbol->tagidx].size;
#endif
                size = offset * scale;
            } else {
                error("sizeof undeclared variable");
                size = 1;
            }
        } else {
            error("sizeof only on type or variable");
        }
        needbrack(T_RPAREN);
        return make_constant(size);
    }
    if ((sname = symname ()) != 0) {
        int local = 1;
        symbol_table_idx = find_locale(sname);
        if (symbol_table_idx == -1) {
            local = 0;
            symbol_table_idx = find_global(sname);
            /* Only valid undeclared name is a function reference */
            if (symbol_table_idx == -1) {
                if (token != T_LPAREN)
                    error("undeclared variable");
                symbol_table_idx = add_global(sname, FUNCTION, CINT, PUBLIC);
                symbol = &symbol_table[symbol_table_idx];
                return make_symbol(symbol);
            }
        }
        symbol = &symbol_table[symbol_table_idx];

        if (local && gen_indirected(symbol)) {
            return make_symbol(symbol);
        }
	return make_symbol(symbol);
    }
    l = constant_node(num);
    if (l == NULL) {
        error("invalid expression");
        return make_constant(0);
    }
    return l;
}

struct node *constant_node(int val[]) {
    struct node *n;
    /* String... */
    if (quoted_string(NULL, val)) {
        /* We have a temporary name in val */
        n = make_label(val[0]);
        n->type = CCHAR + 1;	/* PTR to CHAR */
        return n;
    }
    /* Numeric */
    val[0] = token_value;
    next_token();
    n = make_constant(val[0]);

    switch(token) {
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
    return n;
}

int constant(int val[]) {
    if (number (val));
    /* Quoted strings are constants so we don't need to do any mucking about
       with segments - however we move them to data as we'd otherwise put
       them mid code stream ! */
    else if (quoted_string (NULL, val)) {
        print_label (val[0]);
        newline();
        return 1;
    } else
        return (0);
    output_number (val[0]);
    newline ();
    return (1);
}

int number(int val[]) {
    switch(token) {
    case T_INTVAL:
    case T_LONGVAL:
        val[0] = token_value;
        next_token();
        return CINT;
    case T_UINTVAL:
    case T_ULONGVAL:
        val[0] = token_value;
        next_token();
        return UINT;
    }
    return 0;
}

int quoted_string(int *len, unsigned *position)
{
    unsigned c;
    unsigned l = 0;
    unsigned int count = 0;
    if (token != T_STRING)
        return 0;
    if (position) {
        data_segment_gdata();
        *position = getlabel();
        generate_label(*position);
    }
    while((c = tokbyte()) != 0) {
        if (c == 255)
            c = tokbyte();
        if (count == 0)
            gen_def_byte();
        else
            output_byte(',');
        output_number(c);
        if (count++ == 7) {
            count = 0;
            newline();
        }
        l++;
    }
    if (count != 0)
        newline();
    l++;
    gen_def_byte();
    output_number(0);
    newline();

    if (position)
        code_segment_gtext();
    next_token();
    if (token != T_STRING_END)
        error("bad token stream");
    next_token();
    if (len)
        *len = l;
    return 1;
}

/**
 * parse function argument values
 */

struct node *funcargs(void)
{
    struct node *n = expression(NO);
    if (match(T_COMMA))
        /* Switch around for calling order */
        return tree(T_COMMA, funcargs(), n);
    needbrack(T_RPAREN);
    return n;
}

/**
 * perform a function call
 * @sym: symbol to call
 * @node: node to call if not a symbol call
 */
struct node *callfunction(struct node *n) {
    /* Is this stuff needed any more */
    if (n->sym && IS_FUNCTION(n->sym->type))
        return tree(T_FUNCCALL, make_symbol(n->sym), funcargs());
    else
        return tree(T_FUNCCALL, n, funcargs());
}

void needlval(void) {
    error ("must be lvalue");
}

