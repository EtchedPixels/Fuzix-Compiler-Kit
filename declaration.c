/*
 *	Declaration handling. All cases are actually the same, including
 *	typedef (hence the weird typedef syntax)
 *
 *	We don't deal with the comma cases yet (ie int a, *b)
 */

#include "compiler.h"


unsigned one_typedef(unsigned storage, unsigned type, unsigned name, unsigned unused)
{
	struct symbol *sym;
	if (name == 0) {
		error("invalid typedef");
		junk();
		return 0;
	}
	sym = find_symbol(name);
	if (sym) {
		error("name already in use");
		return 1;
	}
	sym = alloc_symbol(name, 0);
	sym->type = type;
	sym->storage = S_TYPEDEF;
	sym->name = name;
	return 1;
}

void dotypedef(void)
{
	type_iterator(0, 0, 0, one_typedef);
}

unsigned one_declaration(unsigned s, unsigned type, unsigned name, unsigned defstorage)
{
	struct symbol *sym;

	/* It's quite valid C to just write "int;" but usually dumb except
	   that it's used for struct and union */
	if (name == 0) {
		if (!IS_STRUCT(type))
			warning("useless declaration");
		return 1;
	}
	if (s == S_AUTO && defstorage == S_EXTDEF)
		error("no automatic globals");

	if (IS_FUNCTION(type) && !PTR(type) && s == S_EXTDEF)
		s = S_EXTERN;

	/* Do we already have this symbol */
	sym = update_symbol_by_name(name, s, type);

	if (IS_FUNCTION(type) && !PTR(type))
		return 0;

	if ((PTR(type) || !IS_FUNCTION(type)) && match(T_EQ)) {
		if (sym->flags & INITIALIZED)
			error("duplicate initializer");
		sym->flags |= INITIALIZED;
		if (s >= S_LSTATIC)
		        header(H_DATA, sym->name, target_alignof(type));
		initializers(sym, type, s);
		if (s >= S_LSTATIC)
		        footer(H_DATA, sym->name, 0);
	}
	if (s == S_AUTO)
		sym->offset = assign_storage(type, S_AUTO);
	return 1;
}

void declaration(unsigned defstorage)
{
	unsigned s = get_storage(defstorage);
	type_iterator(s, CINT, defstorage, one_declaration);
}
