/*      File error.c: 2.1 (83/03/20,16:02:00) */
/*% cc -O -c %
 *
 */

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "defs.h"
#include "data.h"

void errchar(char c)
{
        write(2, &c, 1);
}

void warning(char *ptr)
{
        fprintf(stderr, "line %d: ", line_num);
        write(2, ptr, strlen(ptr));
        errchar('\n');
}

void error(char *ptr)
{
        warning(ptr);
        errcnt++;
}
