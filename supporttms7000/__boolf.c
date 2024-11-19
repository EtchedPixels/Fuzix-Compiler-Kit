#include "libfp.h"

unsigned _boolf(uint32_t a1)
{
    if (a1 & 0x7FFFFFFFUL)
        return 1;
    return 0;
}

unsigned _notf(uint32_t a1)
{
    if (a1 & 0x7FFFFFFFUL)
        return 0;
    return 1;
}
