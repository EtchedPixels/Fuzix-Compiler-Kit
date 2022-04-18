#include "compiler.h"

/*
 *	Struct declarations
 */

static void struct_add_field(struct symbol *sym, unsigned name, unsigned type)
{
    unsigned *p = sym->idx;
    unsigned n = 0;
    unsigned *t = p + 2;

    while(n < *p) {
        if (*t == name)
            error("duplicate field");
        t += 3;
        n++;
    }
    *t++ = name;
    *t++ = type;
    (*p)++;
    if (sym->storage == S_UNION) {
        /* For a union track the largest element size */
        unsigned s = type_sizeof(type);
        *t = 0;
        if (p[1] < s)
            p[1] = s;
    } else {
        /* For a struct allocate fields and track offset */
        *t = alloc_room(p + 1, type);
    }
}

/* As with the other type/name handlers we need to unify them and also deal
   with the int x,y case */
void struct_declaration(struct symbol *sym)
{
    unsigned name;
    unsigned t;
    unsigned tags[62];	/* max 20 fields per struct for the moment */
    unsigned nfield = 0;
    unsigned err = 0;

    if (sym->flags & INITIALIZED)
        error("struct declared twice");
    sym->flags |= INITIALIZED;

    /* Temporarily, as this can recurse */
    sym->idx = tags;
    
    require(T_LCURLY);
    while(token != T_RCURLY) {
        t = type_and_name(&name, 1, CINT);
        if (nfield == 20) {
            if (err == 0)
                error("too many struct/union fields");
            err = 1;
        } else {
            struct_add_field(sym, name, t);
            nfield++;
        }
        require(T_SEMICOLON);
    }
    require(T_RCURLY);

    sym->idx = idx_copy(tags, 3 + 2 * *tags);
}
