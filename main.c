/*
 *	Compiler pass main loop
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "compiler.h"

static void initializer(struct symbol *sym)
{
	if (sym->flags & INITIALIZED)
		error("duplicate initializer");
	sym->flags |= INITIALIZED;
}

static void toplevel(void)
{
	declaration(S_EXTDEF);
}

int main(int argc, char *argv[])
{
	next_token();
	init_nodes();
	while (token != T_EOF)
		toplevel();
}
