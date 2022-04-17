#include <stdio.h>

#include "compiler.h"

/* Eventually remove stdio usage */

unsigned errors;

void warning(const char *p)
{
	fprintf(stderr, "line %d:%s\n", line_num, p);
}

void error(const char *p)
{
	warning(p);
	errors++;
}

void errorc(const unsigned c, const char *p)
{
	fprintf(stderr, "line %d:'%s '%c'\n", line_num, p, c);
	errors++;
}

void needlval(void)
{
	error("lvalue required");
}

void badtype(void)
{
	error("bad type");
}


void indirections(void)
{
	error("too many indirections");
}
