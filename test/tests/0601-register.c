static unsigned char byteop(void)
{
    register unsigned char i = 16;

    while (i--);
    return i;
    /* buf[i] = rj_sbox(buf[i]); */
} /* aes_subBytes */



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
    return 0;
}
