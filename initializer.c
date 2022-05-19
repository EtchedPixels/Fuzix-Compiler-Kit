#include "compiler.h"

/*
 *	Write a single initialization element to the stream. For auto variables
 *	we generate the assignment tree, for static or globals we generate a
 *	stream of data with types for the backend.
 */
static void initializer_single(struct symbol *sym, unsigned type, unsigned storage)
{
    struct node *n = typeconv(expression_tree(0), type, 1);
    if (storage == S_AUTO) {
        n = tree(T_EQ, make_symbol(sym), n);
        write_tree(n);
    } else {
        put_typed_data(n);
        free_tree(n);
    }
}

/* C99 permits trailing comma and ellipsis */
/* Strictly {} is not permitted - there must be at least one value */

/*
 *	Array bottom level initializer: repeated runs of the same type
 *
 *	TODO: In theory we could have a platform that needs padding
 *	and we don't deal with that aspect of alignment yet
 */
static void initializer_group(struct symbol *sym, unsigned type, unsigned n, unsigned storage)
{
    /* C has a funky special case rule that you can write
       char x[16] = "foo"; which creates a copy of the string in that
       array not a literal reference */
    if (token == T_STRING) {
        if ((type_canonical(type) & ~UNSIGNED) != CCHAR)
            typemismatch();
        copy_string(0, n, 1);
        return;
    }
    require(T_LCURLY);
    while(n && token != T_RCURLY) {
        if (token == T_ELLIPSIS)
            break;
        n--;
        initializer_single(sym, type, storage);
        if (!match(T_COMMA))
            break;
    }
    if (n) {
        unsigned s = type_sizeof(type) * n;
        put_padding_data(s);
    }
    /* Catches any excess elements */
    require(T_RCURLY);
}

/*
 *	Struct and union initializer
 *
 *	This is similar to an array but each element has its own expected
 *	type, and some elements may themselves be structures or arrays. It's
 *	mostly recursion.
 *
 *	Remaining space in the object is padded.
 *
 *	We don't deal with auto here as with arrays because we don't support
 *	the C extensions of auto array and struct with initializers.
 */
static void initializer_struct(struct symbol *psym, unsigned type, unsigned storage)
{
    struct symbol *sym = symbol_ref(type);
    unsigned *p = sym->data.idx;
    unsigned n = *p;
    unsigned s = p[1];	/* Size of object (needed for union) */
    unsigned pos = 0;

    p += 2;
    /* We only initialize the first object */
    if (S_STORAGE(sym->infonext) == S_UNION)
        n = 1;
    require(T_LCURLY);
    /* FIXME: we need to watch the offsets and add internal padding */
    while(n-- && token != T_RCURLY) {
        /* Name, type, offset tuples */
        type = p[1];

        /* Align */
        if (pos != p[2]) {
            put_padding_data(p[2] - pos);
            pos = p[2];
        }
        /* Write out field */
        initializers(psym, type, storage);
        pos += type_sizeof(type);

        /* Next field */
        p += 3;

        if (!match(T_COMMA))
            break;
    }
    if (n == -1 && token != T_RCURLY)
        error("too many initializers");
    require(T_RCURLY);
    /* For a union zerofill the slack if other elements are bigger */
    /* For a struct fill from the offset of the next field to the size of
       the base object */
    if (pos != s)
        put_padding_data(s - pos);	/* Fill remaining space */
}

/*
 *	Array initializer.
 *
 *	We recursively call down through the layers until we hit the bottom
 *	layer of the array which should be a series of values in the type
 *	of the array. The base value may be a structure.
 */
static void initializer_array(struct symbol *sym, unsigned type, unsigned depth, unsigned storage)
{
    unsigned n = array_dimension(type, depth);

    if (depth < array_num_dimensions(type)) {
        require(T_LCURLY);
        while(n--)
            initializer_array(sym, type_deref(type), depth + 1, storage);
        require(T_RCURLY);
    } else {
        type = type_deref(type);
        if (IS_STRUCT(type) && !PTR(type)) {
            require(T_LCURLY);
            initializer_struct(sym, type, storage);
            require(T_RCURLY);
        } else
            initializer_group(sym, type, n, storage);
    }
}

/*
 *	Initialize an object.
 */
void initializers(struct symbol *sym, unsigned type, unsigned storage)
{
    /* FIXME: review pointer rule */
    if (PTR(type) && !IS_ARRAY(type)) {
        initializer_single(sym, type, storage);
        return;
    }
    if (IS_ARITH(type)) {
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
        initializer_array(sym, type, 1, storage);
    else if (IS_STRUCT(type))
        initializer_struct(sym, type, storage);
    else
        error("cannot initialize this type");
}
