/*
 *	Signed integer comparisons.
 *	Check each quadrant
 */

int test_lt(int a, int b)
{
    return a < b;
}

int test_lt8(signed char a, signed char b)
{
    return a < b;
}

int main(int argc, char *argv[])
{
    /* Both positive */
    if (test_lt(33, 11) == 1)
        return 1;
    /* Both negative */
    if (test_lt(-4, -7) == 1)
        return 2;
    /* +ve / -ve */
    if (test_lt(39, -32767) == 1)
        return 3;
    /* -ve / +ve */
    if (test_lt(-1, 5) == 0)
        return 4;
    /* zero */
    if (test_lt(-1, 0) == 0)
        return 5;
    if (test_lt(0, -1) == 1)
        return 6;
    /* Both positive */
    if (test_lt8(33, 11) == 1)
        return 7;
    /* Both negative */
    if (test_lt8(-4, -7) == 1)
        return 8;
    /* +ve / -ve */
    if (test_lt8(39, -127) == 1)
        return 9;
    /* -ve / +ve */
    if (test_lt8(-1, 5) == 0)
        return 10;
    /* zero */
    if (test_lt8(-1, 0) == 0)
        return 11;
    if (test_lt8(0, -1) == 1)
        return 12;
    return 0;
}
