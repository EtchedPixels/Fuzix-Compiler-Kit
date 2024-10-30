/*
 *	6800: original in the line of processors ending with 68HC11
 *	- Only has 8bit operations
 *	- Cannot push x (hack to pop it)
 *	- Cannot  move between accumulators and X except via memory
 *	- Can copy X to and from S but can only move to/from accumulator
 *	  via memory
 *	- Some useful flag behaviour on late devices is not present.
 *
 *	6803:
 *	- Adds 16bit operations
 *	- Adds ABX
 *	- Can push or pull X
 *
 *	6303: like 6803
 *	- Adds some interesting bit ops
 *	- Adds XGDX
 *
 *	68HC11: like 6803
 *	- Adds a bunch of bitops, branch on bit and other stuff
 *	- Adds a Y register
 *	- Has XGDX and XGDY
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"
#include "backend-6800.h"


/*
 *	Helpers for code generation and tracking
 */

/*
 *	Size handling
 */
unsigned get_size(unsigned t)
{
	if (PTR(t))
		return 2;
	if (t == CSHORT || t == USHORT)
		return 2;
	if (t == CCHAR || t == UCHAR)
		return 1;
	if (t == CLONG || t == ULONG || t == FLOAT)
		return 4;
	if (t == CLONGLONG || t == ULONGLONG || t == DOUBLE)
		return 8;
	if (t == VOID)
		return 0;
	error("gs");
	return 0;
}

unsigned get_stack_size(unsigned t)
{
	return get_size(t);
}

void repeated_op(unsigned n, const char *op)
{
	while(n--)
		printf("\t%s\n", op);
}

/* Chance to rewrite the tree from the top rather than none by node
   upwards. We will use this for 8bit ops at some point and for cconly
   propagation */
struct node *gen_rewrite(struct node *n)
{
	return n;
}

static void squash_node(struct node *n, struct node *o)
{
	n->value = o->value;
	n->val2 = o->val2;
	n->snum = o->snum;
	free_node(o);
}

static void squash_left(struct node *n, unsigned op)
{
	struct node *l = n->left;
	n->op = op;
	squash_node(n, l);
	n->left = NULL;
}

static void squash_right(struct node *n, unsigned op)
{
	struct node *r = n->right;
	n->op = op;
	squash_node(n, r);
	n->right = NULL;
}

/*
 *	There isn't a lot we can do the easy way except constants, so stick
 *	constants on the right when we can.
 */
static unsigned is_simple(struct node *n)
{
	unsigned op = n->op;

	/* We can load these directly */
	if (op == T_CONSTANT)
		return 10;
	/* These are as easy but we also have some helpers that benefit
	   most from right constant */
	if (op == T_NAME || op == T_LABEL)
		return 9;
	if (op == T_LBREF || op == T_NREF)
		return 9;
	if (op == T_LOCAL || op == T_ARGUMENT || op == T_LREF)
		return 9;
	return 0;
}

/*
 *	Our chance to do tree rewriting. We don't do much
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	register struct node *r = n->right;
	register struct node *l = n->left;
	register unsigned op = n->op;
	unsigned nt = n->type;
	unsigned off;

	/* Eliminate casts for sign, pointer conversion or same */
	if (op == T_CAST) {
		if (nt == r->type || (nt ^ r->type) == UNSIGNED ||
		 (PTR(nt) && PTR(r->type))) {
			free_node(n);
			return r;
		}
	}
	/* Rewrite function call of a name into a new node so we can
	   turn it easily into call xyz */
	if (op == T_FUNCCALL && r->op == T_NAME && PTR(r->type) == 1) {
		n->op = T_CALLNAME;
		n->snum = r->snum;
		n->value = r->value;
		free_node(r);
		n->right = NULL;
	}
	/* Merge offset to object into a single direct reference */
	if (op == T_PLUS && r->op == T_CONSTANT &&
		(l->op == T_LOCAL || l->op == T_NAME || l->op == T_LABEL || l->op == T_ARGUMENT)) {
		/* We don't care if the right offset is 16bit or 32 as we've
		   got 16bit pointers */
		printf(";merge right %x %lu+%lu\n",
			op, l->value, r->value);
		l->value += r->value;
		free_node(r);
		free_node(n);
		return l;
	}
	/* Label array referencing for locals to help the 6800 */
	if (optsize && !cpu_has_d) {
		if (op == T_PLUS) {
			off = 0;
			if (l->op == T_LOCAL || l->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					off = argbase + frame_len;
				squash_left(n, T_LPLUS);
				n->value += off;
				return n;
			}
		}
	}
	/* Turn a deref of an offset to an object into a single op so we can
	   generate a single lda offset,x in the code generator. This happens
	   in some array dereferencing and especially struct pointer access */
	if (op == T_DEREF || op == T_DEREFPLUS) {
		if (op == T_DEREF)
			n->value = 0;	/* So we can treat deref/derefplus together */
		if (r->op == T_PLUS) {
			off = n->value + r->right->value;
			if (r->right->op == T_CONSTANT && off < 253) {
				n->op = T_DEREFPLUS;
				free_node(r->right);
				n->right = r->left;
				n->value = off;
				free_node(r);
				/* We might then rewrite this again */
				return gen_rewrite_node(n);
			}
		}
	}
	if (op == T_EQ || op == T_EQPLUS) {
		if (op == T_EQ)
			n->value = 0;	/* So we can treat deref/derefplus together */
		if (l->op == T_PLUS) {
			off = n->value + l->right->value;
			if (l->right->op == T_CONSTANT && off < 253) {
				n->op = T_EQPLUS;
				free_node(l->right);
				n->left = l->left;
				n->value = off;
				free_node(l);
				/* We might then rewrite this again */
				return gen_rewrite_node(n);
			}
		}
	}
	/* regptr++  The size will always be the true size for ++ and const */
	if (op == T_DEREF && r->op == T_PLUSPLUS && r->left->op == T_REG)
	{
		n->op = T_RDEREFPLUS;
		n->value = r->left->value;
		free_node(r->left);
		free_node(r->right);
		free_node(r);
		n->right = NULL;
		return n;
	}
	/* *regptr++ =  again the size will be const and right */
	if (op == T_EQ && l->op == T_PLUSPLUS && l->left->op == T_REG)
	{
		n->op = T_REQPLUS;
		n->value = l->left->value;
		free_node(l->left);
		free_node(l->right);
		free_node(l);
		n->left = NULL;
		return n;
	}
	/* *(reg + offset). Optimize this specially as it occurs a lot */
	if (op == T_DEREF && r->op == T_PLUS && r->left->op == T_RREF &&
		r->right->op == T_CONSTANT) {
		n->op = T_RDEREF;
		n->right = NULL;
		n->val2 = r->right->value;	/* Offset to add */
		n->value = r->left->value;	/* Register number */
		free_node(r->right);		/* Discard constant */
		free_node(r->left);		/* Discard T_REG */
		free_node(r);			/* Discsrd plus */
		return n;
	}
	/* *regptr */
	if (op == T_DEREF && r->op == T_RREF) {
		n->op = T_RDEREF;
		n->right = NULL;
		n->val2 = 0;
		n->value = r->value;
		free_node(r);
		return n;
	}
	/* *(reg + offset) =  Optimize this specially as it occurs a lot */
	if (op == T_EQ && l->op == T_PLUS && l->left->op == T_RREF &&
		l->right->op == T_CONSTANT) {
		n->op = T_REQ;
		n->val2 = l->right->value;	/* Offset to add */
		n->value = l->left->value;	/* Register number */
		free_node(l->right);		/* Discard constant */
		free_node(l->left);		/* Discard T_REG */
		free_node(l);			/* Discsrd plus */
		n->left = NULL;
		return n;
	}
	/* *regptr = */
	if (op == T_EQ && l->op == T_RREF) {
		n->op = T_REQ;
		n->val2 = 0;
		n->value = l->value;
		n->left = NULL;
		free_node(l);
		return n;
	}
	if ((op == T_DEREF || op == T_DEREFPLUS) && r->op == T_LREF) {
		/* At this point r->value is the offset for the local */
		/* n->value is the offset for the ptr load */
		r->val2 = n->value;		/* Save the offset so it is squashed in */
		squash_right(n, T_LDEREF);	/* n->value becomes the local ref */
		return n;
	}
	if ((op == T_EQ || op == T_EQPLUS) && l->op == T_LREF) {
		/* At this point r->value is the offset for the local */
		/* n->value is the offset for the ptr load */
		l->val2 = n->value;		/* Save the offset so it is squashed in */
		squash_left(n, T_LEQ);		/* n->value becomes the local ref */
		return n;
	}
	/* Optimizations for 6809 indirect addressing */
	if (cpu_is_09 && n->value == 0 && (op == T_DEREF || op == T_DEREFPLUS) && r->op == T_NREF) {
		/* At this point r->value is the offset for the local */
		/* n->value is the offset for the ptr load */
		squash_right(n, T_NDEREF);	/* n->value becomes the local ref */
		return n;
	}
	if (cpu_is_09 && n->value == 0 && (op == T_DEREF || op == T_DEREFPLUS) && r->op == T_LBREF) {
		/* At this point r->value is the offset for the local */
		/* n->value is the offset for the ptr load */
		squash_right(n, T_LBDEREF);	/* n->value becomes the local ref */
		return n;
	}
	if (cpu_is_09 && n->value == 0 && (op == T_EQ || op == T_EQPLUS) && l->op == T_NREF) {
		squash_left(n, T_NEQ);		/* n->value becomes the local ref */
		return n;
	}
	if (cpu_is_09 && n->value == 0 && (op == T_EQ || op == T_EQPLUS) && l->op == T_LBREF) {
		l->val2 = n->value;		/* Save the offset so it is squashed in */
		squash_left(n, T_LBEQ);		/* n->value becomes the local ref */
		return n;
	}

	/* Rewrite references into a load operation */
	/* For now leave long types out of this unless we have a Y register as the @hireg forms
	   are more complex */
	if (cpu_has_y || nt == CCHAR || nt == UCHAR || nt == CSHORT || nt == USHORT || PTR(nt)) {
		if (op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += argbase + frame_len;
				squash_right(n, T_LREF);
				return n;
			}
			if (r->op == T_REG) {
				squash_right(n, T_RREF);
				return n;
			}
			if (r->op == T_NAME) {
				squash_right(n, T_NREF);
				return n;
			}
			if (r->op == T_LABEL) {
				squash_right(n, T_LBREF);
				return n;
			}
			if (r->op == T_RREF) {
				squash_right(n, T_RDEREF);
				n->val2 = 0;
				return n;
			}
		}
		if (op == T_EQ) {
			if (l->op == T_NAME) {
				squash_left(n, T_NSTORE);
				return n;
			}
			if (l->op == T_LABEL) {
				squash_left(n, T_LBSTORE);
				return n;
			}
			if (l->op == T_LOCAL || l->op == T_ARGUMENT) {
				if (l->op == T_ARGUMENT)
					l->value += argbase + frame_len;
				squash_left(n, T_LSTORE);
				return n;
			}
			if (l->op == T_REG) {
				squash_left(n, T_RSTORE);
				return n;
			}
		}
	}
	/* Commutive operations. We can swap the sides over on these */
	/* We want to put things we can easily use on the right so that
	   we have the best chance of being able to do operations without
	   having to stack values */
	if (op == T_AND || op == T_OR || op == T_HAT || op == T_STAR || op == T_PLUS) {
		if (is_simple(n->left) > is_simple(n->right)) {
			n->right = l;
			n->left = r;
		}
	}
	return n;
}


/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 */
unsigned gen_direct(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned off;
	uint16_t v;
	uint16_t hv;

	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		sp -= r->value;
		if (cpu_has_d || n->val2) /* Varargs */
			adjust_s(r->value, (func_flags & F_VOIDRET) ? 0 : 1);
		return 1;
	case T_EQ:
	case T_EQPLUS:
		off = n->value;
		v = r->value;
		/* If the left side was not simple then try and shortcut the
		   right side */
		if (cpu_has_xgdx && r->op == T_CONSTANT) {
			swap_d_x();
			if (s == 1) {
				load_b_const(v);
				op8_on_ptr("st", off);
				return 1;
			} else if (s == 2) {
				load_d_const(v);
				op16d_on_ptr("st", "st", off);
				return 1;
			} else if (s == 4 && cpu_has_y) {
				load_d_const(v);
				printf("\tldy #%u\n", (unsigned)(r->value >> 16));
				op32d_on_ptr("st", "st", off);
				return 1;
			}
		}
		/* TODO: for many CPU variants we need a list of things we
		 * can short load into D without touching X and to use them
		 * here
		 */
		break;
	case T_PLUS:
		/* So we can track this common case later */
		/* TODO: if the low word is zero or low 24 bits are 0 we can generate stuff like
			leay %d,y */
		if (r->op == T_CONSTANT && r->type != FLOAT) {
			if (s == 4 && cpu_has_y) {
				/* Handle the zero case specially as we can optimzie it, and also
				   because add_d_const will not leave carry right if it optimizes
				   itself out */
				if (r->value & 0xFFFF) {
					add_d_const(r->value & 0xFFFF);
					swap_d_y();
					printf("\tadcb #%u\n", (unsigned)((r->value >> 16) & 0xFF));
					printf("\tadca #%u\n", (unsigned)((r->value >> 24) & 0xFF));
					swap_d_y();
				} else if (cpu_is_09)
					printf("\tleay %u,y\n", (unsigned)(r->value >> 16));
				else {
					swap_d_y();
					printf("\taddd #%u\n", (unsigned)(r->value >> 16));
					swap_d_y();
				}
				return 1;
			}
			if (s == 2) {
				add_d_const(r->value);
				return 1;
			}
			if (s == 1) {
				add_b_const(r->value);
				return 1;
			}
		}
		return write_opd(r, "add", "adc", 0);
	case T_MINUS:
		if (r->op == T_CONSTANT && r->type != FLOAT) {
			if (s == 4 && cpu_has_y) {
				r->value = -r->value;
				if (r->value & 0xFFFF) {
					add_d_const(r->value);
					swap_d_y();
					printf("\tadcb #%u\n", (unsigned)((r->value >> 16) & 0xFF));
					printf("\tadca #%u\n", (unsigned)((r->value >> 24) & 0xFF));
					swap_d_y();
				} else if (cpu_has_lea)
					printf("\tleay %u,y\n", (unsigned)(r->value >> 16));
				else {
					swap_d_y();
					printf("\taddd #%u\n", (unsigned)(r->value >> 16));
					swap_d_y();
				}
				return 1;
			}
			if (s == 2) {
				add_d_const(-r->value);
				return 1;
			}
			if (s == 1) {
				add_b_const(-r->value);
				b_val -= r->value;
				return 1;
			}
		}
		return write_opd(r, "sub", "sbc", 0);
	case T_AND:
		if (r->op == T_CONSTANT) {	/* No need to type check - canno tbe float */
			v = r->value & 0xFFFF;
			hv = r->value >> 16;

			/* Check if it makes sense to do long inline. Only
			   do so if we have Y or the upper half is trivial */
			if (s == 4 && !cpu_has_y && hv && hv != 0xFFFF)
				return 0;

			if ((v & 0xFF) != 0xFF) {
				if (v & 0xFF)
					printf("\tandb #%u\n", v & 0xFF);
				else
					puts("\tclrb");
				modify_b(b_val & v);
			}
			if (s >= 2) {
				v >>= 8;
				if (v != 0xFF) {
					if (v)
						printf("\tanda #%u\n", v);
					else
						puts("\tclra");
				}
				modify_a(a_val & v);
			}
			if (s == 4) {
				if (hv == 0x0000) {
					if (cpu_has_y)
						puts("\tldy #0");
					else {
						puts("\tclr @hireg\n\tclr @hireg+1");
					}
				} else if (hv != 0xFFFF) {
					swap_d_y();
					if (hv & 0xFF)
						printf("\tandb #%u\n", hv & 0xFF);
					else
						puts("\tclrb");
					hv >>= 8;
					if (hv & 0xFF)
						printf("\tanda #%u\n", hv & 0xFF);
					else
						puts("\tclra");
					swap_d_y();
				}
			}
			return 1;
		}
		return write_op(r, "and", "and", 0);
	case T_OR:
		if (r->op == T_CONSTANT) {
			v = r->value & 0xFFFF;
			hv = r->value >> 16;
			/* Check if it makes sense to do long inline. Only
			   do so if we have Y or the upper half is trivial */
			if (s == 4 && !cpu_has_y && hv)
				return 0;
			if (v & 0xFF) {
				printf("\torb #%u\n", v & 0xFF);
				modify_b(b_val | v);
			}
			if (s >= 2) {
				v >>= 8;
				if (v)
					printf("\tora #%u\n", v);
				modify_a(a_val | v);
			}
			if (s == 4) {
				if (hv == 0xFFFF)
					puts("\tldy #0xFFFF");
				else if (hv) {
					swap_d_y();
					if (hv & 0xFF)
						printf("\torb #%u\n", hv & 0xFF);
					hv >>= 8;
					if (hv & 0xFF)
						printf("\tora #%u\n", hv & 0xFF);
					swap_d_y();
				}
			}
			return 1;
		}
		return write_op(r, "or", "or", 0);
	case T_HAT:
		if (r->op == T_CONSTANT) {
			v = r->value & 0xFFFF;
			hv = r->value >> 16;
			if (s == 4 && !cpu_has_y && hv && hv != 0xFFFF)
				return 0;
			if ((v & 0xFF) == 0xFF)
				puts("\tcomb");
			else if (v & 0xFF)
				printf("\teorb #%u\n", v & 0xFF);
			modify_b(b_val ^ v);
			if (s >= 2) {
				v >>= 8;
				if (v == 0xFF)
					puts("\tcoma");
				else if (v & 0xFF)
					printf("\teora #%u\n", v);
				modify_a(a_val ^ v);
			}
			if (s == 4) {
				if (hv == 0xFFFF && !cpu_has_y) {
					puts("\tcom @hireg\n\tcom @hireg+1");
				} else if (hv) {
					swap_d_y();
					if ((hv & 0xFF) == 0xFF)
						puts("\tcomb");
					else if (hv & 0xFF)
						printf("\teorb #%u\n", hv & 0xFF);
					hv >>= 8;
						if (hv == 0xFF)
						puts("\tcoma");
					else if (hv & 0xFF)
						printf("\teora #%u\n", hv);
					swap_d_y();
				}
			}
			return 1;
		}
		return write_op(r, "eor", "eor", 0);
	case T_LTLT:
		return left_shift(n);
	case T_GTGT:
		return right_shift(n);
	case T_EQEQ:
		return cmp_direct(n, "booleq", "booleq");
	case T_BANGEQ:
		return cmp_direct(n, "boolne", "boolne");
	case T_LT:
		return cmp_direct(n, "boolult", "boollt");
	case T_GT:
		return cmp_direct(n, "boolugt", "boolgt");
	case T_LTEQ:
		return cmp_direct(n, "boolule", "boolle");
	case T_GTEQ:
		return cmp_direct(n, "booluge", "boolge");
	case T_SLASH:
		if (r->op == T_CONSTANT) {
			if (s <= 2 && gen_fast_div(s, r->value, n->type))
				return 1;
		}
		break;
	case T_STAR:
		if (r->op == T_CONSTANT) {
			v = r->value;
			if (s <= 2 && can_fast_mul(s, v)) {
				gen_fast_mul(s, v);
				return 1;
			}
		}
		break;
	}
	return 0;
}

/*
 *	Allow the code generator to shortcut the generation of the argument
 *	of a single argument operator (for example to shortcut constant cases
 *	or simple name loads that can be done better directly)
 */
unsigned gen_uni_direct(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned nr = n->flags & NORETURN;
	unsigned off;

	switch(n->op) {
	case T_LEQ:
		/* We have a specific optimization case that occurs a lot
		   *auto = 0, that we can optimize nicely */
		if (r->op == T_CONSTANT && r->value == 0 && nr) {
			if (cpu_is_09) {
				off = n->val2 + sp;
				while(s--)
					printf("\tclr [%u,s]\n", off++);
				return 1;
			}
			/* Offset of pointer in local */
			/* val2 is the local offset, value the data offset */
			off = make_local_ptr(n->value, 256 - s);
			/* off,X is now the pointer */
			printf("\tldx %u,x\n", off);
			invalidate_x();
			uniop_on_ptr("clr", n->val2, s);
			return 1;
		}
		return 0;
	case T_LBEQ:
		if (r->op == T_CONSTANT && r->value == 0 && nr) {
			if (s == 1 && nr) {
				printf("\tclr [T%u+%u]\n", n->val2, (unsigned)n->value);
				return 1;
			}
		}
		return 0;
	case T_NEQ:
		if (r->op == T_CONSTANT && r->value == 0 && nr) {
			if (s == 1 && nr) {
				printf("\tclr [_%s+%u]\n", namestr(n->snum), (unsigned)n->value);
				return 1;
			}
		}
		return 0;
	/* Writes of 0 to an object we can use clr for providing the
	   result is not then re-used */
	case T_LSTORE:
	case T_LBSTORE:
	case T_NSTORE:
		/* Optimizations for constants */
		if (nr && r->op == T_CONSTANT && r->value == 0) {
			if (write_uni_op(n, "clr", 0)) {
				invalidate_d();
				return 1;
			}
		}
		return 0;
	}
	return 0;
}

/*
 *	Try and build an op where we load X with the pointer,
 *	AB with the data and call a helper. Some of these may also
 *	benefit from inline forms later. 32bit also works as the
 *	value ends up in @hireg|Y/AB which is all safe from the load of
 *	the X pointer.
 */

unsigned do_xptrop(struct node *n, const char *op, unsigned off)
{
	unsigned size;
	switch(n->op) {
	case T_ANDEQ:
		op_on_ptr(n, "and", off);
		break;
	case T_OREQ:
		op_on_ptr(n, "or", off);
		break;
	case T_HATEQ:
		op_on_ptr(n, "eor", off);
		break;
	case T_PLUSEQ:
		opd_on_ptr(n, "add", "adc", off);
		break;
	case T_MINUSEQ:
		/* We want to subtract D *from* .X. Negate D and add
		   as this seems the cheapest approach */
		size = get_size(n->type);
		if (size == 1) {
			puts("\tnegb");
			opd_on_ptr(n, "add", "adc", off);
			break;
		}
		if (size == 2) {
			puts("\tcoma\n\tcomb");
			modify_a(~a_val);
			modify_b(~b_val);
			add_d_const(1);
			opd_on_ptr(n, "add", "adc", off);
			break;
		}
		helper(n, "negate");
		opd_on_ptr(n, "add", "adc", off);
		break;
	case T_STAREQ:
		/* If we have D we have mul */
		size = get_size(n->type);
		if (size == 1 && cpu_has_d) {
			printf("\tlda %u,x\n\tmul\n\tstb %u,x\n", off, off);
			return 1;
		}
		/* Fall through */
	/* TODO:
	   MINUSEQ
	   PLUSPLUS
	   MINUSMINUS
	   shift prob not */
	default:
		/* For the cases we can't inline we may have to adjust X because
		   we can't propogate the offset nicely into the helper. On 6809 this
		   is easy, on 6800 it's ugly and we should have a version of the helper
		   that adds a const then falls into the main implementation */
		if (off) {
			if (cpu_has_lea)
				printf("\tleax %u,x\n", off);
			else  if (cpu_has_xgdx)
				printf("\txgdx\n\taddd #%u\n\txgdx\n", off);
			else {
				if (off > 0 && off <= 5 + opt)
					repeated_op(off, "inx");
				else if (off < 0 && off >= -5 - opt)
					repeated_op(-off, "dex");
				else {
					printf("\tjsr __addxconst\n\t.word %u\n", off);
				}
			}
			invalidate_x();
		}
		helper_s(n, op);
		return 1;
	}
	/* Stuff D back into ,X */
	opd_on_ptr(n, "st", "st", off);
	return 1;
}

static unsigned deref_op(unsigned op)
{
	switch(op) {
	case T_NAME:
		return T_NREF;
	case T_LOCAL:
		return T_LREF;
	case T_LABEL:
		return T_LBREF;
	}
	return 0;
}

unsigned write_xsimple(struct node *n, unsigned via_ptr)
{
	const char *op, *op2;
	unsigned off = 0;
	unsigned op16 = 0;
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned top = n->op;

	switch(n->op) {
	case T_ANDEQ:
		op = op2 = "and";
		break;
	case T_OREQ:
		op = op2 = "or";
		break;
	case T_HATEQ:
		op = op2 = "eor";
		break;
	case T_PLUSEQ:
		op = "add";
		op2 = "adc";
		op16 = 1;
		break;
	case T_MINUSEQ:
		op = "sub";
		op2 = "sbc";
		op16 = 1;
		break;
	default:
		return 0;
	}
	top = deref_op(l->op);
	if (via_ptr || top == 0) {
		off = load_x_with(l, 0);
		opd_on_ptr(n, "ld", "ld", off);
		via_ptr = 1;
	} else {
		/* Not sure we should encourage this kind of behaviour ;) */
		/* TODO: turn this simple/simple stuff into a tree rewrite */
		l->op = top;
		l->type = r->type;
		write_opd(l, "ld", "ld", off);
	}
	if (op16)
		write_opd(r, op, op2, 0);
	else
		write_op(r, op, op2, 0);

	if (via_ptr)
		opd_on_ptr(n, "st", "st", off);
	else
		write_opd(l, "st", "st", off);
	set_d_node(l);
	return 1;
}

/* TODO: 6809 could index locals via S so if n->left is LOCAL or
   ARGUMENT we can special case it */
unsigned do_xeqop(struct node *n, const char *op)
{
	unsigned off;
	struct node *l = n->left;
	struct node *r = n->right;
	/* Handle simpler cases of -= the other way around */
	if (is_simple(r) && get_size(n->type) <= 2) {
		if (is_simple(l)) {
			if (write_xsimple(n, 0))
				return 1;
		}
		else if (can_load_r_with(l, 0)) {
			if (write_xsimple(n, 1))
				return 1;
		}
	}
	if (!can_load_r_with(n->left, 0)) {
		printf(";can't load x %u\n", n->left->op);
		/* Compute the left side and stack it */
		codegen_lr(n->left);
		gen_push(n->left);
		/* Get D right, then pull the pointer into X */
		codegen_lr(n->right);
		pop_x();
		sp -= 2;	/* Pulling the pointer back off */
		off = 0;
		/* and drop into he helper */
	} else {
		/* Get the value part into AB */
		codegen_lr(n->right);
		/* Load X (lval of the eq op) up (doesn't disturb AB) */
		off = load_x_with(n->left, 0);
	}
	/* Things we can then inline */
	if (do_xptrop(n, op, off) == 0)
		error("xptrop");
	set_d_node_ptr(n->left);
	return 1;
}

unsigned do_stkeqop(struct node *n, const char *op)
{
	if (cpu_is_09)
		puts("\tpuls x");
	else if (cpu_has_pshx)
		puts("\tpulx");
	else	/** Fun fun. Fake pulx on a 6800 */
		puts("\ttsx\n\tldx ,x\n\tins\n\tins");
	invalidate_x();
	return do_xptrop(n, op, 0);
}

/*
 *	Things we can try and do as a direct memory op for bytes
 */

unsigned memop_const(struct node *n, const char *op)
{
	char buf[32];
	unsigned v;
	struct node *l = n->left;
	struct node *r = n->right;
	v = l->value;
	if (r->op != T_CONSTANT)
		return 0;
	/* The helper has to load x and a value and make a call
	   so is quite expensive */
	if (r->value > 5 && opt < 2)
		return 0;
	switch(l->op) {
	case T_LABEL:
		sprintf(buf, "%s T%u+%u", op, l->val2, v);
		repeated_op(r->value, buf);
		invalidate_mem();
		return 1;
	case T_NAME:
		sprintf(buf, "%s _%s+%u", op, namestr(l->snum), v);
		repeated_op(r->value, buf);
		invalidate_mem();
		return 1;
	/* No ,x forms so cannot do locals */
	}
	return 0;
}

unsigned memop_shift(struct node *n, const char *op, const char *opu)
{
	char buf[32];
	unsigned v;
	unsigned rv;
	struct node *l = n->left;
	struct node *r = n->right;
	v = l->value;
	rv = r->value;
	if (r->op != T_CONSTANT)
		return 0;

	/* Right shifts are sign specific */
	if (n->type & UNSIGNED)
		op = opu;
	if (rv > 7) {
		/* Undefined but do something nice */
		rv = 1;
		op = "clr";
	}
	if (rv > 2 && opt < 2)
		return 0;
	switch(l->op) {
	case T_LABEL:
		sprintf(buf, "%s T%u+%u", op, l->val2, v);
		repeated_op(r->value, buf);
		invalidate_mem();
		return 1;
	case T_NAME:
		sprintf(buf, "%s _%s+%u", op, namestr(l->snum), v);
		repeated_op(r->value, buf);
		invalidate_mem();
		return 1;
	case T_LOCAL:
		/* No ,x forms so cannot do locals */
		if (!cpu_is_09)
			return 0;
		sprintf(buf, "%s %u,s", op, v + sp);
		repeated_op(r->value, buf);
		invalidate_mem();
		return 1;
	}
	return 0;
}

/* Generate inline code for ++ and -- operators when it makes sense */
unsigned add_to_node(struct node *n, int sign, int retres)
{
	struct node *r = n->right;
	struct node *l = n->left;
	unsigned v = l->value;
	unsigned s = get_size(n->type);
	unsigned off;

	if (s > 2 || r->op != T_CONSTANT)
		return 0;
	if (cpu_is_09) {
		if (l->op == T_ARGUMENT || l->op == T_LOCAL) {
			if (l->op == T_ARGUMENT)
				v += argbase + frame_len;
			v += sp;
			load_d_const(sign * r->value);
			printf("\taddd %u,s\n\tstd %u,s\n", v, v);
			invalidate_work();
			if (retres)
				set_d_node_ptr(r);
			else
				add_d_const(-sign * r->value);
			return 1;
		}
	}
	/* It's marginal whether we should go via X or not */
	if (!can_load_r_with(l, 0))
		return 0;
	off = load_x_with(l, 0);
	if (s == 1) {
		op8_on_ptr("ld", off);
		if (!retres) {
			if (cpu_is_09)
				puts("\tpshs b");
			else
				puts("\tpshb");
		}
		add_b_const(sign * r->value);
		op8_on_ptr("st", off);
		if (!retres) {
			if (cpu_is_09)
				puts("\tpuls b");
			else
				puts("\tpulb");
		}
		invalidate_work();
		invalidate_mem();
		set_d_node(l);
		return 1;
	}
	op16d_on_ptr("ld", "ld", off);
	if (!retres) {
		if (cpu_is_09)
			puts("\tpshs d");
		else
			puts("\tpshb\n\tpsha");
	}
	add_d_const(sign * r->value);
	op16d_on_ptr("st", "st", off);
	if (!retres) {
		if (cpu_is_09)
			puts("\tpuls d");
		else
			puts("\tpula\n\tpulb");
	}
	invalidate_work();
	invalidate_mem();
	set_d_node_ptr(l);
	return 1;
}

static void gen_lplus(struct node *n, const char *post)
{
	/* Expression on the right plus sp plus stack ptr plus n->value */
	unsigned v = n->value + sp;
	if (v < 256)
		printf("\tjsr __lplus%sb\n\t.byte %u\n", post, v);
	else
		printf("\tjsr __lplus%s\n\t.word %u\n", post, v);
}

/*
 *	Allow the code generator to shortcut trees it knows
 */
unsigned gen_shortcut(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned s = get_size(n->type);
	unsigned nr = n->flags & NORETURN;
	unsigned v;

	/* Don't generate unreachable code */
	if (unreachable)
		return 1;
	/* Handle operations that are of the form (OP (REG) (thing)) as we can't really
	   talk about 'address' of a register variable for 6809 */
	if (l && l->op == T_REG && cpu_is_09) {
		v = r->value;
		switch(n->op) {
		case T_PLUSPLUS:
			if (!nr)
				puts("\ttfr u,d");
			printf("\tleau %u,u\n", v);
			return 1;
		case T_PLUSEQ:
			if (r->op == T_CONSTANT)
				printf("\tleau %u,u\n", v);
			else {
				codegen_lr(r);
				puts("\tleau d,u");
			}
			if (!nr)
				puts("\ttfr u,d");
			return 1;
		case T_MINUSMINUS:
			if (!nr)
				printf("\ttfr u,d\n");
			printf("\tleau -%u,u\n", v);
			return 1;
		case T_MINUSEQ:
			if (r->op == T_CONSTANT)
				printf("\tleau -%u,u\n", v);
			else {
				codegen_lr(r);
				puts("\tleau d,u");
			}
			if (!nr)
				puts("\ttfr u,d");
			return 1;
		case T_STAREQ:
			/* TODO: fast mul forms */
			codegen_lr(r);
			helper(n, "regmul");
			return 1;
		case T_SLASHEQ:
			/* TODO: short forms */
			codegen_lr(r);
			helper_s(n, "regdiv");
			return 1;
		case T_PERCENTEQ:
			codegen_lr(r);
			helper_s(n, "regmod");
			return 1;
		case T_SHLEQ:
			codegen_lr(r);
			helper(n, "reglsl");
			return 1;
		case T_SHREQ:
			codegen_lr(r);
			helper(n, "regshr");
			return 1;
		/* These don't trash other regs about */
		case T_ANDEQ:
			codegen_lr(r);
			printf("\t%s __regand", jsr_op);
			invalidate_d();
			return 1;
		case T_OREQ:
			codegen_lr(r);
			printf("\t%s __regor", jsr_op);
			invalidate_d();
			return 1;
		case T_HATEQ:
			codegen_lr(r);
			printf("\t%s __regxor", jsr_op);
			invalidate_d();
			return 1;
		}
	}

	switch(n->op) {
	case T_COMMA:
		/* The comma operator discards the result of the left side,
		   then evaluates the right. Avoid pushing/popping and
		   generating stuff that is surplus */
		l->flags |= NORETURN;
		codegen_lr(l);
		/* Parent determines child node requirements */
		codegen_lr(r);
		r->flags |= nr;
		return 1;
	case T_DEREF:
	case T_DEREFPLUS:
		/* Our right hand side is the thing to deref. See if we can
		   get it into X instead */
		printf(";deref r %x %u\n", r->op, (unsigned)r->value);
		v = n->value;
		/* Need to look at off handling
		   TODO: think v + off is all that is needed but for non
		   6809 that means range checking complications */
		if (can_load_r_simple(r, 0)) {
			printf("; go via shortcut\n");
			v += load_x_with(r, 0);
			invalidate_work();
			if (s == 4)
				load32(v);
			else
				opd_on_ptr(n, "ld", "ld", v);
			return 1;
		}
		return 0;
	case T_EQ:	/* Our left is the address */
	case T_EQPLUS:
		v = n->value;
		if (can_load_r_simple(l, 0)) {
			if (r->op == T_CONSTANT && nr && r->value == 0) {
				/* We can optimize thing = 0 for the case
				   we don't also need the value */
				v += load_x_with(l, 0);
				uniop_on_ptr("clr", v, s);
				return 1;
			}
			codegen_lr(r);
			v += load_x_with(l, 0);
			invalidate_mem();
			if (s == 4)
				store32(v, nr);
			else
				opd_on_ptr(n, "st", "st", v);
			return 1;
		}
		if (can_load_d_nox(r, 0)) {
			if (l->op == T_LPLUS) {
				/* Do the LPLUS expression by hand */
				codegen_lr(l->right);
				/* Use the helper to put it in X */
				gen_lplus(l, "x");
			} else {
				codegen_lr(l);
				make_x_d();
			}
			/* X is now our pointer */

			/* Load D without touching X */
			if (r->op == T_CONSTANT) {
				if (r->value == 0 && nr) {	/* Special case */
					uniop_on_ptr("clr", 0, s);
					return 1;
				} else {
					codegen_lr(r);
				}
			} else
				write_op(r, "ld", "ld", 0);
			invalidate_work();

			/* Store */
			invalidate_mem();
			if (s == 4)
				store32(v, nr);
			else
				opd_on_ptr(n, "st", "st", v);
			return 1;
		}
		return 0;
	case T_PLUSEQ:
		if (s == 1 && nr && memop_const(n, "inc"))
			return 1;
		if (s == 2 && add_to_node(n, 1, 1))
			return 1;
		return do_xeqop(n, "xpluseq");
	case T_MINUSEQ:
		if (s == 1 && nr && memop_const(n, "dec"))
			return 1;
		if (s == 2 && add_to_node(n, -1, 1))
			return 1;
		return do_xeqop(n, "xminuseq");
	case T_PLUSPLUS:
		if (s == 1 && nr && memop_const(n, "inc"))
			return 1;
		if (s == 2 && add_to_node(n, 1, nr))
			return 1;
		return do_xeqop(n, "xplusplus");
	case T_MINUSMINUS:
		if (s == 1 && nr && memop_const(n, "dec"))
			return 1;
		if (s == 2 && add_to_node(n, -1, nr))
			return 1;
		return do_xeqop(n, "xmminus");
	case T_STAREQ:
		return do_xeqop(n, "xmuleq");
	case T_SLASHEQ:
		return do_xeqop(n, "xdiveq");
	case T_PERCENTEQ:
		return do_xeqop(n, "xremeq");
	case T_SHLEQ:
		if (s == 1 && nr && memop_shift(n, "lsl", "lsl"))
			return 1;
		return do_xeqop(n, "xshleq");
	case T_SHREQ:
		if (s == 1 && nr && memop_shift(n, "asr", "lsr"))
			return 1;
		return do_xeqop(n, "xshreq");
	case T_ANDEQ:
		return do_xeqop(n, "xandeq");
	case T_OREQ:
		return do_xeqop(n, "xoreq");
	case T_HATEQ:
		return do_xeqop(n, "xhateq");
#if 0
	/* This won't work as is and requires a rethink	*/
	/* Thankfully it's only the expression/expression case */
	case T_MINUS:
		/* Do it backwards if not a const and on a 6809 */
		if (r->op == T_CONSTANT || s > 2 || cpu_is_09 == 0)
			return 0;
		codegen_lr(r);
		/* Try and write it as
			ldd foo  subd 1,s or similar */
		if (write_opd(l, "sub", "sbc", 0))
			return 1;
		/* Ok then we have to write it the long way */
		gen_push(r);
		codegen_lr(l);
		if (s == 1)
			op8_on_spi("sub");
		else
			op16d_on_spi("sub");
		/* our op takes it back off stack */
		sp -= get_stack_size(r->type);
		return 1;
#endif
	case T_RSTORE:
		if (can_load_r_with(r, 0)) {
			load_u_with(r, 0);
			return 1;
		}
		codegen_lr(r);
		puts("\ttfr d,u");
		return 1;
	case T_REQ:
		codegen_lr(r);
		switch(s) {
		case 4:
			printf("\tsty %u,u\n\tstd %u,u\n", n->val2, n->val2 + 2);
			return 1;
		case 2:
			printf("\tstd %u,u\n", n->val2);
			return 1;
		case 1:
			printf("\tstb %u,u\n", n->val2);
			return 1;
		}
		break;
	case T_REQPLUS:
		codegen_lr(r);
		switch(s) {
		case 4:
			puts("\tsty ,u++");
			/* Fall through */
		case 2:
			puts("\tstd ,u++");
			break;
		case 1:
			puts("\tstb ,u+");
			break;
		}
		return 1;
	}
	return 0;
}

static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned ls;
	unsigned rs;

	if (PTR(rt))
		rt = USHORT;
	if (PTR(lt))
		lt = USHORT;

	/* Floats and stuff handled by helper */
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		return 0;

	ls = get_size(lt);
	rs = get_size(rt);

	/* Size shrink is free */
	if (ls <= rs)
		return 1;
	/* Don't do the harder ones */
	if (!(rt & UNSIGNED)) {
		if (cpu_is_09) {
			if (rs == 1 && ls == 2) {
				puts("\tsex");
				return 1;
			}
			/* TODO: 6309 has sexw */
		}

		if (cpu_has_y) {
			if (opt && rs == 1 && ls == 2) {
				puts("\tclra\n\tasrb\n\trolb\n\tsbca #0");
				return 1;
			}
			/* TODO 2->4 byte expansion with Y register */
			return 0;
		}

		if (opt && rs == 1) {
			puts("\tclra\n\tasrb\n\trolb\n\tsbca #0");
			if(ls == 4)
				puts("\tstaa @hireg\n\tstaa @hireg+1");
			return 1;
		}
		if (opt > 2 && ls == 4 && rs == 2) {
			puts("\tpshb\n\tclrb\n\tasra\n\trola\n\tsbcb #0\n\tstab @hireg\n\tstab @hireg+1\n\tpulb");
			return 1;
		}
		return 0;
	}
	if (rs == 1)
		load_a_const(0);
	if (ls == 4) {
		if (cpu_has_y)
			puts("\tldy #0");
		else
			puts("\tclr @hireg\n\tclr @hireg+1");
	}
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned nr = n->flags & NORETURN;
	unsigned v;
	unsigned off;

	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_stack_size(n->left->type);
	v = n->value;
	switch (n->op) {
	case T_CALLNAME:
		invalidate_all();
		printf("\t%s _%s+%u\n", jsr_op, namestr(n->snum), v);
		return 1;
	case T_DEREF:
	case T_DEREFPLUS:
		printf(";deref %u\n", v);
		make_x_d();
		if (s == 1) {
			op8_on_ptr("ld", v);
			invalidate_a();
			return 1;
		}
		if (s == 2) {
			op16d_on_ptr("ld", "ld", v);
			set_d_node(n);
			return 1;
		}
		if (s == 4 && cpu_has_y) {
			printf("\tldy %u,x\n\tldd %u,x\n", v, v + 2);
			set_d_node(n);	/* TODO: review 32 bit cases */
			return 1;
		}
		load32(v);
		return 1;
	case T_EQ:	/* Assign - ToS is address, working value is value */
	case T_EQPLUS:
		invalidate_mem();
		if (s == 1) {
			pop_x();
			op8_on_ptr("st", v);
			return 1;
		}
		if (s == 2) {
			pop_x();
			op16d_on_ptr("st", "st", v);
			return 1;
		}
		if (s == 4) {
			pop_x();
			op32d_on_ptr("st", "st", v);
			return 1;
		}
		break;
	case T_CONSTANT:
		if (s == 1) {
			load_b_const(v);
			return 1;
		}
		if (s == 2) {
			load_d_const(v);
			return 1;
		}
		/* size 4 varies */
		if (cpu_has_y) {
			load_d_const(v);
			if (cpu_is_09 && (n->value >> 16) == (v & 0xFFFF))
				puts("\ttfr d,y");
			else
				/* TODO: tracking on Y ? */
				printf("\tldy #%u\n", (unsigned)((n->value >> 16) & 0xFFFF));
			return 1;
		}
		load_d_const(n->value >> 16);
		if (cpu_has_d)
			puts("\tstd @hireg");
		else
			puts("\tstaa @hireg\n\tstab @hireg+1");
		load_d_const(n->value);
		return 1;
	case T_LABEL:
	case T_NAME:
	case T_LREF:
	case T_NREF:
	case T_LBREF:
		if (d_holds_node(n))
			return 1;
		if (write_opd(n, "ld", "ld", 0)) {
			set_d_node(n);
			return 1;
		}
		if (s == 4 && cpu_has_y) {
			op16y_on_node(n, "ld", 0);
			op16d_on_node(n, "ld", "ld",  2);
			invalidate_work();
			return 1;
		}
		/* TODO: 6800/3 cases for dword */
		return 0;
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
		if (write_opd(n, "st", "st", 0)) {
			invalidate_mem();
			set_d_node(n);
			return 1;
		}
		if (s == 4 && cpu_has_y) {
			op16y_on_node(n, "st", 0);
			op16d_on_node(n, "st", "st", 2);
			return 1;
		}
		break;
	case T_RREF:
		if (d_holds_node(n))
			return 1;
		puts("\ttfr u,d");
		set_d_node(n);
		return 1;
	case T_RSTORE:
		puts("\ttfr d,u");
		set_d_node(n);
		return 1;
	case T_ARGUMENT:
		if (d_holds_node(n))
			return 1;
		v += argbase + frame_len;
	case T_LOCAL:
		/* For v != 0 case it would be more efficient to load
		   const then add @tmp/tmp+1 TODO */
		if (d_holds_node(n))
			return 1;
		if (cpu_has_xgdx) {
			make_local_ptr(v, 0);
			/* X is now the value we need */
			swap_d_x();
			return 1;
		}
		/* The stack offsetting on the non 6809 processors is
		   different. TSX/TXS add and subtract one but the actual
		   S register points one byte off from a 6809 so compensate */
		if (!cpu_is_09) {
			v++;
			if (optsize && !cpu_has_d) {
				/* Try and optimise 6800 cases */
				if (v < 256)
					printf("\tjsr __local_b\n\t.byte %u\n", v);
				else
					printf("\tjsr __local_d\n\t.word %u\n", v);
				return 1;
			}
		}
		move_s_d();
		v += sp;
		add_d_const(v);
		set_d_node(n);
		return 1;
	case T_LDEREF:
		if (cpu_is_09) {
			if (n->val2 == 0 && s <= 2) {
				if (s == 1)
					printf("\tldb [%u,s]\n", v + sp);
				else
					printf("\tldd [%u,s]\n", v + sp);
				invalidate_work();
				return 1;
			} else {
				printf("\tldx %u,s\n", v + sp);
			}
		} else {
			/* Offset of pointer in local */
			/* val2 is the local offset, value the data offset */
			off = make_local_ptr(v, 256 - s);
			/* off,X is now the pointer */
			printf("\tldx %u,x\n", off);
		}
		invalidate_x();
		v = n->val2;
		if (s == 1)
			op8_on_ptr("ld", v);
		else if (s == 2)
			op16d_on_ptr("ld", "ld", v);
		else
			op32d_on_ptr("ld", "ld", v);
		invalidate_work();
		return 1;
	case T_NDEREF:
		/* Deref through a name. Only generated for the 6809
		   and only when the object offset is 0 and size is 1 or 2 */
		if (s == 1)
			printf("\tldb [_%s + %u]\n", namestr(n->snum), v);
		else
			printf("\tldd [_%s + %u]\n", namestr(n->snum), v);
		invalidate_work();
		return 1;
	case T_LBDEREF:
		/* Deref through a label. Only generated for the 6809
		   and only when the object offset is 0 and size is 1 or 2 */
		if (s == 1)
			printf("\tldb [T%u + %u]\n", n->val2, v);
		else
			printf("\tldd [T%u + %u]\n", n->val2, v);
		invalidate_work();
		return 1;
	case T_RDEREF:
		if (s == 4)
			printf("\tldy %u,u\n\tldd %u,u\n", n->val2, n->val2 + 2);
		else if (s == 2)
			printf("\tldd %u,u\n", n->val2);
		else
			printf("\tldb %u,u\n", n->val2);
		invalidate_work();
		return 1;
	case T_RDEREFPLUS:
		if (s == 4)
			puts("\tldy ,u++\n\tldd ,u++");
		else if (s == 2)
			puts("\tldd ,u++");
		else
			puts("\tldb ,u+");
		invalidate_work();
		return 1;
	case T_LEQ:
		/* We probably want some indirecting helpers later */
		if (cpu_is_09) {
			if (n->val2 == 0 && s <= 2) {
				if (s == 1)
					printf("\tstb [%u,s]\n", v + sp);
				else
					printf("\tstd [%u,s]\n", v + sp);
				return 1;
			}
			printf("\tldx %u,s\n", v + sp);
		} else {
			/* Offset of pointer in local */
			off = make_local_ptr(v, 256 - s);
			/* off,X is now the pointer */
			printf("\tldx %u,x\n", off);
		}
		invalidate_x();
		if (s == 1)
			op8_on_ptr("st", n->val2);
		else if (s == 2)
			op16d_on_ptr("st", "st", n->val2);
		else
			op32d_on_ptr("st", "st", n->val2);
		return 1;
	case T_NEQ:
		if (s == 1)
			printf("\tstb [_%s + %u]\n", namestr(n->snum), v);
		else
			printf("\tstd [_%s + %u]\n", namestr(n->snum), v);
		return 1;
	case T_LBEQ:
		if (s == 1)
			printf("\tstb [T%u + %u]\n", n->val2, v);
		else
			printf("\tstd [T%u + %u]\n", n->val2, v);
		return 1;
	case T_LPLUS:	/* 6800 and -Os only */
		gen_lplus(n, "");
		invalidate_work();
		return 1;
	/* Type casting */
	case T_CAST:
		if (nr)
			return 1;
		return gen_cast(n);
	/* Single argument ops we can do directly */
	case T_TILDE:
		if (s == 4)
			return 0;
		if (s == 2) {
			puts("\tcoma\n\tcomb");
			modify_a(~a_val);
			modify_b(~b_val);
		} else {
			puts("\tcomb");
			modify_b(~b_val);
		}
		return 1;
	case T_NEGATE:
		if (s == 2) {
			puts("\tcoma\n\tcomb");
			modify_a(~a_val);
			modify_b(~b_val);
			add_d_const(1);
			return 1;
		}
		if (s == 1) {
			puts("\tnegb");
			modify_b(-b_val);
			return 1;
		}
		return 0;
	/* Double argument ops we can handle easily */
	case T_PLUS:
		if (n->type == FLOAT)
			return 0;
		/* This is a special case as we need to order the
		   operations */
		if ((cpu_has_d && s <= 2) || cpu_is_09)
			return write_tos_opd(n, "add", "adc");
		/* For 6800 punt to helper */
		return 0;
	case T_AND:
		return write_tos_op(n, "and");
	case T_OR:
		return write_tos_op(n, "or");
	case T_HAT:
		return write_tos_op(n, "eor");
	/* These do the maths backwards in effect so use the other equvialent
	   compare for the ordered part */
	case T_EQEQ:
		return cmp_op(n, "booleq", "booleq");
	case T_BANGEQ:
		return cmp_op(n, "boolne", "boolne");
	case T_LT:
		return cmp_op(n, "boolugt", "boolgt");
	case T_GT:
		return cmp_op(n, "boolult", "boollt");
	case T_LTEQ:
		return cmp_op(n, "booluge", "boolge");
	case T_GTEQ:
		return cmp_op(n, "boolule", "boolle");
	/* EQ ops via top of stack. Simpler ones alreadu got handled
	   by direct or shortcut. For a complex one we try and inline it
	   by pulling X, but if we can't then we will use a helper between
	   X and D (or Y:D for long) */
	case T_STAREQ:
		return do_xeqop(n, "xmuleq");
	case T_SLASHEQ:
		return do_xeqop(n, "xdiveq");
	case T_PERCENTEQ:
		return do_xeqop(n, "xremeq");
	case T_SHLEQ:
		if (s == 1 && nr && memop_shift(n, "lsl", "lsl"))
			return 1;
		return do_xeqop(n, "xshleq");
	case T_SHREQ:
		if (s == 1 && nr && memop_shift(n, "asr", "lsr"))
			return 1;
		return do_xeqop(n, "xshreq");
	case T_ANDEQ:
		return do_stkeqop(n, "xandeq");
	case T_OREQ:
		return do_stkeqop(n, "xoreq");
	case T_HATEQ:
		return do_stkeqop(n, "xhateq");
	case T_PLUSEQ:
		return do_stkeqop(n, "xpluseq");
	case T_MINUSEQ:
		return do_stkeqop(n, "xminuseq");
	/* Function calls that were not to a constant name */
	case T_FUNCCALL:
		if (cpu_has_xgdx) {
			make_x_d();
			puts("\tjsr ,x");
		} else
			puts("\tstaa @tmp\n\tstab @tmp+1\n\tjsr @tmp\n");
		invalidate_all();
		return 1;
	}
	return 0;
}

/* TODO
	WIP Track X v S offset
	Conditions (EQEQ BANGEQ LT GT LTEQ GTEQ) - optimize < 0 and
	other easy ones ?
	BANG
	BOOL
	STAR SLASH PERCENT (const optimizations)
	WIP Track register values
	Track condition codes
	Think about how to improve long handling
	Arg helpers like Z8 etc so we can optimize post 6800 a bit
	and also optimize push const cases and long consts especially
	(and probably push arg)
	Optimize arg pushes if have pshx by using LDX PSHX for consts
	Add all the general operations we can do 16bit with a 6803
*/
