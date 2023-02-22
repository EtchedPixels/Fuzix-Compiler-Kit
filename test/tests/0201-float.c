/*
 *	unsigned integer comparisons
 */
int test_flt(float a, float b)
{
    return a < b;
}

int test_fgt(float a, float b)
{
    return a > b;
}

int test_fle(float a, float b)
{
    return a <= b;
}

int test_fge(float a, float b)
{
    return a >= b;
}

int main(int argc, char *argv[])
{
    if (test_flt(33.6, 11.4) == 1)
        return 1;
    if (test_fgt(33.6, 11.4) == 0)
        return 2;
    if (test_fge(33.6, 11.4) == 0)
        return 3;
    if (test_fle(33.6, 11.4) == 1)
        return 4;
    return 0;
}
