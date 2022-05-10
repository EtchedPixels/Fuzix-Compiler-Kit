#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"

/* Eventually remove stdio usage */

unsigned errors;

void warningline(unsigned line, const char *p)
{
	fprintf(stderr, "line %d:%s\n", line, p);
}

void warning(const char *p)
{
	warningline(line_num, p);
}

void errorline(unsigned line, const char *p)
{
	warningline(line, p);
	errors++;
}

void error(const char *p)
{
	warningline(line_num, p);
	errors++;
}

void fatal(const char *p)
{
	error(p);
	exit(255);
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

void typemismatch(void)
{
	error("type mismatch");
}

void invalidtype(void)
{
	error("invalid type");
}

void divzero(void)
{
	warning("division by zero");
}
