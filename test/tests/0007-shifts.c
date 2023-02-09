

int left4(unsigned x)
{
    return x << 4;
}

int right4(int x)
{
    return x >> 4;
}

int lshift(unsigned x, unsigned y)
{
    return x << y;
}

int rshiftu(unsigned x, unsigned y)
{
    return x >> y;
}

int rshifts(int x, unsigned y)
{
    return x >> y;
}

int main(int argc, char *argv[])
{
    if (left4(2) != 32)
        return 1;
    if (right4(32) != 2)
        return 2;
    if (right4(0x8000) != 0xF800)
        return 3;
    if (lshift(0x55, 8) != 0x5500)
        return 4;
    if (rshiftu(0x3000, 12) != 0x03)
        return 5;
    if (rshifts(0x3000, 12) != 0x03)
        return 6;
    if (rshifts(0x8000, 12) != 0xFFF8)
        return 7;
    if (rshifts(12, 0) != 12)
        return 8;
    if (lshift(0x55AA, 0) != 0x55AA)
        return 9;
    return 0;
}
