/*
 *	For targets where it is useful walk a tree and make anything that
 *	can validly be done on a byte so, without breaking the C language
 *	rules
 *
 *	At the end of the pass the following flags are set
 *
 *	BYTEOP		- operation can be done on bytes irrespective of type
 *	BYTETAIL	- operation should return result in the 8bit reg form
 *	BYTEROOT	- operation is the top of a set of ops being done
 *			  byte sized but should return the value as indicated
 *			  by the type.
 *
 *
 *	BYTEABLE	- internal use only
 *
 *	The combinations are
 *
 *	BYTEROOT|BYTEOP	- perform the operation expecting bytes but the
 *			  result should reflect the actual type, including
 *			  any implied casting
 *	BYTEOP		- operation takes bytes and produces bytes
 *	BYTETAIL|BYTEOP	- ditto
 *	BYTETAIL	- operation acts on normal types but result
 *			  should be byte form (no casting needed)
 *
 *	If BYTE_REMAP is defined it will also rewrite the types for
 *	most operations to simplify logic in the CPU specific backend
 *
 *	Note: We assume 8bit bytes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "compiler.h"
#include "backend.h"
#include "backend-byte.h"

/*
 *	Check if both sides of an operation are 8->16bit casts of the same
 *	type. Used to eliminate 16bit comparisons of two 8bit numbers
 */

static unsigned byte_cast(struct node *n)
{
	if (n->op == T_CAST && (n->right->type & ~UNSIGNED) == CCHAR)
		return 1;
	/* Bools are integer types but can be byteified */
	if (n->flags & ISBOOL)
		return 1;
	return 0;
}

/*
 *	As well as casts look for constants. Constants that will match
 *	in the low 8bits and expand correctly in the upper are ok.
 */
static unsigned byte_fold(struct node *n, struct node *c)
{
	unsigned t = n->type;
	unsigned v = c->value;

	if (c->op != T_CONSTANT)
		return 0;
	if ((t & UNSIGNED) || PTR(t)) {
		if (v & 0xFF00)
			return 0;
		c->flags |= BYTEABLE;
		return 1;
	}
	if (IS_INTARITH(t) == 0)
		return 0;

	v &= 0xFF80;
	if (v == 0 || v == 0xFF80) {
		c->flags |= BYTEABLE;
		return 1;
	}
	return 0;
}

static unsigned cast_lr(struct node *n)
{
	unsigned lt = byte_cast(n->left);
	unsigned rt = byte_cast(n->right);

	/* If we can't cast are the constants within our range */
	if (lt == 0)
		lt = byte_fold(n, n->left);
	if (rt == 0)
		rt = byte_fold(n, n->right);

	if (lt == 0 || rt == 0)
		return 0;
	if (lt == rt)
		return 1;
	return 0;
}

/* Returns true if this operation produces a valid lower byte if done on
   a larger sized value. That is if it's subtree can also be done in byte
   mode
   0: Can't change
   1: Can change
   2: Can change - can be root of a byte tree
   3: can be the tail of a byte tree only
 */

static unsigned op_can_byte(register struct node *n)
{
	register unsigned op = n->op;
	register unsigned b = 0;

	if (n->type == CCHAR || n->type == UCHAR)
		b = 1;

	if (!IS_INTORPTR(n->type))	/* Floats etc */
		return 0;

	/* Any casting between pointers and integer types is a candidate
	   on a byte oriented machine */
	if (op == T_CAST) {
		/* If this is a cast to a byte size then we can try and
		   byteify the subtree below it for all cases, otherwise
		   we can just use the low 8bits */
		if (b)
			return BYTEROOT | BYTEABLE;
		return BYTEABLE;
	}
	/* Mathematical operations */
	if (op == T_PLUS || op == T_MINUS || op == T_STAR)
		return BYTEABLE;
	/* Logic operations */
	if (op == T_AND || op == T_OR || op == T_HAT)
		return BYTEABLE;
	/* Left shift (but not right) */
	if (op == T_LTLT)
		return BYTEABLE;
	/* Constants */
	if (op == T_CONSTANT)
		return BYTEABLE | BYTETAIL;
	/* References must be done (logically at least) for the full width because
	   there might be side effects. Backends can be smarter with the BYTETAIL
	   info */
	if (op == T_NAME || op == T_LABEL || op == T_LOCAL || op == T_DEREF)
		return BYTETAIL;
	/* Boolean results : can produce byte results but subtrees unchanged */
	if (op == T_LT || op == T_GT || op == T_LTEQ || op == T_GTEQ) {
		if (cast_lr(n))
			return BYTEABLE | BYTEROOT | BYTETAIL;
		return BYTETAIL;
	}
	if (op == T_EQEQ || op == T_BANGEQ) {
		if (cast_lr(n))
			return BYTEABLE | BYTEROOT | BYTETAIL;
		return BYTETAIL;
	}
	if (op == T_BOOL || op == T_BANG) {
		if (byte_cast(n->right))
			return BYTEABLE | BYTEROOT | BYTETAIL;
		return BYTETAIL;
	}
	return 0;
}

static unsigned op_is_byteright(struct node *n)
{
	register unsigned op = n->op;
	register unsigned b = 0;

	if (n->type == CCHAR || n->type == UCHAR)
		b = 1;

	if (op == T_LTLT || op == T_GTGT)
		return 1;
	if (b && op == T_EQ)
		return 1;
	return 0;
}

/* Mark every node in the tree that we can do bytesize */
static void label_byteable(register struct node *n)
{
	register unsigned r;

	if (n == NULL)
		return;
	r = op_can_byte(n);
	n->flags |= r;
	label_byteable(n->left);
	if (n->right) {
		/* A few operations the one child node can be done in byte
		   form in some cases */
		if (op_is_byteright(n))
			n->right->flags |= BYTEROOT;
		label_byteable(n->right);
	}
}

static unsigned depth;
static unsigned options;

static unsigned byte_convert(register struct node *n)
{
	if (n == NULL)
		return 1;

	/* If this node can be done in byte form then check if the
	   subnodes down to the point the bottom of the tree, or to
	   a point it doesn't matter can */
	if (n->flags & (BYTEABLE | BYTEROOT)) {
		if (!(n->flags & BYTETAIL) || (n->flags & BYTEROOT)) {
			depth++;
			if (!byte_convert(n->left) || !byte_convert(n->right)) {
				depth--;
				return 0;
			}
		}
		/* Can do child notes in byte ops so we can do ourselves this way if
		   we are not merely BYTEROOT */
		if (n->flags & BYTEABLE)
			n->flags |= BYTEOP;
		/* If we walked through a byteroot then the thing we walked through is
		   no longer a BYTEROOT and should just run as a byteop */
		/* BYTEROOT ops must do the action full size on byteop children
		   so preserve n->type in that case */
		if ((options & BTF_RELABEL) && !(n->flags & (BYTEROOT|BYTETAIL)))
			n->type = CCHAR | (n->type & UNSIGNED);
/*        if (depth != 0)
            n->flags &= ~BYTEROOT; */
		/* Recursively mark out the BYTEOP tree */
		return 1;
	}
	/* Our node is not byteable but the result is, stop looking down the tree
	   but permit conversions above us. The obvious example here is something like
	   !x which requires x is fully evaluated but whose result irrespective of the
	   type of x can be properly resolved in byte form */
	if (n->flags & BYTETAIL) {
		n->flags &= ~BYTEROOT;	/* FIXME: this needs thought */
		/* Type is preserved because we do the things normally on the
		   sub ops but 8bit result going up */
		return 1;
	}
	/* Our node isn't byteable */
	return 0;
}

/* This is not very efficient but it's small and our trees are quite
   size limited */
static void byte_walk_subtree(register struct node *n)
{
	if (n == NULL)
		return;
	byte_walk_subtree(n->left);
	byte_walk_subtree(n->right);
	if (n->flags & BYTEROOT) {
		depth = 0;
		byte_convert(n);
	}
}

/* Label up and analyse the tree. At this point any operation that
   can be done using byte operations is labelled as BYTEOP. We do
   not add nodes for transitions so the backend will need to spot
   this if it uses differing registers for 8/16bit */
void byte_label_tree(register struct node *n, unsigned opt)
{
	options = opt;
	label_byteable(n);
	byte_walk_subtree(n);
}
