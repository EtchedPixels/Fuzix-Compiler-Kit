#include "libfp.h"

/* TODO; check rules on NaN etc */

unsigned _ccnef(uint32_t a2, uint32_t a1)
{
    if (a1 == a2)
        return 0;
    if ((a1 | a2) & 0x7FFFFFFFUL)
        return 1;
    return 0;
}
