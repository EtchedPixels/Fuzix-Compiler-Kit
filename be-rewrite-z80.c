#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"
#include "backend-z80.h"

/* Check if a single bit is set or clear */

int bitcheckb1(uint8_t n)
{
	unsigned m = 1;
	unsigned i;

	for (i = 0; i < 8; i++) {
		if (n == m)
			return i;
		m <<= 1;
	}
	return -1;
}

int bitcheck1(unsigned n, unsigned s)
{
	register unsigned i;
	unsigned m = 1;

	if (s == 1)
		return bitcheckb1(n);
	for (i = 0; i < 16; i++) {
		if (n == m)
			return i;
		m <<= 1;
	}
	return -1;
}

int bitcheck0(unsigned n, unsigned s)
{
	if (s == 1)
		return bitcheckb1((~n) & 0xFF);
	return bitcheck1((~n) & 0xFFFF, 2);
}

unsigned get_size(register unsigned t)
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
	fprintf(stderr, "type %x\n", t);
	error("gs");
	return 0;
}

unsigned get_stack_size(unsigned t)
{
	unsigned n = get_size(t);
	if (n == 1)
		return 2;
	return n;
}

static void squash_node(register struct node *n, struct node *o)
{
	n->value = o->value;
	n->val2 = o->val2;
	n->snum = o->snum;
	free_node(o);
}

static void squash_left(register struct node *n, unsigned op)
{
	struct node *l = n->left;
	n->op = op;
	squash_node(n, l);
	n->left = NULL;
}

static void squash_right(register struct node *n, unsigned op)
{
	struct node *r = n->right;
	n->op = op;
	squash_node(n, r);
	n->right = NULL;
}

/*
 *	Heuristic for guessing what to put on the right.  This is very
 *	processor dependent.  For Z80 we can fetch most global or static
 *	objects but locals are problematic
 */

static unsigned is_simple(struct node *n)
{
	register unsigned op = n->op;

	/* Multi-word objects are never simple */
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;
	/* We can load these directly into a register */
	if (op == T_CONSTANT || op == T_LABEL || op == T_NAME || op == T_REG)
		return 10;
	/* We can load this directly into a register but it may be a byte longer */
	if (op == T_NREF || op == T_LBREF)
		return 9;
	if (op == T_RREF || op == T_RDEREF)
		return 5;
	return 0;
}

/*
 *	Allow aribtrary rewriting before the rewrite_node process is called
 *	bottom up. Lets us do things like working out which tree sections
 *	could be 8bit, or downward propagation of properties
 */
struct node *gen_rewrite(struct node *n)
{
	return n;
}

/* Can we stuff this type into a pointer for deref and assignment */

static int type_compatible(struct node *n, unsigned t)
{
	unsigned sz = get_size(t);
	if (sz == 1)
		return 1;
	if (n->value == 1)		/* BC char only */
		return 0;
	return 1;			/* IX and IY can do all sizes */
}
/*
 *	Our chance to do tree rewriting. We don't do much for the Z80
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process. We also rewrite dereferences with
 *	offsets so we can use ix and iy nicely.
 */
struct node *gen_rewrite_node(register struct node *n)
{
	register struct node *r = n->right;
	register struct node *l = n->left;
	struct node *c;
	register unsigned op = n->op;
	unsigned nt = n->type;
	int val;

	/* Squash some byte comparisons down into byte ops */
/*	if (n->op == T_EQEQ && l->op == T_CAST) {
		printf(";EQEQ %x L %x T %x R %x V %lx\n",
			n->flags,
			l->op, l->right->type,
			r->op, r->value);
	} */
	if (n->op == T_EQEQ && l->op == T_CAST && l->right->type == UCHAR &&
		r->op == T_CONSTANT && r->value <= 0xFF) {
		n->op = T_BYTEEQ;
		n->value = r->value;
		n->right = l->right;
		n->left = NULL;
		free_node(l);	/* Dump the cast */
		free_node(r);
		return n;
	}
	/* Squash some byte comparisons down into byte ops */
	if (n->op == T_BANGEQ && l->op == T_CAST && l->right->type == UCHAR &&
		r->op == T_CONSTANT && r->value <= 0xFF) {
		n->op = T_BYTENE;
		n->value = r->value;
		n->right = l->right;
		n->left = NULL;
		free_node(l);	/* Dump the cast */
		free_node(r);
		return n;
	}

	/* spot the following tree
			T_DEREF
			    T_PLUS
		       T_RREF    T_CONSTANT -128-127-size
	  so we can rewrite EQ/RDEREF off base + offset from pointer within range using ix offset */
	if (op == T_DEREF) {
		if (r->op == T_PLUS) {
			c = r->right;
			if (r->left->op == T_RREF && c->op == T_CONSTANT && type_compatible(r->left, nt)) {
				val = c->value;
				/* For now - depends on size */
				/* IX and IY only ranged, BC char * direct */
				if (val == 0 || (val >= -128 && val < 125 && r->left->value != 1)) {
					n->op = T_RDEREF;
					n->val2 = val;
					n->value = r->left->value;
					n->right = NULL;
					free_tree(r);
					return n;
				}
			}
		} else if (r->op == T_RREF && type_compatible(r, nt)) {
			/* Check - are we ok with BC always ? */
			n->op = T_RDEREF;
			n->val2 = 0;
			n->value = r->value;
			n->right = NULL;
			free_node(r);
			return n;
		}
	}
	if (op == T_EQ) {
		if (l->op == T_PLUS) {
			c = l->right;
			if (l->left->op == T_RREF && c->op == T_CONSTANT) {
				val = c->value;
				/* For now - depends on size */
				/* IX and IY only -  BC char * direct only */
				if (val == 0 || (val >= -128 && val < 125 && l->left->value != 1)) {
					n->op = T_REQ;
					n->val2 = val;
					n->value = l->left->value;
					n->left = NULL;
					free_tree(l);
					return n;
				}
			}
		} else if (l->op == T_RREF) {
			val = l->value;
			/* For now - depends on size */
			/* IX and IY only -  BC char * direct only */
			if (val == 0 || (val >= -128 && val < 125 && l->value != 1)) {
				n->op = T_REQ;
				n->val2 = 0;
				n->value = val;
				n->left = NULL;
				free_node(l);
				return n;
			}
		}
	}
	/* Merge offset to object into a  single direct reference */
	if (op == T_PLUS && r->op == T_CONSTANT &&
		(l->op == T_LOCAL || l->op == T_NAME || l->op == T_LABEL || l->op == T_ARGUMENT)) {
		/* We don't care if the right offset is 16bit or 32 as we've
		   got 16bit pointers */
		l->value += r->value;
		free_node(r);
		free_node(n);
		return l;
	}
	/* Rewrite references into a load operation */
	if (nt == CCHAR || nt == UCHAR || nt == CSHORT || nt == USHORT || PTR(nt)) {
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
	/* Commutive operations. We can swap the sides over on these */
	if (op == T_AND || op == T_OR || op == T_HAT || op == T_STAR || op == T_PLUS) {
/*		printf(";left %d right %d\n", is_simple(n->left), is_simple(n->right)); */
		if (is_simple(n->left) > is_simple(n->right)) {
			n->right = l;
			n->left = r;
		}
	}
	return n;
}
