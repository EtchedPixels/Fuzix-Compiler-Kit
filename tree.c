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
#ifdef DEBUG
	if (debug) {
		fprintf(debug, "tree %04x [", op);
		if (l)
			fprintf(debug, "%04x ", l->op);
		if (r)
			fprintf(debug, "%04x ", r->op);
		fprintf(debug, "]\n");
	}
#endif
	n->left = l;
	n->right = r;
	n->op = op;
	/* Inherit from left if present, right if not */
	if (l)
		n->type = l->type;
	else if (r)
		n->type = r->type;
	c = constify(n);
	if (c)
		return c;
	return n;
}

struct node *sf_tree(unsigned op, struct node *l, struct node *r)
{
	struct node *n = tree(op, l, r);
	/* A dereference is only a side effect if it might be volatile */
	if (op == T_DEREF) {
		if (voltrack)
			n->flags |= SIDEEFFECT;
	} else
		n->flags |= SIDEEFFECT;
	return n;
}

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

	n->value = 0;
	switch(S_STORAGE(s->infonext)) {
	case S_LSTATIC:
		n->op = T_LABEL;
		n->val2 = s->data.offset;
		break;
	case S_AUTO:
		n->op = T_LOCAL;
		n->value = s->data.offset;
		break;
	case S_ARGUMENT:
		n->op = T_ARGUMENT;
		n->value = s->data.offset;
		break;
	default:
		n->op = T_NAME;
	}
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
#ifdef DEBUG
	if (debug)
		fprintf(debug, "name %04x type %04x\n", s->name, s->type);
#endif		
	return n;
}

struct node *make_label(unsigned label)
{
	struct node *n = new_node();
	n->op = T_LABEL;
	n->val2 = label;
	n->value = 0;
	n->flags = 0;
#ifdef TARGET_CHAR_UNSIGNED
	n->type = PTRTO|UCHAR;
#else
	n->type = PTRTO|CHAR;
#endif
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

static unsigned transitive(unsigned op)
{
	if (op == T_AND || op == T_OR || op == T_HAT ||
	    op == T_PLUS || op == T_STAR)
		return 1;
	return 0;
}

struct node *make_rval(struct node *n)
{
	unsigned nt = n->type;
	if (n->flags & LVAL) {
		if (IS_ARRAY(nt)) {
			if (PTR(nt) == array_num_dimensions(nt)) {
				n->flags &= ~LVAL;
				return n;
			}
			n = sf_tree(T_DEREF, NULL, n);
			/* Decay to base type of array */
			if (!PTR(nt))
				n->type = type_canonical(nt);
			return n;
		} else if (IS_FUNCTION(nt) && !PTR(nt)) {
			n->flags &= ~LVAL;
		} else
			return sf_tree(T_DEREF, NULL, n);
	}
	return n;
}

struct node *make_noreturn(struct node *n)
{
	n->flags |= NORETURN;
	return n;
}

struct node *make_cast(struct node *n, unsigned t)
{
	unsigned nt = type_canonical(n->type);
	n->type = nt;
	if (nt != t) {
		n = tree(T_CAST, NULL, n);
		n->type = t;
	}
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
	out_block(n, sizeof(struct node));
	if (n->left)
		write_subtree(n->left);
	if (n->right)
		write_subtree(n->right);
	free_node(n);
}

void write_tree(struct node *n)
{
	out_block("%^", 2);
	write_subtree(n);
}

void write_null_tree(void)
{
	write_tree(tree(T_NULL, NULL, NULL));
}

/*
 *	Trees with type rules
 */

struct node *bool_tree(struct node *n)
{
	if (n->op == T_BOOL)
		return n;
	n = tree(T_BOOL, NULL, n);
	n->type = CINT;
	return n;
}

/* Calculate arithmetic promotion */
static unsigned arith_promotion(unsigned lt, unsigned rt)
{
	if (PTR(lt))
		lt = UINT;
	if (PTR(rt))
		rt = UINT;
	/* Our types are ordered for a reason */
	/* Does want review versus standard TODO */
	if (rt > lt)
		lt = rt;
	if (lt < CINT)
		lt = CINT;
	if (lt < FLOAT) {
		if((rt | lt) & UNSIGNED)
			lt |= UNSIGNED;
	}
	return lt;
}

struct node *arith_promotion_tree(unsigned op, struct node *l,
				  struct node *r)
{
	/* We know both sides are arithmetic */
	unsigned lt = type_canonical(l->type);
	unsigned rt = type_canonical(r->type);
	struct node *n;

	lt = arith_promotion(lt, rt);

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
		struct node *n;
		lt = arith_promotion(lt, lt);
		if (lt != rt)
			l = make_cast(l, lt);
		n = tree(op, l, make_cast(r, CINT));
		n->type = lt;
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
	return bool_tree(n);
}

struct node *assign_tree(struct node *l, struct node *r)
{
	unsigned lt = type_canonical(l->type);
	unsigned rt = type_canonical(r->type);

	if (lt == rt)
		return sf_tree(T_EQ, l, r);
	if (PTR(lt)) {
		type_pointermatch(l, r);
		return sf_tree(T_EQ, l, r);
	} else if (PTR(rt))
		typemismatch();
	else if (!IS_ARITH(lt) || !IS_ARITH(rt)) {
		invalidtype();
	}
	return sf_tree(T_EQ, l, make_cast(r, l->type));
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
	n = tree(op, bool_tree(l), bool_tree(r));
	n->type = CINT;
	return n;
}

/* Constant conversion

   Needs review and to be a bit more precise
 */
unsigned long trim_constant(unsigned t, unsigned long value, unsigned warn)
{
	int sign = 1;
	unsigned long ov = value;

	/* Signed is more fun */
	if (!(t & UNSIGNED)) {
		if ((signed long)value < 0) {
			sign = -1;
			value = -value;
		}
	}
	/* Now trim the unsigned bit pattern */
	switch(t & 0xF0) {
	case UCHAR:
		value &= TARGET_CHAR_MASK;
		break;
	case USHORT:
		value &= TARGET_SHORT_MASK;
		break;
	case ULONG:
		value &= TARGET_LONG_MASK;
		break;
	}
	/* And do the range check */
	if (warn && ov != value)
		warning("out of range");
	/* Then put the sign back so we sign extend into the upper bits */
	return ((signed long)value) * sign;
}

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
	value = trim_constant(t, value, 0);
	return make_constant(value, t);
}

static unsigned is_name(unsigned n)
{
	if (n >= T_NAME && n <= T_ARGUMENT)
		return 1;
	return 0;
}

/* Check of the tree has side effects */
static unsigned tree_impure(struct node *n)
{
	if (n->right) {
		if (tree_impure(n->right))
			return 1;
	}
	if (n->left) {
		if (tree_impure(n->left))
			return 1;
	}
	return n->flags & SIDEEFFECT;
}

/*
 *	TODO:
 *	We need a sensible way of not rewalking the same trees
 *
 *	Walk down a tree and attempt to reduce it to the simplest
 *	form we can manage. At the moment we do this repeatedly as
 *	we build the tree but that needs rethinking. On the other hand
 *	we don't want to do it at the end or we risk running out of nodes
 *	as we remove a lot of nodes as we go.
 */

struct node *constify(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned op = n->op;

	/* Remember if we are a node or child of a node that has a side
	   effect. This determines what can be eliminated */

	/* Casting of constant form objects */
	/* We block casting of structures and arrays to each other higher up
	   so all we have to worry about is truncating constants and just
	   relabelling the type on a name or label */
	if (op == T_CAST) {
		if (r->op == T_CONSTANT)
			return replace_constant(n, n->type, r->value);
		if (r->op == T_NAME || r->op == T_LABEL) {
			r->type = n->type;
			free_node(n);
			return r;
		}
	}
	/* Remove multiply by 1 or 0 */
	if (op == T_STAR && r->op == T_CONSTANT) {
		if (r->value == 1) {
			free_node(r);
			free_node(n);
			return l;
		}
		/* We can only do this if n and l have no side effects */
		if (r->value == 0 && !tree_impure(l)) {
			l = make_constant(0, n->type);
			free_tree(n);
			return l;
		}
	}
	/* Divide by 1 */
	if (op == T_SLASH && r->op == T_CONSTANT) {
		if (r->value == 1) {
			free_node(n);
			free_tree(r);
			return l;
		}
	}
	/* Unsigned ops that resolve to never true/false */
	if (r && r->op == T_CONSTANT && r->value == 0 && (n->type & UNSIGNED) && !tree_impure(l)) {
		if (op == T_LT) {
			free_tree(l);
			free_node(r);
			free_node(n);
			warning("always false");
			return bool_tree(make_constant(0, CINT));
		}
		if (op == T_GTEQ) {
			free_tree(l);
			free_node(r);
			free_node(n);
			warning("always true");
			return bool_tree(make_constant(1, CINT));
		}
	}
#if 0
	/* This will always fail as we don't eliminate T_BOOL or understand
	   T_BOOL(const) is a constant value that still typecasts and flag sets */
	/* The logic ops are special as they permit shortcuts and need to be
	   dealt with l->r */
	if (op == T_ANDAND || op == T_OROR) {
		l = constify(l);
		if (l == NULL || !IS_INTARITH(l->type) || (l->flags & LVAL))
			return NULL;
		if (op == T_OROR && l->value) {
			free_tree(l);
			free_tree(r);
			free_node(n);
			return bool_tree(make_constant(1, CINT));
		}
		if (op == T_ANDAND && !l->value) {
			free_tree(l);
			free_tree(r);
			free_node(n);
			return bool_tree(make_constant(0, CINT));
		}
	}
#endif
	if (r) {
		r = constify(r);
		if (r == NULL)
			return NULL;
		n->right = r;
	}
	if (l) {
		unsigned lt = l->type;
		unsigned long value = l->value;

		/* Lval names are constant but a maths operation on two name lval is not */
		if (is_name(l->op) || is_name(r->op)) {
			if (op != T_PLUS)
				return NULL;
			/* Special case for name + const */
			if (is_name(l->op)) {
				if (is_name(r->op))
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
		if (l) {
			l = constify(l);
			if (l == NULL)
				return NULL;
			n->left = l;
		}
		/* Only do constant work with simple types */
		if (!IS_INTARITH(lt) && !PTR(lt))
			return NULL;
		if (l->flags & LVAL)
			return NULL;

		switch(op) {
		case T_PLUS:
			value += r->value;
			break;
		case T_MINUS:
			value -= r->value;
			break;
		case T_STAR:
			value *= r->value;
			break;
		case T_SLASH:
			/* Zero may cause an exception which may be what
			   the programmer wanted so don't optimize it out */
			if (r->value == 0) {
				divzero();
				return NULL;
			} else if (l->type & UNSIGNED)
				value /= r->value;
			else
				value = (signed long)value / r->value;
			break;
		case T_PERCENT:
			if (r->value == 0) {
				divzero();
				return NULL;
			} else if (l->type & UNSIGNED)
				value %= r->value;
			else
				value = (signed long)value % r->value;
			break;
		case T_ANDAND:
			value = value && r->value;
			break;
		case T_OROR:
			value = value || r->value;
			break;
		case T_AND:
			value &= r->value;
			break;
		case T_OR:
			value |= r->value;
			break;
		case T_HAT:
			value ^= r->value;
			break;
		case T_LTLT:
			value <<= r->value;
			break;
		case T_GTGT:
			if (l->type & UNSIGNED)
				value >>= r->value;
			else
				value = ((signed long)value) >> r->value;
			break;
		case T_LT:
			if (l->type & UNSIGNED)
				value = value < r->value;
			else
				value = (signed long)value < (signed long )r->value;
			break;
		case T_LTEQ:
			if (l->type & UNSIGNED)
				value = value <= r->value;
			else
				value = (signed long)value <= (signed long )r->value;
			break;
		case T_GT:
			if (l->type & UNSIGNED)
				value = value < r->value;
			else
				value = (signed long)value < (signed long )r->value;
			break;
		case T_GTEQ:
			if (l->type & UNSIGNED)
				value = value < r->value;
			else
				value = (signed long)value < (signed long )r->value;
			break;
		default:
			return NULL;
		}
		return replace_constant(n, lt, value);
	}
	if (r) {
		/* Uni-ops */
		unsigned rt = r->type;
		unsigned long value = r->value;
		if (!IS_INTARITH(rt) && !PTR(rt))
			return NULL;
		if (r->flags & LVAL)
			return NULL;

		switch(op) {
		case T_NEGATE:
			/* This also cleans up any negative constants that were tokenized
			   as T_NEGATE, T_CONST <n> */
			value =  -value;
			break;
		case T_TILDE:
			value = ~value;
			break;
		case T_BANG:
			value = !value;
			break;
		/* We can't do T_BOOL because it's used to set flags */
		case T_CAST:
			/* We are working with integer constant types so this is ok */
			return replace_constant(n, n->type, value);
		default:
			return NULL;
		}
		return replace_constant(n, rt, value);
	}
	/* Terminal node.. are we const ?? */
	if (is_constname(n))
		return n;
	return NULL;
}
