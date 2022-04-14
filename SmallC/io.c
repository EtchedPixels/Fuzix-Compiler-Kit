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
