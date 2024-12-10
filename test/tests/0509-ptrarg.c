
void narg(char c, char *pb)
{
    *pb |= c;
}

int main(int argc, char *argv[])
{
    char p = 0;
    narg(4, &p);
    if (p != 4)
        return 1;
    return 0;
}
