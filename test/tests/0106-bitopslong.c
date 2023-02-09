
unsigned long and(unsigned long a, unsigned long b)
{
    return a & b;
}

unsigned long or(unsigned long a, unsigned long b)
{
    return a | b;
}

unsigned long xor(unsigned long a, unsigned long b)
{
    return a ^ b;
}

unsigned long bitflip(unsigned long a)
{
    return ~a;
}

int main(int argc, char *argv[])
{
    if (and(0xFFFFF000, 0x0000FFFF) != 0x0000F000)
        return 1;
    if (xor(0xFF00FF00, 0x00FF00FF) != 0xFFFFFFFF)
        return 2;
    if (or(0x55AA55AA, 0xAA55AA55) != 0xFFFFFFFF)
        return 3;
    if (bitflip(0xAAAAAAAA) != 0x55555555)
        return 4;
    return 0;
}