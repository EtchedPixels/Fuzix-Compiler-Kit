static unsigned char byteop(void)
{
    register unsigned char i = 16;

    while (i--);
    return i;
}

static unsigned int preop(void)
{
    register unsigned i = 16;
    unsigned j = 0;

    while (i--)
        j++;
    return j;
}

int main(int argc, char *argv[])
{
    register int x = 0;
    if (x != 0)
        return 1;
    while(x++ < 30);
    if (x != 31)
        return 2;
    if (byteop() != 0xFF)
        return 3;
    if (preop() != 16)
        return 4;
    return 0;
}
