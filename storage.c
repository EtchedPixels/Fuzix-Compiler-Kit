#include "compiler.h"

unsigned is_storage_word(void)
{
	return (token >= T_AUTO && token <= T_STATIC);
}

unsigned get_storage(unsigned dflt)
{
	skip_modifiers();
	if (!is_storage_word())
		return dflt;
	switch (token) {
	case T_AUTO:
		return AUTO;
	case T_REGISTER:
		return AUTO;	/* For now */
	case T_STATIC:
		if (dflt == T_AUTO)
			return LSTATIC;
		else
			return STATIC;
	case T_EXTERN:
		return EXTERN;
	}
	/* gcc */
	return 0;
}

/*
 *	Storage I/O - hacks for now
 */

void put_typed_data(struct node *n, unsigned storage)
{
	write(1, "%[", 2);
	if (n->op != T_PAD && !is_constname(n))
		error("not constant");
	n->snum = storage;
	write(1, n, sizeof(struct node));
}

void put_padding_data(unsigned space, unsigned storage)
{
	struct node *n = make_constant(space, UINT);
	n->op = T_PAD;
	put_typed_data(n, storage);
	free_node(n);
}
