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
 * @return 	 - root node of tree
 */
struct node *expression(int comma) {
    LVALUE lval;
    struct node *n;

    /* Build a tree of comma operations */
    n = make_rval(hier1(&lval));
    if (!comma || !match(T_COMMA))
        return n;
    n = tree(T_COMMA, n, expression(comma));
    return n;
}

/**
 * assignment operators
 * @param lval
 * @return 
 */
struct node *hier1 (LVALUE *lval) {
    LVALUE lval2[1];
    struct node *l, *r;
    unsigned fc;
    unsigned scale = 1;

    l = hier1a (lval);
    if (match (T_EQ)) {
        if ((l->flags & LVAL) == 0) {
            needlval ();
            return (0);
        }
        r = make_rval(hier1 (lval2));
        return tree(T_EQ, l, r);	/* Assignment */
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
            if ((l->flags & LVAL) == 0) {
                needlval ();
                return (0);
            }
            r = make_rval(hier1 (lval2));
            switch (fc) {
                case T_MINUSEQ: {
                    if (dbltest(lval,lval2))
                        scale = lval->tagsym ? lval->tagsym->size : INTSIZE;
                    break;
                }
                case T_PLUSEQ: {
                    if (dbltest(lval,lval2))
                        scale = lval->tagsym ? lval->tagsym->size : INTSIZE;
                    break;
                }
            }
            if (scale)
                return tree(fc, l, tree(T_STAR, r, make_constant(scale)));
            return tree(fc, l, r);
        } else
            return l;
    }
    /* gcc */
    return NULL;
}

/**
 * processes ? : expression
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier1a (LVALUE *lval) {
    struct node *l;
    struct node *a1, *a2;
    LVALUE lval2[1], lval3[1];

    l = hier1b (lval);
    if (!match(T_QUESTION))
        return (l);

    l = make_rval(l);
    /* Now do the left of the colon */
    a1 = make_rval(hier1a(lval2));
    if (!match(T_COLON)) {
        error("missing colon");
        return l;
    }
    a2 = make_rval(hier1b(lval3));
    return tree(T_QUESTION, l, tree(T_COLON, a1, a2));
}

/**
 * processes logical or ||
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier1b (LVALUE *lval) {
    struct node *l;
    LVALUE lval2[1];

    l = hier1c (lval);

    if (!match(T_OROR))
        return (l);

    return tree(T_OROR, make_rval(l), make_rval(hier1b(lval2)));
}

/**
 * processes logical and &&
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier1c (LVALUE *lval) {
    struct node *l;
    LVALUE lval2[1];

    l = hier2 (lval);

    if (!match(T_ANDAND))
        return (l);

    return tree(T_ANDAND, make_rval(l), make_rval(hier1c(lval2)));
}

/**
 * processes bitwise or |
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier2 (LVALUE *lval) {
    struct node *l;
    LVALUE lval2[1];

   l = hier3 (lval);
   if (!match(T_OR))
        return (l);

    return tree(T_OR, make_rval(l), make_rval(hier2(lval2)));
}

/**
 * processes bitwise exclusive or
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier3 (LVALUE *lval) {
    struct node *l;
    LVALUE lval2[1];

    l = hier4 (lval);
    if (!match(T_HAT))
        return (l);

    return tree(T_HAT, make_rval(l), make_rval(hier3(lval2)));
}

/**
 * processes bitwise and &
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier4 (LVALUE *lval) {
    struct node *l;
    LVALUE lval2[1];

    l = hier5 (lval);
    if (!match(T_AND))
        return (l);

    return tree(T_AND, make_rval(l), make_rval(hier4(lval2)));
}

/**
 * processes equal and not equal operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier5 (LVALUE *lval) {
    struct node *l;
    unsigned op;
    LVALUE lval2[1];

    l = hier6 (lval);
    if (token != T_EQEQ && token  != T_BANGEQ)
        return (l);
    op = token;
    next_token();
    return tree(op, make_rval(l), make_rval(hier5(lval2)));
}

/**
 * comparison operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier6 (LVALUE *lval) {
    struct node *l;
    unsigned op;
    LVALUE lval2[1];

    l = hier7 (lval);

    if (token != T_LT && token != T_GT && token != T_LTEQ && token != T_GTEQ)
        return (l);
    op = token;
    next_token();
    /* This assumes we deal with types somewhere */
    return tree(op, make_rval(l), make_rval(hier6(lval2)));
}

/**
 * bitwise left, right shift
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier7 (LVALUE *lval) {
    struct node *l;
    unsigned op;
    LVALUE lval2[1];

    l = hier8(lval);
    if (token != T_GTGT && token != T_LTLT)
        return l;

    op = token;
    next_token();
    return tree(op, make_rval(l), make_rval(hier7(lval2)));
}

/**
 * addition, subtraction
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier8 (LVALUE *lval) {
    struct node *l, *r;
    unsigned op;
    LVALUE lval2[1];
    unsigned scale = 1;
    unsigned scaledir = 1;

    l = hier9 (lval);
    if (token != T_PLUS && token != T_MINUS)
        return l;

    op = token;
    next_token();

    l = make_rval(l);
    r = make_rval(hier8(lval2));

    if (op == T_PLUS) {
        // if left is pointer and right is int, scale right)
        if (dbltest(lval,lval2))
            scale = lval->tagsym ? lval->tagsym->size : INTSIZE;
        // will scale left if right int pointer and left int
        /* FIXME: swap sides according to scaling need. For now just
           hack right */
    } else if (op == T_MINUS) {
        /* if dbl, can only be: pointer - int, or
                                pointer - pointer, thus,
            in first case, int is scaled up,
            in second, result is scaled down. */
        if (dbltest(lval,lval2))
            scale = lval->tagsym ? lval->tagsym->size : INTSIZE;

        /* if both pointers, scale result */
        if ((lval->ptr_type & CINT) && (lval2->ptr_type & CINT))
            scaledir = 2;
    }
    if (scale == 1)
        return tree(op, l, r);
    if (scaledir == 2)
        return tree(T_SLASH, tree(op, l, r), make_constant(scale));
    /* Work out which side to scale */
    if (lval->ptr_type & CINT)
        return tree(op, l, tree(T_STAR, r, make_constant(scale)));
    else
        return tree(op, tree(T_STAR, l, make_constant(scale)), r);
}

/**
 * multiplication, division, modulus
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier9 (LVALUE *lval) {
    struct node *l;
    unsigned op;
    LVALUE lval2[1];

    l = hier10 (lval);
    if (token != T_STAR && token != T_PERCENT && token != T_SLASH)
        return l;
    op = token;
    next_token();

    return tree(op, l, hier9(lval2));
}

/**
 * increment, decrement, negation operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */

struct node *hier10 (LVALUE *lval) {
    struct node *l, *r;
    unsigned op;
    SYMBOL *ptr;

    op = token;
    if (token != T_PLUSPLUS && token != T_MINUSMINUS && token != T_MINUS &&
        token != T_TILDE && token != T_BANG && token != T_STAR && token != T_AND) {
        /* Check for trailing forms */
        l = hier11(lval);
        if (token == T_PLUSPLUS || token == T_MINUSMINUS) {
            next_token();
            if (!(l->flags & LVAL)) {
                needlval();
                return l;
            }
            if (token == T_PLUSPLUS)
                return tree(T_POSTINC, NULL, l);
            else
                return tree(T_POSTDEC, NULL, l);
        }
        return l;
    }

    next_token();

    switch(op) {
    case T_PLUSPLUS:
    case T_MINUSMINUS:
        r = hier10(lval);
        if (!r->flags & LVAL) {
            needlval ();
            return (0);
        }
        /* TODO:  Scaling... */
        return tree(op, NULL, r);
    case T_TILDE:
    case T_BANG:
        return tree(op, NULL, make_rval(hier10(lval)));
    /* Disambiguate forms */
    case T_MINUS:
        return tree(T_NEGATE, NULL, make_rval(hier10(lval)));
    case T_STAR: 
        if ((ptr = lval->symbol) != 0)
            lval->indirect = ptr->type;
        else
            lval->indirect = CINT;
        lval->ptr_type = 0;  // flag as not pointer or array
        return tree(T_DEREF, NULL, make_rval(hier10(lval)));
    case T_AND:
        r = hier10(lval);
        if (r->flags & LVAL) {
            error("illegal address of");
            return r;
        }
        ptr = lval->symbol;
        lval->ptr_type = ptr->type;
        lval->indirect = ptr->type;
        return tree(T_ADDROF, NULL, r);
    }
    /* gcc */
    return NULL;
}

/**
 * array subscripting
 * @param lval
 *
 * Build a tree for anything nailed after the primary. So for example
 * foo[4][5] ends up as             add
 *                              foo       add
 *                                    4*n       5
 */
struct node *hier11(LVALUE *lval) {
    int     direct;
    struct node *l, *r;
    SYMBOL *ptr;
    unsigned sname;
    unsigned scale;

    l = primary(lval);

    ptr = lval->symbol;
    if (token == T_LSQUARE || token == T_LPAREN || token == T_DOT || token == T_POINTSTO) {
        FOREVER {
            if (match(T_LSQUARE)) {
                if (ptr == 0) {
                    error("can't subscript");
                    junk();
                    needbrack(T_RSQUARE);
                    return (0);
                } else if (ptr->identity == POINTER) {
                    l = make_rval(l);
                } else if (ptr->identity != ARRAY) {
                    error("can't subscript");
                }
                r = expression (YES);
                needbrack (T_RSQUARE);
                /* Need a proper method for this stuff */
                if (ptr->type == STRUCT)
                    scale = tag_table[ptr->tagidx].size;
                else if(ptr->type == CINT || ptr->type == UINT)
                    scale = 2;
                else
                    scale = 1;
                l = tree(T_PLUS, l, tree(T_STAR, r, make_constant(scale)));
                l->flags |= LVAL;
                lval->indirect = ptr->type;
                lval->ptr_type = 0;
            } else if (match (T_LPAREN)) {
                if (ptr == NULL) {
                    l = callfunction(NULL, l);
                } else if (ptr->identity != FUNCTION) {
                    l = callfunction(ptr, NULL);
                } else {
                    l = callfunction(ptr, NULL);
                }
                lval->symbol = 0;
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
                if (direct == 0)
                    l = tree(T_DEREF, NULL, l);

                l = tree(T_PLUS, l, make_constant(ptr->offset));
                l->flags |= LVAL;
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
                }
            }
            else return l;
        }
    }
    return l;
}

