/*
 *	Initializers .. really needs the tree consting stuff doing to work
 *	properly.
 */

#include "compiler.h"

/* This is a bit trickier but we need our tree constifier to do it right */

/* A single initialization value */
static void initializer_single(struct symbol *sym, unsigned type, unsigned storage)
{
    int v;
    /* Will be a coerce(constify(expression)()) - need to handle pointers
       and "string" also handle names being counted as constant in this
       situation */
    struct node *n = constant_node();
    /* Need to make constant_node do its own casting TODO */
    if (storage == AUTO) {
        write_tree(tree(T_EQ, make_symbol(sym), n));
    } else {
        /* TODO */
//    put_typed_data(type, v, storage);
       ;
    }
}

/* C99 permits trailing comma and ellipsis */
/* Strictly {} is not permitted - there must be at least one value */

/* Array initializer. Repeated runs of the same type */
static void initializer_group(struct symbol *sym, unsigned type, unsigned n, unsigned storage)
{
    /* Up to n comma delimited initializers of a type */
    /* For now via number */
    require(T_LCURLY);
    while(n && token != T_RCURLY) {
        if (token == T_ELLIPSIS)
            break;
        n--;
        next_token();
        initializer_single(sym, type, storage);
        if (!match(T_COMMA))
            break;
    }
    if (n) {
        unsigned s = type_sizeof(type) * n;
//        put_padding(s, storage);
    }
    require(T_RCURLY);
}

/* Struct and union initializer */
static void initializer_struct(struct symbol *psym, unsigned type, unsigned storage)
{
    struct symbol *sym = symbol_ref(type);
    unsigned *p = sym->idx;
    unsigned n = *p;
    unsigned s = p[1];	/* Size of object (needed for union) */

    p += 2;
    /* We only initialize the first object */
    if (sym->storage == S_UNION)
        n = 1;
    require(T_LCURLY);
    while(n-- && token != T_RCURLY) {
        type = p[1];
        p += 3;
        initializers(psym, type, storage);
        if (!match(T_COMMA))
            break;
    }
    require(T_RCURLY);
    /* For a union zerofill the slack if other elements are bigger */
    /* For a struct fill from the offset of the next field to the size of
       the base object */
#if 0
    if (sym->storage == S_UNION)
        put_padding(s - type_sizeof(type), storage);
    else if (n)
        put_padding(s - p[2], storage);	/* From offset of field to end */
#endif
}

static void initializer_array(struct symbol *sym, unsigned type, unsigned depth, unsigned storage)
{
    unsigned n = array_dimension(n, depth);
    if (depth < array_num_dimensions(type)) {
        require(T_LCURLY);
        while(n--)
            initializer_array(sym, type, depth + 1, storage);
        require(T_RCURLY);
    } else {
        if (IS_STRUCT(type) && !PTR(type))
            initializer_struct(sym, type, storage);
        else
            initializer_group(sym, type, array_dimension(type, depth), storage);
    }
}

void initializers(struct symbol *sym, unsigned type, unsigned storage)
{
    /* FIXME: review pointer rule */
    if (PTR(type)) {
        initializer_single(sym, type, storage);
        return;
    }
    if (IS_SIMPLE(type) && IS_ARITH(type)) {
        initializer_single(sym, type, storage);
        return;
    }
    /* No complex stack initializers, for now at least */
    if (storage == S_AUTO) {
        error("not a valid auto initializer");
        return;
    }
    if (storage == S_EXTERN) {
        error("cannot initialize external");
        return;
    }
    if (IS_FUNCTION(type))
        error("init function");	/* Shouldn't get here, we don't use "=" for
                                   function forms even if it would be more
                                   logical than the C syntax */
    else if (IS_ARRAY(type))
        initializer_array(sym, type, 0, storage);
    else if (IS_STRUCT(type))
        initializer_struct(sym, type, storage);
    else
        error("cannot initialize this type");
}
