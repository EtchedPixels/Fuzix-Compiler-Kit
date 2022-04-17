/*
 *	Look up and manage symbols. We have two lists, one global and one
 *	local. To avoid two lists we keep a "last local" and "last global"
 *	pointer. This allows us to keep dumping local names whilst still
 *	being able to defines globals in local contexts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

struct symbol symtab[MAXSYM];
struct symbol *last_sym = symtab;
struct symbol *local_top = symtab;

struct symbol *symbol_ref(unsigned type)
{
	return symtab + INFO(type);
}

/* Local symbols have priority in all cases */
struct symbol *find_symbol(unsigned name)
{
	struct symbol *s = last_sym;
	struct symbol *gmatch = NULL;
	/* Walk backwards so that the first local we find is highest priority
	   by scope */
	while (s >= symtab) {
		if (s->name == name) {
			if (s->storage <= LSTATIC)
				return s;
			else	/* Still need to look for a local */
				gmatch = s;
		}
		s--;
	}
	return gmatch;
}

void pop_local_symbols(struct symbol *top)
{
	struct symbol *s = top + 1;
	while (s <= last_sym) {
		if (s->storage <= S_LSTATIC)
			s->storage = S_FREE;
		s++;
	}
	local_top = top;
}

struct symbol *mark_local_symbols(void)
{
	return local_top;
}

/* The symbols from 0 to local_top are a mix of kinds but as we have not
   discarded below that point are all full. Between that and last_sym there
   may be holes, above last_sym is free */
struct symbol *alloc_symbol(unsigned name, unsigned local)
{
	struct symbol *s = local_top;
	while (s <= &symtab[MAXSYM]) {
		if (s->storage == S_FREE) {
			if (local && local_top < s)
				local_top = s;
			if (last_sym < s)
				last_sym = s;
			s->name = name;
			s->offset = 0;
			s->idx = 0;
			s->flags = 0;
			return s;
		}
		s++;
	}
	fatal("too many symbols");
}

struct symbol *update_symbol(unsigned name, unsigned storage,
			     unsigned type)
{
	unsigned local = 0;
	struct symbol *sym;
	if (storage <= S_LSTATIC)
		local = 1;
	sym = find_symbol(name);
	if (sym != NULL) {
		if (sym->storage > S_EXTDEF)
			error("invalid name");
		else if (sym->storage <= S_LSTATIC || !local) {
			fprintf(stderr, "Found sym %d\n", name);
			/* Type matching is going to be a good deal more complex FIXME */
			if (sym->type != type)
				error("type mismatch");
			if (sym->storage == storage)
				return sym;
			/* extern foo and now found foo */
			if (sym->storage == S_EXTERN && storage == S_EXTDEF) {
				sym->storage = S_EXTDEF;
				return sym;
			}
			/* foo and now found extern foo */
			if (sym->storage == S_EXTDEF && storage == S_EXTERN)
				return sym;
			error("storage class mismatch");
			return sym;
		}
		/* We can find a matching global for our local. That's allowed
		   but bad */
		else
			warning("local name obscures global");
	}
	fprintf(stderr, "Create sym %d\n", name);
	/* Insert new symbol */
	sym = alloc_symbol(name, local);
	sym->type = type;
	sym->storage = storage;
	sym->flags = 0;
	sym->idx = 0;
	sym->offset = 0;
	return sym;
}

/*
 *	Find or insert a function prototype. We keep these in the sym table
 *	as a handy way to get an index for types.
 *
 *	Although it has a cost we really need to fold all the equivalently
 *	typed argument sets into a single instance to save memory.
 */
static struct symbol *do_func_match(unsigned *template)
{
	struct symbol *sym = symtab;
	unsigned len = *template + 1;
	while(sym <= last_sym) {
		if (sym->storage == S_FUNCDEF && memcmp(sym->idx, template, len) == 0)
			return sym;
		sym++;
	}
	sym = alloc_symbol(0, 0);
	sym->storage = S_FUNCDEF;
	sym->idx = idx_copy(template, len);
	return sym;
}

unsigned func_symbol_type(unsigned *template)
{
	return C_FUNCTION | ((do_func_match(template) - symtab) << 3);
}

unsigned func_return(unsigned n)
{
	if (!IS_FUNCTION(n))
		return CINT;
	return symtab[INFO(n)].type;	/* Type of function is its return type */
}

/*
 *	Array type helpers
 */

unsigned array_num_dimensions(unsigned type)
{
	struct symbol *sym = symbol_ref(type);
	return *sym->idx;
}

unsigned array_dimension(unsigned type, unsigned depth)
{
	struct symbol *sym = symbol_ref(type);
	return sym->idx[depth];
}

/*
 *	Struct helpers
 */

static struct symbol *find_struct(unsigned name, unsigned t)
{
	struct symbol *sym = symtab;
	while(sym <= last_sym) {
		if (sym->name == name && sym->storage == t)
			return sym;
		sym++;
	}
	return NULL;
}

struct symbol *update_struct(unsigned name, unsigned t)
{
	struct symbol *sym;
	if (t)
		t = S_STRUCT;
	else
		t = S_UNION;
	sym = find_struct(name, t);
	if (sym == NULL) {
		sym = alloc_symbol(name, 0);	/* TODO scoping */
		sym->storage = t;
		sym->flags = 0;
		sym->idx = NULL;	/* TODO */
	}
	return sym;
}


unsigned type_of_struct(struct symbol *sym)
{
	return C_STRUCT|((sym - symtab) << 3);
}
