/*
 * File expr.c: 2.2 (83/06/21,11:24:26)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

/**
 * unsigned operand ?
 */
int nosign(LVALUE *is) {
    SYMBOL *ptr;
    
    if((is->ptr_type) ||
      ((ptr = is->symbol) && (ptr->type & UNSIGNED))) {
        return 1;
    }
    return 0;
}

/**
 * lval.symbol - symbol table address, else 0 for constant
 * lval.indirect - type indirect object to fetch, else 0 for static object
 * lval.ptr_type - type pointer or array, else 0
 * @param comma
 * @return 
 */
void expression(int comma) {
    LVALUE lval;
    int k;

    do {
        k = hier1 (&lval);
        if (k & FETCH)
            rvalue(&lval, k);
        if (!comma)
            return;
    } while (match (T_COMMA));
}

/**
 * assignment operators
 * @param lval
 * @return 
 */
int hier1 (LVALUE *lval) {
    int     k;
    LVALUE lval2[1];
    unsigned fc;

    k = hier1a (lval);
    if (match (T_EQ)) {
        if ((k & FETCH) == 0) {
            needlval ();
            return (0);
        }
        if (lval->indirect)
            gen_push(k);
        k = hier1 (lval2);
        if (k & FETCH)
            k = rvalue(lval2, k);
        store (lval);
        return (0);
    } else {      
        fc = token;
        if  (match (T_MINUSEQ) ||
            match (T_PLUSEQ) ||
            match (T_STAREQ) ||
            match (T_SLASHEQ) ||
            match (T_PERCENTEQ) ||
            match (T_SHREQ) ||
            match (T_SHLEQ) ||
            match (T_ANDEQ) ||
            match (T_HATEQ) ||
            match (T_OREQ)) {
            if ((k & FETCH) == 0) {
                needlval ();
                return (0);
            }
            if (lval->indirect)
                gen_push(k);
            k = rvalue(lval, k);
            gen_push(k);
            k = hier1 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            switch (fc) {
                case T_MINUSEQ: {
                    if (dbltest(lval,lval2)) {
                        gen_multiply(lval->ptr_type, lval->tagsym ? lval->tagsym->size : INTSIZE);
                    }
                    gen_sub();
                    result (lval, lval2);
                    break;
                }
                case T_PLUSEQ: {
                    if (dbltest(lval,lval2)) {
                        gen_multiply(lval->ptr_type, lval->tagsym ? lval->tagsym->size : INTSIZE);
                    }
                    gen_add (lval,lval2);
                    result(lval,lval2);
                    break;
                }
                case T_STAREQ:
                    gen_mult ();
                    break;
                case T_SLASHEQ:
                    if(nosign(lval) || nosign(lval2)) {
                        gen_udiv();
                    } else {
                        gen_div();
                    }
                    break;
                case T_PERCENTEQ:
                    if(nosign(lval) || nosign(lval2)) {
                        gen_umod();
                    } else {
                        gen_mod();
                    }
                    break;
                case T_SHREQ:
                    if (nosign(lval)) {
                        gen_logical_shift_right();
                    } else {
                        gen_arithm_shift_right();
                    }
                    break;
                case T_SHLEQ: gen_arithm_shift_left(); break;
                case T_ANDEQ: gen_and (); break;
                case T_HATEQ: gen_xor (); break;
                case T_OREQ: gen_or (); break;
            }
            store (lval);
            return (0);
        } else
            return (k);
    }
}

/**
 * processes ? : expression
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier1a (LVALUE *lval) {
    int     k, lab1, lab2;
    LVALUE lval2[1];

    k = hier1b (lval);
    if (token != T_QUESTION)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER
        if (match (T_QUESTION)) {
            gen_test_jump (lab1 = getlabel (), FALSE);
            k = hier1b (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            gen_jump (lab2 = getlabel ());
            print_label (lab1);
            output_label_terminator ();
            newline ();
            if (!match (T_COLON)) {
                error ("missing colon");
                return (0);
            }
            k = hier1b (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            print_label (lab2);
            output_label_terminator ();
            newline ();
        } else
            return (0);
}

/**
 * processes logical or ||
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier1b (LVALUE *lval) {
    int     k, lab;
    LVALUE lval2[1];

    k = hier1c (lval);

    if (token != T_OROR)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER
        if (match (T_OROR)) {
            gen_test_jump (lab = getlabel (), TRUE);
            k = hier1c (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            print_label (lab);
            output_label_terminator ();
            newline ();
            gen_convert_primary_reg_value_to_bool();
        } else
            return (0);
}

/**
 * processes logical and &&
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier1c (LVALUE *lval) {
    int     k, lab;
    LVALUE lval2[1];

    k = hier2 (lval);

    if (token != T_ANDAND)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER
        if (match (T_ANDAND)) {
            gen_test_jump (lab = getlabel (), FALSE);
            k = hier2 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            print_label (lab);
            output_label_terminator ();
            newline ();
            gen_convert_primary_reg_value_to_bool();
        } else
            return (0);
}

/**
 * processes bitwise or |
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier2 (LVALUE *lval) {
    int     k;
    LVALUE lval2[1];

    k = hier3 (lval);
    if (token != T_OR)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER {
        if (match(T_OR)) {
            gen_push(k);
            k = hier3 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            gen_or ();
    } else
            return (0);
    }
}

/**
 * processes bitwise exclusive or
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier3 (LVALUE *lval) {
    int     k;
    LVALUE lval2[1];

    k = hier4 (lval);
    if (token != T_HAT)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER {
            if (match(T_HAT)) {
                gen_push(k);
                k = hier4 (lval2);
                if (k & FETCH)
                    k = rvalue(lval2, k);
                gen_xor ();
            } else
                return (0);
    }
}

/**
 * processes bitwise and &
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier4 (LVALUE *lval) {
    int     k;
    LVALUE lval2[1];

    k = hier5 (lval);
    if (token != T_AND)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER {
        if (match(T_AND)) {
            gen_push(k);
            k = hier5 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            gen_and ();
        } else
            return (0);
    }

}

/**
 * processes equal and not equal operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier5 (LVALUE *lval) {
    int     k;
    LVALUE lval2[1];

    k = hier6 (lval);

    if (token != T_EQEQ && token != T_BANGEQ)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER {
        if (match (T_EQEQ)) {
            gen_push(k);
            k = hier6 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            gen_equal ();
        } else if (match (T_BANGEQ)) {
            gen_push(k);
            k = hier6 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            gen_not_equal ();
        } else
            return (0);
    }

}

/**
 * comparison operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier6 (LVALUE *lval) {
    int     k;
    LVALUE lval2[1];

    k = hier7 (lval);

    if (token != T_LT && token != T_GT && token != T_LTEQ && token != T_GTEQ)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER {
        if (match (T_LTEQ)) {
            gen_push(k);
            k = hier7 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            if (nosign(lval) || nosign(lval2)) {
                gen_unsigned_less_or_equal ();
                continue;
            }
            gen_less_or_equal ();
        } else if (match (T_GTEQ)) {
            gen_push(k);
            k = hier7 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            if (nosign(lval) || nosign(lval2)) {
                gen_unsigned_greater_or_equal ();
                continue;
            }
            gen_greater_or_equal();
        } else if (match(T_LT)) {
            gen_push(k);
            k = hier7 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            if (nosign(lval) || nosign(lval2)) {
                gen_unsigned_less_than ();
                continue;
            }
            gen_less_than ();
        } else if (match(T_GT)) {
            gen_push(k);
            k = hier7 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            if (nosign(lval) || nosign(lval2)) {
                gen_usigned_greater_than ();
                continue;
            }
            gen_greater_than();
        } else
            return (0);
    }

}

/**
 * bitwise left, right shift
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier7 (LVALUE *lval) {
    int     k;
    LVALUE lval2[1];

    k = hier8(lval);

    if (token != T_GTGT && token != T_LTLT)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER {
        if (match(T_GTGT)) {
            gen_push(k);
            k = hier8 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            if (nosign(lval)) {
                gen_logical_shift_right();
            } else {
                gen_arithm_shift_right();
            }
        } else if (match(T_LTLT)) {
            gen_push(k);
            k = hier8 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            gen_arithm_shift_left();
        } else
            return (0);
    }

}

/**
 * addition, subtraction
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier8 (LVALUE *lval) {
    int     k;
    LVALUE lval2[1];

    k = hier9 (lval);
    if (token != T_PLUS && token != T_MINUS)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER {
        if (match (T_PLUS)) {
            gen_push(k);
            k = hier9 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            // if left is pointer and right is int, scale right
            if (dbltest(lval,lval2)) {
                gen_multiply(lval->ptr_type, lval->tagsym ? lval->tagsym->size : INTSIZE);
            }
            // will scale left if right int pointer and left int
            gen_add (lval,lval2);
            result (lval, lval2);
        } else if (match (T_MINUS)) {
            gen_push(k);
            k = hier9 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            /* if dbl, can only be: pointer - int, or
                                pointer - pointer, thus,
                in first case, int is scaled up,
                in second, result is scaled down. */
            if (dbltest(lval,lval2)) {
                gen_multiply(lval->ptr_type, lval->tagsym ? lval->tagsym->size : INTSIZE);
            }
            gen_sub ();
            /* if both pointers, scale result */
            if ((lval->ptr_type & CINT) && (lval2->ptr_type & CINT)) {
                gen_divide_by_two(); /* divide by intsize */
            }
            result (lval, lval2);
        } else
            return (0);
    }
}

/**
 * multiplication, division, modulus
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier9 (LVALUE *lval) {
    int     k;
    LVALUE lval2[1];

    k = hier10 (lval);
    if (token != T_STAR && token != T_PERCENT && token != T_SLASH)
        return (k);
    if (k & FETCH)
        k = rvalue(lval, k);
    FOREVER {
        if (match (T_STAR)) {
            gen_push(k);
            k = hier10 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            gen_mult ();
        } else if (match (T_SLASH)) {
            gen_push(k);
            k = hier10 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            if(nosign(lval) || nosign(lval2)) {
                gen_udiv();
            } else {
                gen_div ();
            }
        } else if (match (T_PERCENT)) {
            gen_push(k);
            k = hier10 (lval2);
            if (k & FETCH)
                k = rvalue(lval2, k);
            if(nosign(lval) || nosign(lval2)) {
                gen_umod();
            } else {
                gen_mod ();
            }
        } else
            return (0);
    }

}

/**
 * increment, decrement, negation operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier10 (LVALUE *lval) {
    int     k;
    SYMBOL *ptr;

    if (match (T_PLUSPLUS)) {
        if (((k = hier10 (lval)) & FETCH) == 0) {
            needlval ();
            return (0);
        }
        if (lval->indirect)
            gen_push(k);
        k = rvalue(lval, k);
        gen_increment_primary_reg (lval);
        store (lval);
        return (HL_REG);
    } else if (match (T_MINUSMINUS)) {
        if (((k = hier10 (lval)) & FETCH) == 0) {
            needlval ();
            return (0);
        }
        if (lval->indirect)
            gen_push(k);
        k = rvalue(lval, k);
        gen_decrement_primary_reg (lval);
        store (lval);
        return (HL_REG);
    } else if (match (T_MINUS)) {
        k = hier10 (lval);
        if (k & FETCH)
            k = rvalue(lval, k);
        gen_twos_complement();
        return (HL_REG);
    } else if (match (T_TILDE)) {
        k = hier10 (lval);
        if (k & FETCH)
            k = rvalue(lval, k);
        gen_complement ();
        return (HL_REG);
    } else if (match (T_BANG)) {
        k = hier10 (lval);
        if (k & FETCH)
            k = rvalue(lval, k);
        gen_logical_negation();
        return (HL_REG);
    } else if (match(T_STAR)) {
        k = hier10 (lval);
        if (k & FETCH)
            k = rvalue(lval, k);
        if ((ptr = lval->symbol) != 0)
            lval->indirect = ptr->type;
        else
            lval->indirect = CINT;
        lval->ptr_type = 0;  // flag as not pointer or array
        return FETCH | k;
    } else if (match(T_AND)) {
        k = hier10 (lval);
        if ((k & FETCH) == 0) {
            error ("illegal address");
            return (0);
        }
        ptr = lval->symbol;
        lval->ptr_type = ptr->type;
        if (lval->indirect) {
            if (k & DE_REG) {
                gen_swap();
            }
            return (HL_REG);
        }
        // global and non-array
        gen_immediate ();
        output_name ((ptr = lval->symbol)->name);
        newline ();
        lval->indirect = ptr->type;
        return (HL_REG);
    } else {
        k = hier11 (lval);
        if (match (T_PLUSPLUS)) {
            if ((k & FETCH) == 0) {
                needlval ();
                return (0);
            }
            if (lval->indirect)
                gen_push(k);
            k = rvalue(lval, k);
            gen_increment_primary_reg (lval);
            store (lval);
            gen_decrement_primary_reg (lval);
            return (HL_REG);
        } else if (match (T_MINUSMINUS)) {
            if ((k & FETCH) == 0) {
                needlval ();
                return (0);
            }
            if (lval->indirect)
                gen_push(k);
            k = rvalue(lval, k);
            gen_decrement_primary_reg (lval);
            store (lval);
            gen_increment_primary_reg (lval);
            return (HL_REG);
        } else
            return (k);
    }

}

/**
 * array subscripting
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int hier11(LVALUE *lval) {
    int     direct, k;
    SYMBOL *ptr;
    unsigned sname;

    k = primary(lval);
    ptr = lval->symbol;
    if (token == T_LSQUARE || token == T_LPAREN || token == T_DOT || token == T_POINTSTO)
        FOREVER {
            if (match(T_LSQUARE)) {
                if (ptr == 0) {
                    error("can't subscript");
                    junk();
                    needbrack(T_RSQUARE);
                    return (0);
                } else if (ptr->identity == POINTER) {
                    k = rvalue(lval, k);
                } else if (ptr->identity != ARRAY) {
                    error("can't subscript");
                    k = 0;
                }
                gen_push(k);
                expression (YES);
                needbrack (T_LSQUARE);
                gen_multiply(ptr->type, tag_table[ptr->tagidx].size);
                gen_add (NULL,NULL);
                //lval->symbol = 0;
                lval->indirect = ptr->type;
                lval->ptr_type = 0;
                k = FETCH | HL_REG;
            } else if (match (T_LPAREN)) {
                if (ptr == 0) {
                    callfunction(0);
                } else if (ptr->identity != FUNCTION) {
                    k = rvalue(lval, k);
                    callfunction(0);
                } else {
                    callfunction(ptr->name);
                }
                lval->symbol = 0;
                k = 0;
            } else if ((direct=match(T_DOT)) || match(T_POINTSTO)) {
                if (lval->tagsym == 0) {
                    error("can't take member");
                    junk();
                    return 0;
                }
                if ((sname = symname()) == 0 ||
                   ((ptr=find_member(lval->tagsym, sname)) == 0)) {
                    error("unknown member");
                    junk();
                    return 0;
                }
                if ((k & FETCH) && direct == 0) {
                    k = rvalue(lval, k);
                }
                if (k == DE_REG) {
                    gen_swap();
                }
                add_offset(ptr->offset); // move pointer from struct begin to struct member
                lval->symbol = ptr;
                lval->indirect = ptr->type; // lval->indirect = lval->val_type = ptr->type
                lval->ptr_type = 0;
                lval->tagsym = NULL_TAG;
                if (ptr->type == STRUCT) {
                    lval->tagsym = &tag_table[ptr->tagidx];
                }
                if (ptr->identity == POINTER) {
                    lval->indirect = CINT;
                    lval->ptr_type = ptr->type;
                    //lval->val_type = CINT;
                }
                if (ptr->identity==ARRAY ||
                    (ptr->type==STRUCT && ptr->identity==VARIABLE)) {
                    // array or struct
                    lval->ptr_type = ptr->type;
                    //lval->val_type = CINT;
                    k = 0;
                } else {
                    k = FETCH | HL_REG;
                }
            }
            else return k;
        }
    if (ptr == 0)
        return k;
    if (ptr->identity == FUNCTION) {
        gen_immediate();
        output_name(ptr->name);
        newline();
        return 0;
    }
    return k;
}

