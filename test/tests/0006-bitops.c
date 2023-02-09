
unsigned and(unsigned a, unsigned b)
{
    return a & b;
}

unsigned or(unsigned a, unsigned b)
{
    return a | b;
}

unsigned xor(unsigned a, unsigned b)
{
    return a ^ b;
}

unsigned bitflip(unsigned a)
{
    return ~a;
}

int main(int argc, char *argv[])
{
    if (and(4, 5) != 4)
        return 1;
    if (xor(128, 192) != 64)
        return 2;
    if (or(0x55, 0xAA) != 0xFF)
        return 3;
    if (bitflip(0xAA) != 0xFF55)
        return 4;
    return 0;
}