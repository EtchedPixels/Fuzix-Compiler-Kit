/*
 *	IEEE floats have a sign bit so this is trivial
 *
 *	Need to deal with all the nan etc corner cases maybe
 */

#include "libfp.h"

uint32_t _negatef(uint32_t a)
{
    a ^= 0x80000000;
    return a;
}

uint32_t _minusf(uint32_t a1, uint32_t a2)
{
    uint32_t r = _negatef(a2);
    return _negatef(_plusf(a1, r));
}
