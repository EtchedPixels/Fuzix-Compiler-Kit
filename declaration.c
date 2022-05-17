/*
 *	Declaration handling. All cases are actually the same, including
 *	typedef (hence the weird typedef syntax)
 *
 *	We don't deal with the comma cases yet (ie int a, *b)
 */

#include "compiler.h"


unsigned one_typedef(unsigned type, unsigned name)
{
	if (name == 0) {
		error("invalid typedef");
		junk();
		return 0;
	}
	update_symbol_by_name(name, S_TYPEDEF, type);
	return 1;
}

void dotypedef(void)
{
	unsigned type = get_type();
	unsigned name;

	if (type == UNKNOWN)
		type = CINT;

//	while (is_modifier() || is_type_word() || token >= T_SYMBOL || token == T_STAR) {
	while (token != T_SEMICOLON) {
		unsigned utype = type_name_parse(S_TYPEDEF, type, &name);
		if (one_typedef(utype, name) == 0)
			return;
		if (!match(T_COMMA))
			break;
	}
	need_semicolon();
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

	if (s != S_EXTERN && (PTR(type) || !IS_FUNCTION(type)) && match(T_EQ)) {
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
		sym->data.offset = assign_storage(type, S_AUTO);
	return 1;
}

void declaration(unsigned defstorage)
{
	unsigned s = get_storage(defstorage);
	unsigned name;
	unsigned utype;
	unsigned type;

	type = get_type();
	if (type == UNKNOWN)
		type = CINT;

//	while (is_modifier() || is_type_word() || token >= T_SYMBOL || token == T_STAR) {
	while (token != T_SEMICOLON) {
		utype = type_name_parse(s, type, &name);
		if (one_declaration(s, utype, name, defstorage) == 0)
			return;
		if (!match(T_COMMA))
			break;
	}
	need_semicolon();
}
