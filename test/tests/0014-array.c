
int x[8][8];
char y[8][8];
int z[8];

int main(int argc, char *argv[])
{
    int *p = (int *)x;
    char *q = (char *)y;
    int *r = z;

    if (sizeof(z) != 16)
        return 1;
    if (r != &z[0])
        return 2;
    z[0] = 1;
    if (*r != 1)
        return 3;
    z[0]++;
    if (*r != 2)
        return 4;
    z[0]--;
    if (*r != 1)
        return 5;

    if (sizeof(x) != 128)
        return 6;
    if (sizeof(y) != 64)
        return 7;

    if (p != &x[0][0])
        return 8;
    x[0][0] = 1;
    if (*p != 1)
        return 9;
    x[0][0]++;
    if (*p != 2)
        return 10;
    x[0][0]--;
    if (*p != 1)
        return 11;

    if (q != &y[0][0])
        return 12;
    y[0][0] = 1;
    if (*q != 1)
        return 13;
    y[0][0]++;
    if (*q != 2)
        return 14;
    y[0][0]--;
    if (*q != 1)
        return 15;

    return 0;
}

