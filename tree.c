/*
 *	Tree operations to build a node tree
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "compiler.h"

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
	struct node *c;
	if (debug) {
		fprintf(debug, "tree %04x [", op);
		if (l)
			fprintf(debug, "%04x ", l->op);
		if (r)
			fprintf(debug, "%04x ", r->op);
		fprintf(debug, "]\n");
	}
	n->left = l;
	n->right = r;
	n->op = op;
	/* Default inherit from right */
	if (r)
		n->type = r->type;
	c = constify(n);
	if (c)
		return c;
	return n;
}

/*  TODO: get rid of T_xxxval forms and rely on ->type field plus
	T_CONSTANT */
struct node *make_constant(unsigned long value, unsigned type)
{
	struct node *n = new_node();
	n->op = T_CONSTANT;
	n->value = value;
	n->type = type;
	return n;
}

struct node *make_symbol(struct symbol *s)
{
	struct node *n = new_node();
	switch(s->storage) {
	case S_AUTO:
		n->op = T_LOCAL;
		break;
	case S_ARGUMENT:
		n->op = T_ARGUMENT;
		break;
	default:
		n->op = T_NAME;
	}
	n->value = s->offset;
	n->snum = s->name;
	n->flags = LVAL;
	n->type = s->type;
	/* Rewrite implicit pointer forms */
#if 0
	if (!PTR(s->type)) {
		if (IS_FUNCTION(s->type) || IS_ARRAY(s->type))
			n->type++;
	}
#endif
	if (debug)
		fprintf(debug, "name %04x type %04x\n", s->name, s->type);
	return n;
}

struct node *make_label(unsigned label)
{
	struct node *n = new_node();
	n->op = T_LABEL;
	n->value = label;
	n->flags = 0;
	/* FIXME: we need a general setting for default char type */
	n->type = PTRTO|UCHAR;
	return n;
}

unsigned is_constant(struct node *n)
{
	return (n->op == T_CONSTANT) ? 1 : 0;
}

/* Constant or name in linker constant form */
unsigned is_constname(struct node *n)
{
	/* The address of a symbol is a link time constant so can go in initializers */
	/* A dereferenced form however is not */
	/* Locals are not a fixed address */
	if (n->op == T_NAME && (n->flags & LVAL))
		return 1;
	/* A label is also fixed by the linker so constant, thus we can fix
	   up stuff like "hello" + 3 */
	if (n->op == T_LABEL)
		return 1;
	return is_constant(n);
}

unsigned is_constant_zero(struct node *n)
{
	if (is_constant(n))
		return !n->value;
	return 0;
}


#define IS_NAME(x)		((x) >= T_NAME && (x) <= T_ARGUMENT)

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
	/* Sign casting is just symbolic */
	if ((t & 0xF0) == (n->type & 0xF0))
		return n;
	/* Pointer casting likewise */
	if (PTR(n->type)) {
		n->type = t;
		return n;
	}
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

	/* C rules, we might want to defer this and be smart though */
	if (lt < CINT)
		lt = CINT;

	if (lt < FLOAT) {
		if((rt | lt) & UNSIGNED)
			lt |= UNSIGNED;
	}

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
	if (op == T_LTLT || op == T_GTGT) {
		struct node *n = tree(op, l, make_cast(r, CINT));
		n->type = l->type;
		return n;
	} else
		return arith_promotion_tree(op, l, r);
}

/* Two argument ordered compare - allows pointers
		< > <= >= == != */
struct node *ordercomp_tree(unsigned op, struct node *l, struct node *r)
{
	struct node *n;
	if (type_pointermatch(l, r))
		n = tree(op, l, r);
	else
		n = arith_tree(op, l, r);
	/* But the final logic 0 or 1 is integer except for assign when it's
	   right side */
	n->type = CINT;
	return n;
}

struct node *assign_tree(struct node *l, struct node *r)
{
	if (l->type == r->type)
		return tree(T_EQ, l, r);
	if (PTR(l->type)) {
		type_pointermatch(l, r);
		return tree(T_EQ, l, r);
	} else if (PTR(r->type))
		typemismatch();
	else if (!IS_ARITH(l->type) || !IS_ARITH(r->type))
		invalidtype();
	return tree(T_EQ, l, make_cast(r, l->type));
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

/* Constant conversion */

/* FIXME: will need to use the right types for n->value etc eventually
   and maybe union a float/double */

/* For now this only supports integer types */
static struct node *replace_constant(struct node *n, unsigned t, unsigned long value)
{
	if (n->left)
		free_node(n->left);
	if (n->right)
		free_node(n->right);
	free_node(n);
	/* Mask the bits by type */
	switch(t & 0xF0) {
	case CCHAR:
		value &= 0xFF;
		break;
	case CINT:
		value &= 0xFFFF;
		break;
	case CLONG:
		value &= 0xFFFFFFFFUL;
		break;
	}
	return make_constant(value, t);
}

struct node *constify(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;

	/* Remove multiply by 1 or 0 */
	if (n->op == T_STAR && r->op == T_CONSTANT) {
		if (r->value == 1) {
			free_node(n);
			free_node(r);
			return l;
		}
		if (r->value == 0) {
			l = make_constant(0, n->type);
			free_tree(n);
			return l;
		}
	}
	/* Divide by 1 */
	if (n->op == T_SLASH && r->op == T_CONSTANT) {
		if (r->value == 1) {
			free_node(n);
			free_tree(r);
			return l;
		}
	}


	if (l) {
		l = constify(l);
		if (l == NULL)
			return NULL;
		n->left = l;
	}
	if (r) {
		r = constify(r);
		if (r == NULL)
			return NULL;
		n->right = r;
	}
	if (l) {
		unsigned lt = l->type;

		/* Lval names are constant but a maths operation on two name lval is not */
		if (l->op == T_NAME || r->op == T_NAME) {
			if (n->op != T_PLUS)
				return NULL;
			/* Special case for NAME + const */
			if (l->op == T_NAME) {
				if (r->op == T_NAME)
					return NULL;
				l->value += r->value;
				free_node(r);
				free_node(n);
				return l;
			}
			r->value += l->value;
			free_node(l);
			free_node(n);
			return r;
		}
		/* Only do constant work with simple types */
		if (!IS_INTARITH(lt) && !PTR(lt))
			return NULL;
		if (l->flags & LVAL)
			return NULL;

		switch(n->op) {
		case T_PLUS:
			n = replace_constant(n, lt, l->value + r->value);
			break;
		case T_MINUS:
			n = replace_constant(n, lt, l->value - r->value);
			break;
		case T_STAR:
			n = replace_constant(n, lt, l->value * r->value);
			break;
		case T_SLASH:
			if (r->value == 0)
				divzero();
			else
				n = replace_constant(n, lt, l->value / r->value);
			break;
		case T_PERCENT:
			if (r->value == 0)
				divzero();
			else
				n = replace_constant(n, lt, l->value % r->value);
			break;
		case T_ANDAND:
			n = replace_constant(n, lt, l->value && r->value);
			break;
		case T_OROR:
			n = replace_constant(n, lt, l->value || r->value);
			break;
		case T_AND:
			n = replace_constant(n, lt, l->value & r->value);
			break;
		case T_OR:
			n = replace_constant(n, lt, l->value | r->value);
			break;
		case T_HAT:
			n = replace_constant(n, lt, l->value ^ r->value);
			break;
		case T_LTLT:
			n = replace_constant(n, lt, l->value << r->value);
			break;
		case T_GTGT:
			if (l->type & UNSIGNED)
				n = replace_constant(n, lt, l->value >> r->value);
			else
				n = replace_constant(n, lt, ((signed long)l->value) >> r->value);
			break;
		default:
			return NULL;
		}
		return n;
	}
	if (r) {
		/* Uni-ops */
		unsigned rt = r->type;
		if (!IS_INTARITH(rt) && !PTR(rt))
			return NULL;
		if (r->flags & LVAL)
			return NULL;

		switch(n->op) {
		case T_NEGATE:
			/* This also cleans up any negative constants that were tokenized
			   as T_NEGATE, T_CONST <n> */
			n = replace_constant(n, rt, -r->value);
			break;
		case T_TILDE:
			n = replace_constant(n, rt, ~r->value);
			break;
		case T_BANG:
			n = replace_constant(n, rt, !r->value);
			break;
		case T_BOOL:
			n = replace_constant(n, rt, !!r->value);
			break;
		case T_CAST:
			/* We are working with integer constant types so this is ok */
			n = replace_constant(n, n->type, r->value);
			break;
		default:
			return NULL;
		}
		return n;
	}
	/* Terminal node.. are we const ?? */
	if (is_constname(n))
		return n;
	return NULL;
}
