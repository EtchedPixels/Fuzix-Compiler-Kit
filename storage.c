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
