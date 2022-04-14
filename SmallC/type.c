/*
 *	Type support
 */

#include <stdio.h>

#include "defs.h"
#include "type.h"

void badtype(void)
{
    error("bad type");
}

#define PTRSIZE 	2
static unsigned sizetab[16] = {
    /* FIXME - target should supply */
    1, 2, 4, 8,
    1, 2, 4, 8,
    4, 8, 1, 0,	/* A void has no size but a void * is deemed to be 1 */
    0, 0, 0, 0
};

static unsigned primitive_sizeof(unsigned t)
{
    unsigned s = sizetab[(t >> 4) & 0x0F];
    if (s == 0) {
        error("cannot size type");
        s = 1;
    }
    return s;
}

unsigned type_deref(unsigned t)
{
    switch(CLASS(t)) {
    case C_SIMPLE:
        if (PTR(t))
            return --t;
        break;
    default:;
    }
    error("cannot dereference");
    return CINT;
    /* FIXME: arrays */
}

unsigned type_ptr(unsigned t)
{
    if (PTR(t) == 7)
        error("too many indirections");
    else
        t++;
    return t;
}

unsigned type_sizeof(unsigned t)
{
    if (PTR(t))
        return PTRSIZE;
    if (IS_SIMPLE(t))
        return primitive_sizeof(t);
    /* TODO array struct */
    return 1;
}

unsigned type_ptrscale(unsigned t)
{
    /* FIXME arrays ? */
    if (!PTR(t)) {
        error("not a pointer");
        return 1;
    }
    return sizeof(type_deref(t));
}

/* lvalue conversion is handled by caller */
unsigned type_addrof(unsigned t)
{
    if (PTR(t))
        return t-1;
    error("cannot take address");
    return VOID + 1;
}

unsigned type_ptrscale_binop(unsigned op, unsigned l, unsigned r, unsigned *div)
{
    *div = 1;

    /* FIXME: need an array to pointer helper of some form */
    if (PTR(l) && PTR(r)) {
        if (l != r)
            error("pointer type mismatch");
        if (op == T_MINUS)
            return type_ptrscale(r);
        else {
            error("invalid pointer difference");
            return 1;
        }
    }
    *div = 0;
    if (PTR(l) && IS_ARITH(r))
        return type_ptrscale(l);
    if (PTR(r) && IS_ARITH(l))
        return type_ptrscale(r);
    if (IS_ARITH(l) && IS_ARITH(r))
        return 1;
    error("invalid types");
    return 1;
}

unsigned node_constant(struct node *n)
{
    if (n->op >= T_INTVAL && n->op <= T_ULONGVAL)
        return 1;
    return 0;
}

unsigned node_constant_zero(struct node *n)
{
    if (node_constant(n))
        return !n->value;
    return 0;
}

static int type_pointermatch(struct node *l, struct node *r)
{
    unsigned lt = l->type;
    unsigned rt = r->type;
    /* The C zero case */
    if (node_constant_zero(l) && PTR(rt))
        return 1;
    if (node_constant_zero(r) && PTR(lt))
        return 1;
    /* Not pointers */
    if (!PTR(lt) || !PTR(rt))
        return 0;
    /* Same depth and type */
    if (lt == rt)
        return 1;
    /* void * is fine */
    if (BASE_TYPE(lt) == VOID)
        return 1;
    if (BASE_TYPE(rt) == VOID)
        return 1;
    /* sign errors */
    if (BASE_TYPE(lt) < FLOAT && (lt ^ rt) == UNSIGNED) {
        warning("sign mismatch");
        return 1;
    }
    return 0;
}
        
    
/* Assumes any implicit conversions are done first */
unsigned type_uniop(unsigned op, struct node *n)
{
    unsigned nt = n->type;
    switch (op) {
        case T_TILDE:
            if (IS_ARITH(nt))
                return nt;
            break;
        case T_BANG:
            if (PTR(nt) || IS_ARITH(nt))
                return CINT;
            break;
        case T_STAR:
            if (PTR(nt))
                return nt - 1;
            break;
        case T_AND:
            /* FIXME .. addrof able is complicated */
            return type_addrof(nt);
    }
    badtype();
    return CINT;
}

/* Returns the result type (and thus the final co-oercion type) */
unsigned type_binop(unsigned op, struct node *l, struct node *r)
{
    int ptr = 0;
    unsigned lt = l->type;
    unsigned rt = r->type;

    if (PTR(lt) && PTR(rt))
        ptr = 2;
    else if (PTR(lt) || PTR(rt))
        ptr = 1;
    switch(op) {
    case T_LTLT:
    case T_GTGT:
    case T_AND:
    case T_OR:
    case T_HAT:
    case T_PERCENT:
        if (IS_INTARITH(lt) && IS_INTARITH(rt))
            return lt;
        break;
    case T_MINUS:
        if (ptr == 2 && type_pointermatch(l, r))
            return lt;
    case T_PLUS:
        if (ptr == 1) {
            if (PTR(lt) && IS_INTARITH(rt))
                return lt;
            if (PTR(rt) && IS_INTARITH(lt))
                return rt;
            break;
        }
    case T_STAR:
    case T_SLASH:
        if (IS_ARITH(lt) && IS_ARITH(rt))
            return lt;
        break;

    case T_LT:
    case T_GT:
    case T_LTEQ:
    case T_GTEQ:
    case T_EQEQ:
    case T_BANGEQ:
        if (IS_ARITH(lt) && IS_ARITH(rt))
            return lt;
        /* handles const 0 and void casting internally */
        return type_pointermatch(l, r);

    case T_ANDAND:
    case T_OROR:
        /* we can ptr &&  because it's implicity a zero compare */
        if ((IS_ARITH(lt) || PTR(lt)) && (IS_ARITH(rt) || PTR(rt)))
            break;

    /* Doesn't matter what is each side */
    case T_COMMA:
        return ANY;

    /* arguably same same.. needs work but is
        (foo ? struct a : struct b).x valid
        for us at this point */
    case T_COLON:
        if (type_pointermatch(l, r))
            return lt;
        if (IS_ARITH(lt) && IS_ARITH(rt))
            return lt;
        break;

    case T_QUESTION:
        if (IS_ARITH(lt) || PTR(lt))
            return rt;
        break;
    case T_MINUSEQ:
        if (PTR(lt) && PTR(rt) && type_pointermatch(l, r))
            return CINT;
    case T_PLUSEQ:
        if (PTR(lt) && IS_INTARITH(rt))
            return lt;
    case T_STAREQ:
    case T_PERCENTEQ:
    case T_SHREQ:
    case T_SHLEQ:
    case T_ANDEQ:
    case T_OREQ:
        if (IS_INTARITH(lt) && IS_INTARITH(rt))
            return lt;
        break;
    case T_EQ:
        if (type_pointermatch(l, r))
            return lt;
        if (lt == rt)
            return lt;
        /* Someone has to own assign co-ercion ?? */
        break;
    }
    badtype();
    return CINT;
}

static unsigned typeconflict(void)
{
    error("type conflict");
    return CINT;
}

static unsigned is_modifier(void)
{
    return (token == T_CONST || token == T_VOLATILE);
}

static unsigned is_storage_word(void)
{
    return (token >= T_AUTO && token <= T_STATIC);
}

static unsigned is_type_word(void)
{
    return (token >= T_CHAR && token <= T_VOID);
}

static void skip_modifiers(void)
{
    while(is_modifier())
        next_token();
}

unsigned get_storage(unsigned dflt)
{
    skip_modifiers();
    if (!is_storage_word())
        return dflt;
    switch(token) {
    case T_AUTO:
        return AUTO;
    case T_REGISTER:
        return AUTO;	/* For now */
    case T_STATIC:
        if (dflt == T_AUTO)
            return LSTATIC;	/* Do we care about having LSTATIC FIXME */
        else
            return STATIC;
    case T_EXTERN:
        return EXTERN;
    }
    /* gcc */
    return 0;
}

unsigned structured_type(unsigned sflag)
{
    unsigned name = symname();
    unsigned type;
    unsigned ptr = 0;
    int otag;	/* Until sorted */
    if (name == 0) {
        illname();
        return CINT;
    }
    /* Need to replace this with our new symbol stuff */
    if ((otag = find_tag(name)) == -1) {
        error("unknown struct/uinion");
        return CINT;
    }
    while(match(T_STAR)) {
        skip_modifiers();	/* volatile const etc */
        ptr++;
    }

    if (ptr > 7)
        error("too many indirections");
    /* Ok assemble the type */
#if 0
    type = CLASS_STRUCT | (sym_number(sym) << 3);
    type += ptr;
#endif
    return type;
}

static unsigned once_flags;

static void set_once(unsigned bits)
{
    if (once_flags & bits)
        typeconflict();
    once_flags |= bits;
}

/*
 *	Parse all the type blurb up to the symbol name. We don't deal with
 *	typedefs yet and that complicates things as you can have typedefs
 *	that have pointer/arrayness. Also TODO is enum...
 *
 *	For our simpler case it's a lot easier. We are going to hand back
 *	either a struct or union type with some degree of pointer indirection
 *	possible
 *
 *	Stuff following the name is not our problem. So int x[4] is an int
 *	to us. The fact it's any array is the callers fun for now but wants
 *	centralizing somewhere else (especially once we hit typedef)
 */
unsigned get_type(void)
{
    unsigned sflag = 0;
    unsigned type = CINT;	/* default */
    unsigned ptr = 0;


    /* Not a type */
    if (!is_modifier() && token != T_STAR && !is_type_word())
        return UNKNOWN;

    skip_modifiers();	/* volatile const etc */

    while(match(T_STAR)) {
        skip_modifiers();	/* volatile const etc */
        ptr++;
    }

    if (ptr > 7)
        error("too many indirections");

    once_flags = 0;

    skip_modifiers();	/* volatile const etc */

    if ((sflag = match(T_STRUCT)) || match(T_UNION))
        return structured_type(sflag);
    while(is_type_word()) {
        switch(token) {
        case T_SHORT:
            set_once(1);
            break;
        case T_LONG:
            /* For now -- long long is for the future */
            set_once(2);
            break;
        case T_UNSIGNED:
            set_once(4);
            break;
        case T_SIGNED:
            set_once(8);
            break;
        case T_CHAR:
            type = CCHAR;
            break;
        case T_INT:
            type = CINT;
            break;
        case T_FLOAT:
            type = FLOAT;
            break;
        case T_DOUBLE:
            type = DOUBLE;
            break;
        }
        next_token();
    }
    /* Now put it all together */

    /* No signed unsigned or short longs */
    if ((once_flags & 3) == 3 || (once_flags & 12) == 12)
        return typeconflict();
    /* No long or short char */
    if (type == CCHAR && (once_flags & 3))
        return typeconflict();
    /* No signed/unsigned float or double */
    if ((type == FLOAT || type == DOUBLE) && (once_flags & 12))
        return typeconflict();
    /* No void modifiers */
    if (type == VOID && once_flags)
        return typeconflict();
    /* long */
    if (type == CINT && (once_flags & 2))
        type = CLONG;
    if (type == FLOAT && (once_flags & 2))
        type = DOUBLE;
    /* short */
    /* FIXME: allow for int being long/short later */
    if (type == CINT && (once_flags & 1))
        type = CINT;
    /* signed/unsigned */
    if (once_flags & 4)
        type |= UNSIGNED;
    /* We don't deal with default unsigned char yet .. */
    if (once_flags & 8)
        type &= ~UNSIGNED;

    while(match(T_STAR)) {
        skip_modifiers();	/* volatile const etc */
        ptr++;
    }

    if (ptr > 7)
        error("too many indirections");
    return type + ptr;
}

/*
 *	Above this lot sits a parser for typedef and declarations
 *	that handles the name (sometimes optional) and trailing array etc
 *
 *	We want to split that out from the current primary handling so we
 *	can use it for typedef.
 */

