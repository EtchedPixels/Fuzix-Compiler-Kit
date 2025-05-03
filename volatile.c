#include "compiler.h"

static unsigned volatiles[NUM_VOLATILE];
static unsigned *nextv = volatiles;

unsigned is_volatile(register unsigned sym)
{
    register unsigned *p = volatiles;
    while(p != nextv)
        if (*p++ == sym)
            return 1;
    return 0;
}

void add_volatile(unsigned sym)
{
    if (is_volatile(sym))
        return;
    if (nextv == &volatiles[NUM_VOLATILE])
        error("too many volatiles");
    *nextv++ = sym;
}

