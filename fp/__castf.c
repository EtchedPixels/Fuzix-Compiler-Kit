#include "libfp.h"

unsigned long _castf_ul(uint32_t a1)
{
	int shift;
    if ((a1 & 0x7FFFFFFFUL)==0)
		return 0;
	shift = 24-EXP(a1)+EXCESS;
	if (shift>=24)
		return 0;
	if (shift<0)
		return MANT(a1)<<-shift;
	return MANT(a1) >> shift;
}

unsigned _castf_u(uint32_t a1)
{
	return _castf_ul(a1);
}

unsigned char _castf_uc(uint32_t a1)
{
	return _castf_ul(a1);
}

long _castf_l(uint32_t a1)
{
    if (a1 == 0)
        return 0;
    if (a1 & 0x80000000)
        return -_castf_ul(_negatef(a1));
    return _castf_ul(a1);
}

int _castf_(uint32_t a1)
{
    return _castf_l(a1);
}

signed char _castf_c(uint32_t a1)
{
    return _castf_l(a1);
}
