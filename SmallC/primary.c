/*
 * File primary.c: 2.4 (84/11/27,16:26:07)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

int primary(LVALUE *lval) {
    unsigned sname;
    int     num[1], k, symbol_table_idx, offset, reg;
    SYMBOL *symbol;

    lval->ptr_type = 0;  // clear pointer/array type
    lval->tagsym = 0;
    if (match (T_LPAREN)) {
        k = hier1 (lval);
        needbrack (T_RPAREN);
        return (k);
    }
    if (match(T_SIZEOF)) {
        needbrack(T_LPAREN);
        gen_immediate();
        if (match(T_INT)) output_number(INTSIZE);
        else if (match(T_CHAR)) output_number(1);
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
                output_number(offset);
            } else {
                error("sizeof undeclared variable");
                output_number(0);
            }
        } else {
            error("sizeof only on type or variable");
        }
        needbrack(T_RPAREN);
        newline();
        lval->symbol = 0;
        lval->indirect = 0;
        return(0);
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
                return 0;
            }
        }
        symbol = &symbol_table[symbol_table_idx];

        if (local && gen_indirected(symbol)) {
            reg = gen_get_locale(symbol);
            lval->symbol = symbol;
            lval->indirect = symbol->type;
            if (symbol->type == STRUCT) {
                lval->tagsym = &tag_table[symbol->tagidx];
            }
            if (symbol->identity == ARRAY ||
                (symbol->identity == VARIABLE && symbol->type == STRUCT)) {
                lval->ptr_type = symbol->type;
                return reg;
            }
            if (symbol->identity == POINTER) {
                lval->indirect = CINT;
                lval->ptr_type = symbol->type;
            }
            return FETCH | reg;
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
                return FETCH | HL_REG;
            }

            if (symbol->storage == LSTATIC) {
                gen_get_locale(symbol);
            } else {
                gen_immediate();
                output_label_name(symbol->name);
                newline();
            }
            lval->indirect = symbol->type;
            lval->ptr_type = symbol->type;
        } else {

	    /* Function call */
            lval->ptr_type = symbol->type;
	}
	return 0;
    }
    lval->symbol = 0;
    lval->indirect = 0;
    if (constant(num))
        return 0;
    else {
        error("invalid expression");
        gen_immediate();
        output_number(0);
        newline();
        junk();
        return 0;
    }
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

int constant(int val[]) {
    if (number (val))
        gen_immediate ();
    /* Quoted strings are constants so we don't need to do any mucking about
       with segments - however we move them to data as we'd otherwise put
       them mid code stream ! */
    else if (quoted_string (NULL, val)) {
        gen_immediate ();
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
 * perform a function call
 * called from "hier11", this routine will either call the named
 * function, or if the supplied ptr is zero, will call the contents
 * of HL
 * @param ptr name of the function
 */
void callfunction(unsigned ptr) {
    int     nargs;

    nargs = 0;

    if (ptr == 0)
        gen_push (HL_REG);
    /* WTF ?FIXME */
    while (token != T_RPAREN) {
        if (endst ())
            break;
        expression (NO);
        if (ptr == 0)
            gen_swap_stack ();
        /* Worth making the first argument pass in HL ?? */
        gen_push (HL_REG);
        nargs++;
        /* Will need to track sizes later */
        if (!match (T_COMMA))
            break;
    }
    needbrack (T_RPAREN);
    if (aflag)
        gnargs(nargs);
    if (ptr)
        gen_call (ptr);
    else
        callstk ();
    stkp = gen_modify_stack (stkp + nargs * INTSIZE);
}

void needlval(void) {
    error ("must be lvalue");
}

