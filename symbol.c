/*
 *	Look up and manage symbols. We have two lists, one global and one
 *	local. To avoid two lists we keep a "last local" and "last global"
 *	pointer. This allows us to keep dumping local names whilst still
 *	being able to defines globals in local contexts.
 */

#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"

struct symbol symtab[MAXSYM];
struct symbol *last_sym = symtab;
struct symbol *local_top = symtab;

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
	struct symbol *s = top;
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
			s->offset = 0;
			s->idx = 0;
			s->flags = 0;
			return s;
		}
		s++;
	}
	error("too many symbols");
	exit(1);
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
		if (sym->storage <= S_LSTATIC || !local) {
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
	sym->name = name;
	sym->type = type;
	sym->storage = storage;
	sym->flags = 0;
	sym->idx = 0;
	sym->offset = 0;
	return sym;
}
