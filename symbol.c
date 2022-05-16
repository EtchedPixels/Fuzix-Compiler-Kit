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
struct symbol *last_sym = symtab - 1;
struct symbol *local_top = symtab;

struct symbol *symbol_ref(unsigned type)
{
	return symtab + INFO(type);
}

/* Local symbols have priority in all cases */
/* Find a symbol in the normal name space */
struct symbol *find_symbol(unsigned name)
{
	struct symbol *s = last_sym;
	struct symbol *gmatch = NULL;
	/* Walk backwards so that the first local we find is highest priority
	   by scope */
	while (s >= symtab) {
		if (s->name == name && s->storage < S_TYPEDEF) {
			if (s->storage <= S_LSTATIC)
				return s;
			else	/* Still need to look for a local */
				gmatch = s;
		}
		s--;
	}
	return gmatch;
}

struct symbol *find_symbol_by_class(unsigned name, unsigned class)
{
	struct symbol *s = last_sym;
	struct symbol *gmatch = NULL;
	/* Walk backwards so that the first local we find is highest priority
	   by scope */
	while (s >= symtab) {
		if (s->name == name && s->storage == class) {
			if (s->storage <= S_LSTATIC)
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
		if (s->storage <= S_LSTATIC) {
			s->storage = S_FREE;
			s->name = 0;
		}
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

/*
 *	Create or update a symbol. Warn about any symbol we are hiding.
 *	A symbol can be setup as C_ANY meaning "we've no idea yet" to hold
 *	the slot. Once the types are found it will get updated with the
 *	types and any checking done.
 */
struct symbol *update_symbol(struct symbol *sym, unsigned name, unsigned storage,
			     unsigned type)
{
	unsigned local = 0;
	if (storage <= S_LSTATIC)
		local = 1;
	if (sym != NULL && sym->type != C_ANY) {
		if (type == C_ANY)
			return sym;
		if (sym->storage > S_TYPEDEF)
			error("invalid name");
		else if (sym->storage <= S_LSTATIC || !local) {
			/* Type matching is going to be a good deal more complex FIXME */
			if (sym->type != type)
				typemismatch();
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
	}
	/* Insert new symbol */
	if (sym == NULL)
		sym = alloc_symbol(name, local);
	/* Fill in the new or reserved symbol */
	sym->type = type;
	sym->storage = storage;
	sym->flags = 0;
	sym->idx = 0;
	sym->offset = 0;
	return sym;
}

/* Update a symbol by name. In the case of things like typedefs we need
   to do an explicit search, otherwise we look for conventional names */
struct symbol *update_symbol_by_name(unsigned name, unsigned storage,
			     unsigned type)
{
	struct symbol *sym;
	if (storage >= S_TYPEDEF)
		sym = find_symbol_by_class(name, storage);
	else
		sym = find_symbol(name);
	return update_symbol(sym, name, storage, type);
}

/*
 *	Find or insert a function prototype. We keep these in the sym table
 *	as a handy way to get an index for types.
 *
 *	Although it has a cost we really need to fold all the equivalently
 *	typed argument sets into a single instance to save memory.
 */
static struct symbol *do_func_match(unsigned rtype, unsigned *template)
{
	struct symbol *sym = symtab;
	unsigned len = sizeof(unsigned) * (*template + 1);
	while(sym <= last_sym) {
		if (sym->storage == S_FUNCDEF && sym->type == rtype && memcmp(sym->idx, template, len) == 0) {
			return sym;
		}
		sym++;
	}
	sym = alloc_symbol(0xFFFF, 0);
	sym->storage = S_FUNCDEF;
	sym->idx = idx_copy(template, len);
	sym->type = rtype;
	return sym;
}

unsigned func_symbol_type(unsigned rtype, unsigned *template)
{
	struct symbol *s = do_func_match(rtype, template);
	return C_FUNCTION | ((s - symtab) << 3);
}

unsigned func_return(unsigned n)
{
	if (!IS_FUNCTION(n))
		return CINT;
	return symtab[INFO(n)].type;	/* Type of function is its return type */
}

unsigned *func_args(unsigned n)
{
	if (!IS_FUNCTION(n))
		return NULL;
	return symtab[INFO(n)].idx;	/* Type of function is its return type */
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

unsigned make_array(unsigned type)
{
	struct symbol *sym = alloc_symbol(0xFFFF, 0);
	sym->storage = S_ARRAY;
	sym->type = type;
	sym->flags = 0;
	sym->idx = idx_get(9);
	*sym->idx = 0;
	return C_ARRAY | ((sym - symtab) << 3);
}

void array_add_dimension(unsigned type, unsigned num)
{
	struct symbol *sym = symbol_ref(type);
	unsigned *idx = sym->idx;
	if (*idx == 8)
		error("too many dimensions");
	else {
		(*idx)++;
		idx[*idx] = num;
	}
}

/*
 *	Struct helpers
 */

static struct symbol *find_struct(unsigned name)
{
	struct symbol *sym = symtab;
	/* Anonymous structs are unique each time */
	if (name == 0)
		return 0;
	while(sym <= last_sym) {
		if (sym->name == name && (sym->storage == S_STRUCT || sym->storage == S_UNION))
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
	sym = find_struct(name);
	if (sym == NULL) {
		sym = alloc_symbol(name, 0);	/* TODO scoping */
		sym->storage = t;
		sym->flags = 0;
		sym->idx = NULL;	/* Not yet known */
	} else {
		if (sym->storage != t)
			error("declared both union and struct");
	}
	return sym;
}

unsigned *struct_find_member(unsigned name, unsigned fname)
{
	struct symbol *s = symtab + INFO(name);
	unsigned *ptr, idx;
	/* May be a known type but not one with fields yet declared */
	if (s->idx == NULL)
		return NULL;
	idx = *s->idx;	/* Number of fields */
	ptr = s->idx + 2;	/* num fields, sizeof */
	while(idx--) {
		/* name, type, offset tuples */
		if (*ptr == fname)
			return ptr;
		ptr += 3;
	}
	return NULL;
}

unsigned type_of_struct(struct symbol *sym)
{
	return C_STRUCT|((sym - symtab) << 3);
}

/*
 *	Generate the BSS at the end
 */

void write_bss(void)
{
	struct symbol *s = symtab;
	while(s <= last_sym) {
		if (debug)
			fprintf(debug, "sym %x %x %x %d\n", s->name, s->type, s->flags, s->storage);
		if (!IS_FUNCTION(s->type) && s->storage >= S_LSTATIC && s->storage <= S_EXTDEF) {
			if (s->storage == S_EXTDEF)
				header(H_EXPORT, s->name, 0);
			if (!(s->flags & INITIALIZED)) {
				unsigned n = type_sizeof(s->type);
				header(H_BSS, s->name, target_alignof(s->type));
				put_padding_data(n);
				footer(H_BSS, s->name, 0);
			}
		}
		s++;
	}
}
