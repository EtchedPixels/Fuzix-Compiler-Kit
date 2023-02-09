
long mul(long a, long b)
{
    return a * b;
}

unsigned long divu(unsigned long a, unsigned long b)
{
    return a / b;
}

unsigned long modu(unsigned long a, unsigned long b)
{
    return a % b;
}

long divs(long a, long b)
{
    return a / b;
}

long mods(long a, long b)
{
    return a % b;
}

int main(int argc, char *argv[])
{
    if (mul(0xFF,0xFF) != 65025U)
        return 1;
    if (mul(0xFF,0) != 0)
        return 2;
    if (divu(200, 10) != 20)
        return 3;
    if (divu(209, 10) != 20)
        return 4;
    if (divs(200, 10) != 20)
        return 5;
    if (divs(200, -10) != -20)
        return 6;
    if (divs(-200, 10) != -20)
        return 7;
    if (modu(1000, 10) != 0)
        return 8;
    if (modu(1005, 10) != 5)
        return 9;
    if (mods(1000, 10) != 0)
        return 10;
    if (mods(1005, 10) != 5)
        return 11;
    if (mods(-1005, 10) != 5)
        return 12;
    if (mods(-1005, -10) != -5)
        return 13;
    if (mods(1005, -10) != -5)
        return 14;
    return 0;
}
