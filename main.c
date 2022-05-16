/*
 *	Compiler pass main loop
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "compiler.h"

FILE *debug;

/*
 *	A C program consists of a series of declarations that by default
 *	are external definitions.
 */
static void toplevel(void)
{
	if (token == T_TYPEDEF) {
		next_token();
		dotypedef();
	} else
		declaration(S_EXTDEF);
}

int main(int argc, char *argv[])
{
	next_token();
	init_nodes();
	if (argv[1]) {
		debug = fopen(argv[1], "w");
		if (debug == NULL) {
			perror(argv[1]);
			return 255;
		}
	}
	while (token != T_EOF)
		toplevel();
	/* No write out any uninitialized variables */
	write_bss();
	out_write();
	return 0;
}
