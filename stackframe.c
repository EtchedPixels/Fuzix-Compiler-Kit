/*
 *	Assign offsets in the stack frame (or elsewhere)
 *
 *	FIXME: make these structs and track max too
 */

#include <stdio.h>
#include "compiler.h"

static unsigned arg_frame;
static unsigned local_frame;

/*
 *	We will need to deal with alignment rules later
 */

unsigned alloc_room(unsigned *p, unsigned type)
{
    unsigned s = type_sizeof(type);
    unsigned a = 0; /* type_alignof(type) */

    *p = (*p + a) & ~a;
    a = *p;
    *p += s;
    return a;
}

unsigned assign_storage(unsigned type, unsigned storage)
{
    unsigned *p = &arg_frame;
    if (storage == S_AUTO)
        p = &local_frame;
    return alloc_room(p, type);
}

void mark_storage(unsigned *a, unsigned *b)
{
    *a = arg_frame;
    *b = local_frame;
}

void pop_storage(unsigned *a, unsigned *b)
{
    arg_frame = *a;
    local_frame = *b;
}
