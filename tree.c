/*
 *	Tree operations to build a node tree
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "compiler.h"

#define NUM_NODES 100

static struct node node_table[NUM_NODES];
static struct node *nodes;

struct node *new_node(void)
{
	struct node *n;
	if (nodes == NULL) {
		error("too complex");
		exit(1);
	}
	n = nodes;
	nodes = n->right;
	n->left = n->right = NULL;
	n->value = 0;
	n->flags = 0;
	n->type = 0;
	n->sym = NULL;
	return n;
}

void free_node(struct node *n)
{
	n->right = nodes;
	nodes = n;
}

void init_nodes(void)
{
	int i;
	struct node *n = node_table;
	for (i = 0; i < NUM_NODES; i++)
		free_node(n++);
}


struct node *tree(unsigned op, struct node *l, struct node *r)
{
	struct node *n = new_node();
	fprintf(stderr, "tree %04x [", op);
	if (l)
		fprintf(stderr, "%04x ", l->op);
	if (r)
		fprintf(stderr, "%04x ", r->op);
	fprintf(stderr, "]\n");
	n->left = l;
	n->right = r;
	n->op = op;
	/* Default inherit from right */
	if (r)
		n->type = r->type;
	return n;
}

/* Will need type info.. */
struct node *make_constant(unsigned value)
{
	struct node *n = new_node();
	n->op = T_UINTVAL;
	n->value = value;
	n->type = 0; /* FIXME */
	fprintf(stderr, "const %04x\n", value);
	return n;
}

struct node *make_symbol(struct symbol *s)
{
	struct node *n = new_node();
	n->op = s->name;
	n->value = s->offset;
	n->sym = s;
	n->flags = LVAL;
	n->type = s->type;
	/* Rewrite implicit pointer forms */
	if (!PTR(s->type)) {
		if (IS_FUNCTION(s->type) || IS_ARRAY(s->type))
			n->type++;
	}
	if (s->storage == S_AUTO)
		n->flags |= NAMEAUTO;
	if (s->storage == S_ARGUMENT)
		n->flags |= NAMEARG;
	fprintf(stderr, "name %04x\n", s->name - 0x8000);
	return n;
}

struct node *make_label(unsigned label)
{
	struct node *n = new_node();
	n->op = T_LABEL;
	n->value = label;
	n->flags = 0;
	fprintf(stderr, "label %04x\n", label);
	return n;
}

unsigned is_constant(struct node *n)
{
	return (n->op >= T_INTVAL && n->op <= T_ULONGVAL) ? 1 : 0;
}

unsigned is_constant_zero(struct node *n)
{
	if (is_constant(n))
		return !n->value;
	return 0;
}


#define IS_NAME(x)		((x) >= 0x8000)

static void nameref(struct node *n)
{
	if (is_constant(n->right) && IS_NAME(n->left->op)) {
		unsigned value = n->left->value + n->right->value;
		struct node *l = n->left;
		free_node(n->right);
		*n = *n->right;
		n->value = value;
		n->left = NULL;
		n->right = NULL;
		free_node(l);
	}
}

void canonicalize(struct node *n)
{
	if (n->left)
		canonicalize(n->left);
	if (n->right)
		canonicalize(n->right);
	/* Get constants onto the right hand side */
	if (n->left && is_constant(n->left)) {
		struct node *t = n->left;
		n->left = n->right;
		n->right = t;
	}
	/* Propogate NOEFF side effect checking. If our children have a side
	   effect then we don't even if we were safe ourselves */
	if (!(n->left->flags & NOEFF) || !(n->right->flags & NOEFF))
		n->flags &= -~NOEFF;

	/* Turn name + constant into name offsets */
	if (n->op == T_PLUS)
		nameref(n);

	/* Target rewrites - eg turning DEREF of local + const into a name with
	   offset when supported or switching around subtrace trees that are NOEFF */
	//target_canonicalize(n);
}

struct node *make_rval(struct node *n)
{
	if (n->flags & LVAL)
		return tree(T_DEREF, NULL, n);
	return n;
}

struct node *make_noreturn(struct node *n)
{
	n->flags |= NORETURN;
	return n;
}

struct node *make_cast(struct node *n, unsigned t)
{
	n = tree(T_CAST, NULL, n);
	n->type = t;
	return n;
}

void free_tree(struct node *n)
{
	if (n->left)
		free_tree(n->left);
	if (n->right)
		free_tree(n->right);
	free_node(n);
}

static void write_subtree(struct node *n)
{
	write(1, n, sizeof(struct node));
	if (n->left)
		write_subtree(n->left);
	if (n->right)
		write_subtree(n->right);
	free_node(n);
}

void write_tree(struct node *n)
{
	write(1, "%^", 2);
	write_subtree(n);
}

void write_null_tree(void)
{
	write_tree(tree(T_NULL, NULL, NULL));
}

/*
 *	Trees with type rules
 */

struct node *arith_promotion_tree(unsigned op, struct node *l,
				  struct node *r)
{
	/* We know both sides are arithmetic */
	unsigned lt = l->type;
	unsigned rt = r->type;
	struct node *n;

	if (PTR(lt))
		lt = UINT;
	if (PTR(rt))
		rt = UINT;

	/* Our types are ordered for a reason */
	/* Does want review versus standard TODO */
	if (r->type > l->type)
		lt = r->type;

	if (l->type != lt)
		l = make_cast(l, lt);
	if (r->type != lt)
		r = make_cast(r, lt);
	n = tree(op, l, r);
	n->type = lt;
	return n;
}

/* Two argument arithmetic including float - multiply and divide, plus
   some subsets of more general operations below */
struct node *arith_tree(unsigned op, struct node *l, struct node *r)
{
	if (!IS_ARITH(l->type) || !IS_ARITH(r->type))
		badtype();
	return arith_promotion_tree(op, l, r);
}

/* Two argument integer or bit pattern
   << >> & | ^ */
struct node *intarith_tree(unsigned op, struct node *l, struct node *r)
{
	unsigned lt = l->type;
	unsigned rt = r->type;
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		badtype();
	/* FIXME: cast r for shifts ? */
	if (op == T_LTLT || op == T_GTGT) {
		struct node *n = tree(op, l, make_cast(r, CINT));
		n->type = l->type;
		return n;
	} else
		return arith_promotion_tree(op, l, r);
}

/* Two argument ordered compare - allows pointers, also assignment
		< > <= >= == != */
struct node *ordercomp_tree(unsigned op, struct node *l, struct node *r)
{
	struct node *n;
	if (type_pointermatch(l, r))
		n = tree(op, l, r);
	else
		n = arith_tree(op, l, r);
	/* But the final logic 0 or 1 is integer */
	n->type = CINT;
	return n;
}

/* && || */
struct node *logic_tree(unsigned op, struct node *l, struct node *r)
{
	unsigned lt = l->type;
	unsigned rt = r->type;
	struct node *n;

	if (!PTR(lt) && !IS_ARITH(lt))
		badtype();
	if (!PTR(rt) && !IS_ARITH(rt))
		badtype();
	n = arith_promotion_tree(op, l, r);
	n->type = CINT;
	return n;
}
