
int func(unsigned x)
{
    if (x != 4)
	return 1;
    return 0;
}

int func2(unsigned x, ...)
{
    if (x != 4)
	return 1;
    return 0;
}

int
main(int argc, char **argv)
{
	int	(*f)(unsigned x);
	int	(*v)(unsigned x, ...);

	f = &func;
	v = &func2;

	if (f(4))
	    return 1;
        if (v(4,4))
            return 2;
        return 0;
}