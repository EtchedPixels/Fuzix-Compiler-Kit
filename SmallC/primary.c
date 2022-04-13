/*
 * File primary.c: 2.4 (84/11/27,16:26:07)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

struct node *primary(LVALUE *lval) {
    unsigned sname;
    struct node *l;
    int     num[1], symbol_table_idx, offset;
    SYMBOL *symbol;
    unsigned size;

    lval->ptr_type = 0;  // clear pointer/array type
    lval->tagsym = 0;
    if (match (T_LPAREN)) {
        l = hier1 (lval);
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
                if ((symbol->type & CINT) ||
                    (symbol->identity == POINTER))
                    offset *= INTSIZE;
                else if (symbol->type == STRUCT)
                    offset *= tag_table[symbol->tagidx].size;
                size = offset;
            } else {
                error("sizeof undeclared variable");
                size = 1;
            }
        } else {
            error("sizeof only on type or variable");
        }
        needbrack(T_RPAREN);
        newline();
        lval->symbol = 0;
        lval->indirect = 0;
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
                symbol_table_idx = add_global(sname, FUNCTION, CINT, 0, PUBLIC);
                symbol = &symbol_table[symbol_table_idx];
                lval->symbol = symbol;
                lval->indirect = 0;
                return make_symbol(symbol);
            }
        }
        symbol = &symbol_table[symbol_table_idx];

        if (local && gen_indirected(symbol)) {
            lval->symbol = symbol;
            lval->indirect = symbol->type;
            if (symbol->type == STRUCT) {
                lval->tagsym = &tag_table[symbol->tagidx];
            }
            if (symbol->identity == ARRAY ||
                (symbol->identity == VARIABLE && symbol->type == STRUCT)) {
                lval->ptr_type = symbol->type;
            } else if (symbol->identity == POINTER) {
                lval->indirect = CINT;
                lval->ptr_type = symbol->type;
            }
            return make_symbol(symbol);
        }

	/* Globals, function names */
	lval->symbol = symbol;
	lval->indirect = 0;
        if (symbol->identity != FUNCTION) {

	    /* Globals and anything we can directly access */
            if (symbol->type == STRUCT) {
                lval->tagsym = &tag_table[symbol->tagidx];
            }
            if (symbol->identity != ARRAY &&
                (symbol->identity != VARIABLE || symbol->type != STRUCT)) {
                if (symbol->identity == POINTER) {
                    lval->ptr_type = symbol->type;
                }
                return make_symbol(symbol);
            }

            lval->indirect = symbol->type;
            lval->ptr_type = symbol->type;
        } else {

	    /* Function call */
            lval->ptr_type = symbol->type;
	}
	return make_symbol(symbol);
    }
    lval->symbol = 0;
    lval->indirect = 0;
    l = constant_node(num);
    if (l == NULL) {
        error("invalid expression");
        return make_constant(0);
    }
    return l;
}

/**
 * true if val1 -> int pointer or int array and val2 not pointer or array
 * @param val1
 * @param val2
 * @return 
 */
int dbltest(LVALUE *val1, LVALUE *val2) {
    if (val1 == NULL)
        return (FALSE);
    if (val1->ptr_type) {
        if (val1->ptr_type & CCHAR)
            return (FALSE);
        if (val2->ptr_type)
            return (FALSE);
        return (TRUE);
    }
    return (FALSE);
}

/**
 * determine type of binary operation
 * @param lval
 * @param lval2
 * @return 
 */
void result(LVALUE *lval, LVALUE *lval2) {
    if (lval->ptr_type && lval2->ptr_type)
        lval->ptr_type = 0;
    else if (lval2->ptr_type) {
        lval->symbol = lval2->symbol;
        lval->indirect = lval2->indirect;
        lval->ptr_type = lval2->ptr_type;
    }
}

/* FIXME: typing */
struct node *constant_node(int val[]) {
    /* String... */
    if (quoted_string(NULL, val)) {
        /* We have a temporary name in val */
        return make_label(val[0]);
    }
    /* Numeric */
    switch(token) {
        /* Hack for now */
    case T_INTVAL:
    case T_LONGVAL:
    case T_UINTVAL:
    case T_ULONGVAL:
        val[0] = token_value;
        next_token();
        return make_constant(val[0]);
    default: return NULL;
    }
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
struct node *callfunction(struct symbol *sym, struct node *n) {
    if (sym)
        return tree(T_FUNCCALL, make_symbol(sym), funcargs());
    else
        return tree(T_FUNCCALL, n, funcargs());
}

void needlval(void) {
    error ("must be lvalue");
}

