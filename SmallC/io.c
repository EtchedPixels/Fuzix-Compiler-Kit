/*      File io.c: 2.1 (83/03/20,16:02:07) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"
#include "data.h"

#if 0
/*
 *      open input file
 */
int openin(char *p)
{
        strcpy(fname, p);
        fixname (fname);
        if (!checkname (fname))
                return (NO);
        if ((input = open(fname, O_RDONLY)) == -1) {
                pl ("Open failure\n");
                return (NO);
        }
        return (YES);

}

/*
 *      open output file
 */
int openout(void)
{
        outfname (fname);
        if ((output = open (fname, O_WRONLY|O_TRUNC|O_CREAT, 0644)) == -1) {
                pl ("Open failure");
                return (NO);
        }
        return (YES);

}

/*
 *      change input filename to output filename
 */
void outfname(char *s)
{
        while (*s)
                s++;
        *--s = 's';

}

/**
 * remove NL from filenames
 */
void fixname(char *s)
{
        while (*s && *s++ != LF);
        if (!*s) return;
        *(--s) = 0;

}

/**
 * check that filename is "*.c"
 */
int checkname(char *s)
{
        while (*s)
                s++;
        if (*--s != 'c')
                return (NO);
        if (*--s != '.')
                return (NO);
        return (YES);

}
#endif
/*
 *      print a carriage return and a string only to console
 *
 */
void pl (char *str)
{
        write(1, "\n", 1);
        write(1, str, strlen(str));
}


void writee(char *str)
{
        write(2, str, strlen(str));
}
