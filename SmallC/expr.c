/*
 * File expr.c: 2.2 (83/06/21,11:24:26)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

/**
 * lval.symbol - symbol table address, else 0 for constant
 * lval.indirect - type indirect object to fetch, else 0 for static object
 * lval.ptr_type - type pointer or array, else 0
 * @param comma
 * @return 	 - root node of tree
 */
struct node *expression(int comma) {
    struct node *n;

    /* Build a tree of comma operations */
    n = make_rval(hier1());
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
struct node *hier1 (void) {
    struct node *l, *r;
    unsigned fc;
    unsigned scale = 1;

    l = hier1a ();
    if (match (T_EQ)) {
        if ((l->flags & LVAL) == 0) {
            needlval ();
            return (0);
        }
        r = make_rval(hier1());
        return tree(T_EQ, l, r);	/* Assignment */
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
            r = make_rval(hier1 ());
            switch (fc) {
                case T_MINUSEQ:
                case T_PLUSEQ:
                    scale = type_ptrscale(l->type);
                    break;
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
struct node *hier1a (void) {
    struct node *l;
    struct node *a1, *a2;

    l = hier1b ();
    if (!match(T_QUESTION))
        return (l);

    l = make_rval(l);
    /* Now do the left of the colon */
    a1 = make_rval(hier1a());
    if (!match(T_COLON)) {
        error("missing colon");
        return l;
    }
    a2 = make_rval(hier1b());
    return tree(T_QUESTION, l, tree(T_COLON, a1, a2));
}

/**
 * processes logical or ||
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier1b(void) {
    struct node *l;

    l = hier1c ();

    if (!match(T_OROR))
        return (l);

    return tree(T_OROR, make_rval(l), make_rval(hier1b()));
}

/**
 * processes logical and &&
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier1c (void) {
    struct node *l;

    l = hier2 ();

    if (!match(T_ANDAND))
        return (l);

    return tree(T_ANDAND, make_rval(l), make_rval(hier1c()));
}

/**
 * processes bitwise or |
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier2 (void) {
    struct node *l;

   l = hier3 ();
   if (!match(T_OR))
        return (l);

    return tree(T_OR, make_rval(l), make_rval(hier2()));
}

/**
 * processes bitwise exclusive or
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier3 (void) {
    struct node *l;

    l = hier4 ();
    if (!match(T_HAT))
        return (l);

    return tree(T_HAT, make_rval(l), make_rval(hier3()));
}

/**
 * processes bitwise and &
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier4 (void) {
    struct node *l;

    l = hier5 ();
    if (!match(T_AND))
        return (l);

    return tree(T_AND, make_rval(l), make_rval(hier4()));
}

/**
 * processes equal and not equal operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier5 (void) {
    struct node *l;
    unsigned op;

    l = hier6 ();
    if (token != T_EQEQ && token  != T_BANGEQ)
        return (l);
    op = token;
    next_token();
    return tree(op, make_rval(l), make_rval(hier5()));
}

/**
 * comparison operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier6 (void) {
    struct node *l;
    unsigned op;

    l = hier7 ();

    if (token != T_LT && token != T_GT && token != T_LTEQ && token != T_GTEQ)
        return (l);
    op = token;
    next_token();
    /* This assumes we deal with types somewhere */
    return tree(op, make_rval(l), make_rval(hier6()));
}

/**
 * bitwise left, right shift
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier7 (void) {
    struct node *l;
    unsigned op;

    l = hier8();
    if (token != T_GTGT && token != T_LTLT)
        return l;

    op = token;
    next_token();
    return tree(op, make_rval(l), make_rval(hier7()));
}

/**
 * addition, subtraction
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier8 (void) {
    struct node *l, *r;
    unsigned op;
    unsigned scale = 1;
    unsigned scalediv;

    l = hier9 ();
    if (token != T_PLUS && token != T_MINUS)
        return l;

    op = token;
    next_token();

    l = make_rval(l);
    r = make_rval(hier8());

    if (op == T_PLUS) {
        // if left is pointer and right is int, scale right)
        scale = type_ptrscale_binop(op, l->type, r->type, &scalediv);
            /* val->tagsym ? lval->tagsym->size : INTSIZE; */
        // will scale left if right int pointer and left int
        /* FIXME: swap sides according to scaling need. For now just
           hack right */
    } else if (op == T_MINUS) {
        /* if dbl, can only be: pointer - int, or
                                pointer - pointer, thus,
            in first case, int is scaled up,
            in second, result is scaled down. */
        scale = type_ptrscale_binop(op, l->type, r->type, &scalediv);
    }
    if (scale == 1)
        return tree(op, l, r);
    if (scalediv)
        return tree(T_SLASH, tree(op, l, r), make_constant(scale));
    if (PTR(l->type))
        return tree(op, l, tree(T_STAR, r, make_constant(scale)));
    else
        return tree(op, tree(T_STAR, l, make_constant(scale)), r);
}

/**
 * multiplication, division, modulus
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
struct node *hier9 (void) {
    struct node *l;
    unsigned op;

    l = hier10 ();
    if (token != T_STAR && token != T_PERCENT && token != T_SLASH)
        return l;
    op = token;
    next_token();

    return tree(op, l, hier9());
}

/**
 * increment, decrement, negation operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */

struct node *hier10 (void) {
    struct node *l, *r;
    unsigned op;

    op = token;
    if (token != T_PLUSPLUS && token != T_MINUSMINUS && token != T_MINUS &&
        token != T_TILDE && token != T_BANG && token != T_STAR && token != T_AND) {
        /* Check for trailing forms */
        l = hier11();
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
        r = hier10();
        if (!r->flags & LVAL) {
            needlval ();
            return (0);
        }
        /* TODO:  Scaling... */
        return tree(op, NULL, r);
    case T_TILDE:
    case T_BANG:
        return tree(op, NULL, make_rval(hier10()));
    /* Disambiguate forms */
    case T_MINUS:
        return tree(T_NEGATE, NULL, make_rval(hier10()));
    case T_STAR:
        /* TODO *array */
        r = tree(T_DEREF, NULL, make_rval(hier10()));
        r->type = r->right->type - 1;
        return r;
    case T_AND:
        r = hier10();
        if (r->flags & LVAL) {
            error("illegal address of");
            return r;
        }
        r = tree(T_ADDROF, NULL, r);
        r->type = type_addrof(r->type);
        return r;
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
struct node *hier11(void) {
    int     direct;
    struct node *l, *r;
    unsigned ptr;
    struct symbol *sym;
    unsigned sname;
    unsigned scale;

    l = primary();

    ptr = PTR(l->type) || IS_ARRAY(l->type);
    if (token == T_LSQUARE || token == T_LPAREN || token == T_DOT || token == T_POINTSTO) {
        FOREVER {
            if (match(T_LSQUARE)) {
                if (ptr == 0) {
                    error("can't subscript");
                    junk();
                    needbrack(T_RSQUARE);
                    return (0);
                }
                r = expression (YES);
                needbrack (T_RSQUARE);
                /* Need a proper method for this stuff */
                /* TODO arrays */
                scale = type_ptrscale(l->type);
#if 0
                if (IS_STRUCT(l->type)
                    scale = tag_table[INFO(l->type)].size;
                else if(ptr->type == CINT || ptr->type == UINT)
                    scale = 2;
                else
                    scale = 1;
#endif
                l = tree(T_PLUS, l, tree(T_STAR, r, make_constant(scale)));
                l->flags |= LVAL;
                l->type = type_deref(l->type);
            } else if (match (T_LPAREN)) {
                l = callfunction(l);
            } else if ((direct=match(T_DOT)) || match(T_POINTSTO)) {
                if (direct == 0)
                    l = tree(T_DEREF, NULL, l);

                if (PTR(l->type) || !IS_STRUCT(l->type)) {
                    error("can't take member");
                    junk();
                    return 0;
                }
                /* FIXME: we should pull this out of the tag table and
                   struct ? */
                if ((sname = symname()) == 0 ||
                   ((sym=find_member(INFO(l->type), sname)) == 0)) {
                    error("unknown member");
                    junk();
                    return 0;
                }
                l = tree(T_PLUS, l, make_constant(sym->offset));
                l->flags |= LVAL;
                l->sym = sym;
                l->type = sym->type;
            }
            else return l;
        }
    }
    return l;
}

