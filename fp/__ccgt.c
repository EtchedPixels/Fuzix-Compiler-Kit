#include "libfp.h"

unsigned __ccltf(uint32_t a1, uint32_t a2)
{
    if (a1 < 0 && a2 < 0) {
        if (a2 > a1)
            return 1;
        return 0;
    }
    if (a1 > a2)
        return 1;
    return 0;
}
