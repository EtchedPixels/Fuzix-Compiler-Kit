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
	unsigned argsave, locsave;
	struct symbol *ltop;
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

	if (IS_FUNCTION(type) && !PTR(type)) {
		if (token == T_LCURLY) {
			if (s == S_EXTDEF)
				header(H_EXPORT, name, 0);
			ltop = mark_local_symbols();
			mark_storage(&argsave, &locsave);
			function_body(s, name, type);
			pop_local_symbols(ltop);
			pop_storage(&argsave, &locsave);
			return 0;
		} else if (s == S_EXTDEF)
			s = S_EXTERN;
	}
	/* Do we already have this symbol */
	sym = update_symbol(name, s, type);

	if (sym->flags & INITIALIZED)
		error("duplicate initializer");
	if ((PTR(type) || !IS_FUNCTION(type)) && match(T_EQ)) {
		sym->flags |= INITIALIZED;
		if (s >= S_LSTATIC)
		        header(H_DATA, sym->name, target_alignof(type));
		initializers(sym, type, s);
		if (s >= S_LSTATIC)
		        footer(H_DATA, sym->name, 0);
	}
	if (s == AUTO)
		sym->offset = assign_storage(type, S_AUTO);
	return 1;
}

void declaration(unsigned defstorage)
{
	unsigned s = get_storage(defstorage);
	next_token();
	type_iterator(s, CINT, defstorage, one_declaration);
}
