

long left4(unsigned long x)
{
    return x << 4;
}

long right4(long x)
{
    return x >> 4;
}

long lshift(unsigned long x, unsigned long y)
{
    return x << y;
}

long rshiftu(unsigned long x, unsigned long y)
{
    return x >> y;
}

long rshifts(long x, unsigned long y)
{
    return x >> y;
}

/* TODO - should test shifts by optimized paths 8,16,24 based */

int main(int argc, char *argv[])
{
    if (left4(2) != 32)
        return 1;
    if (right4(32) != 2)
        return 2;
    if (right4(0x80000000) != 0xF8000000)
        return 3;
    if (lshift(0x55, 25) != 0xAA000000)
        return 4;
    if (rshiftu(0x30000000, 28) != 0x03)
        return 5;
    if (rshifts(0x3000, 12) != 0x03)
        return 6;
    if (rshifts(0x80000000, 12) != 0xFFF80000)
        return 7;
    if (rshifts(0x01020304, 0) != 0x01020304)
        return 8;
    if (lshift(0x01020304, 0) != 0x01020304)
        return 9;
    return 0;
}
