/*
 *	Declaration handling. All cases are actually the same, including
 *	typedef (hence the weird typedef syntax)
 *
 *	We don't deal with the comma cases yet (ie int a, *b)
 */

#include <stdio.h>
#include "compiler.h"

void dotypedef(void)
{
	unsigned name;
	unsigned type = type_and_name(&name, 1, UNKNOWN);
	struct symbol *sym;
	if (type == UNKNOWN || name == 0) {
		error("invalid typedef");
		junk();
		return;
	}
	sym = find_symbol(name);
	if (sym) {
		error("name already in use");
		return;
	}
	sym = alloc_symbol(name, 0);
	sym->type = type;
	sym->storage = S_TYPEDEF;
	sym->name = name;
}

void declaration(unsigned defstorage)
{
	unsigned type;
	unsigned name;
	unsigned s = get_storage(defstorage);
	struct symbol *sym;
	struct symbol *ltop;

	/* Create a local symbol context for the arguments - they are
	   local names for the function body */
	ltop = mark_local_symbols();
	type = type_and_name(&name, 1, CINT);

	if (name == 0) {
		junk();
		pop_local_symbols(ltop);
		return;
	}

	if (s == S_AUTO && defstorage == S_EXTDEF)
		error("no automatic globals");

	if (IS_FUNCTION(type) && !PTR(type)) {
		fprintf(stderr, "body? %x\n", token);
		if (token == T_LCURLY) {
			function_body(s, name, type);
			return;
		} else if (s == S_EXTDEF)
			s = S_EXTERN;
	}

	/* Do we already have this symbol */
	sym = update_symbol(name, s, type);
	if ((PTR(type) || !IS_FUNCTION(type)) && match(T_EQ))
		initializers(type, s);
	need_semicolon();
}
