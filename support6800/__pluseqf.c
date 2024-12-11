#include "libfp.h"

//
//	*TOS += high:d
//
uint32_t _pluseqf(float a2, float *a1)
{
	return (*a1 = *a1 + a2);
}
//
//	*TOS -= high:d
//
uint32_t _minuseqf(float a2, float *a1)
{
	return (*a1 = *a1 - a2);
}
//
//	*TOS *= high:d
//
uint32_t _muleqf(float a2, float *a1)
{
	return (*a1 = *a1 * a2);
}
//
//	*TOS /= high:d
//
uint32_t _diveqf(float a2, float *a1)
{
	return (*a1 = *a1 / a2);
}
