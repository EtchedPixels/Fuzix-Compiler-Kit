/*
 *	Assign offsets in the stack frame
 */

#include <stdio.h>
#include "compiler.h"

static unsigned arg_frame;
static unsigned local_frame;

/*
 *	We will need to deal with alignment rules later
 */

unsigned assign_storage(unsigned type, unsigned storage)
{
    unsigned s = type_sizeof(type);
    unsigned a = 0; /* type_alignof(type) */
    unsigned *p = &arg_frame;

    if (storage == S_AUTO)
        p = &local_frame;

    *p = (*p + a) & ~a;
    a = *p;
    *p += s;
    return a;
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
