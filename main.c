/*
 *	Compiler pass main loop
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "compiler.h"

/*
 *	A C program consists of a series of declarations that by default
 *	are external definitions.
 */
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
	/* No write out any uninitialized variables */
	write_bss();
}
