int main(int argc, char *argv[])
{
    register int x = 0;
    if (x != 0)
        return 1;
    while(x++ < 30);
    if (x != 30)
        return 0;
    return 0;
}
