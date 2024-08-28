#include "libfp.h"

uint32_t _castul_f(unsigned long a1)
{
    int exp = 24 + EXCESS;
    if (a1 == 0)
        return 0;		/* Float 0 is the same */
    /* Move down until our first one bit is the implied bit */
    while(a1 & 0xFF000000UL) {
        exp++;
        a1 >>= 1;
    }
    /* Move smaller numbers up until the first 1 bit is in the implied 1
       position */
    while(!(a1 & 0x01000000)) {
        exp--;
        a1 <<= 1;
    }
    /* And assemble */
    return PACK(0, exp, a1);
}

/* We could just use the uint32_t helper but 16bit is actually much simpler */
uint32_t _castu_f(unsigned a1)
{
    uint32_t r;
    int exp = 24 + EXCESS;

    if (a1 == 0)
        return a1;
    r = a1;
    while(!(r & 0x01000000)) {
        exp--;
        r <<= 1;
    }
    return PACK(0, exp, r);
}

uint32_t _castuc_f(unsigned char a1)
{
    uint32_t r;
    int exp = 24 + EXCESS;

    if (a1 == 0)
        return a1;
    r = a1;
    while(!(r & 0x01000000)) {
        exp--;
        r <<= 1;
    }
    return PACK(0, exp, r);
}

uint32_t _castl_f(long a1)
{
    if (a1 < 0)
        return _negatef(_castul_f(-a1));
    return _castul_f(a1);
}

uint32_t _cast_f(int a1)
{
    if (a1 < 0)
        return _negatef(_castu_f(-a1));
    return _castu_f(a1);
}

uint32_t _castc_f(signed char a1)
{
    if (a1 < 0)
        return _negatef(_castuc_f(-a1));
    return _castuc_f(a1);
}
