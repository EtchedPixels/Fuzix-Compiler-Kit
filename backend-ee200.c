/*
 *	Electrodata EE200
 *
 *	Like an 8086 our register choices are mostly prescribed by the
 *	instruction set. In particular we have almost no indirect ops
 *	so have to use A and B for this. A, B and X have a good range of
 *	load and stores but X only 16bit ones. X is also used by the rather
 *	strange hybrid call stack/link. Y and Z are indexes but have no
 *	easy to way to load and save them directly. S is the stack pointer
 *
 *	Everything is big endian.
 *
 *	The Centurion CPU4/5 are basically the same but the CPU6 is quite
 *	different and will require some extra handling
 *	- Registers may not be accessible aliased to low RAM
 *	- Single reg ops have a second argument which specifies repeats
 *	  (eg inc by 2 or shift by 5 are possible) and load with small
 *	  constant
 *	- Unsigned 16->32bit MUL and 32->16 DIV instructions uses adjacent
 *	  registers (or memory)
 *	- Direct constant loads to any reg via xfr
 *	- Proper PUSH/POP range instructions allow Y and Z to be pushed too
 *	- Block moves
 *	- Bignum maths (up to 128bit memory->memory including add sub
 *	  mul (signed but buggy for negatives), div (signed) with or without
 *	  remainder
 *	- some single word ops can also operate on memory
 *
 *	Things to do are numerous
 *	- Float
 *	- Multiply and divide optimised forms via shifting
 *	- Rewrite some ops to use indirection like 6809 has
 *	- Tracking B and what A and X point to
 *	- register variable support for Y and Z
 *	- CCONLY tracking
 *	- Use X instead of A whenever X happens to hold the right stuff
 *	  (and vice versa)
 *	- CPU6 support
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

#define ARGBASE	4	/* Bytes between arguments and locals if no reg saves */

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned unreachable;	/* Code is unreachable */
static unsigned argbase;	/* Argument offset in current function */

/* Export the C symbol */
void gen_export(const char *name)
{
	printf("	.export _%s\n", name);
}

void gen_segment(unsigned s)
{
	switch(s) {
	case A_CODE:
		printf("\t.code\n");
		break;
	case A_DATA:
		printf("\t.data\n");
		break;
	case A_LITERAL:
		printf("\t.literal\n");
		break;
	case A_BSS:
		printf("\t.bss\n");
		break;
	default:
		error("gseg");
	}
}

void gen_prologue(const char *name)
{
	unreachable = 0;
	printf("_%s:\n", name);
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size;
	argbase = ARGBASE;
	printf("\tstx (-s)\n");	/* Stack X (our return addr) */
	if (func_flags & F_REG(1)) {
		printf("\txfr y,x\n");
		printf("\tstx (-s)\n");
		argbase += 2;
	}
	if (func_flags & F_REG(2)) {
		printf("\txfr z,x\n");
		printf("\tstx (-s)\n");
		argbase += 2;
	}
	if (size) {
		if (size <= 2) {
			printf("\tdcr s\n");
			if (size == 2)
				printf("\tdcr s\n");
		} else {
			printf("\tldb %u\n", ((unsigned)-size) & 0xFFFF);
			printf("\tadd b,s\n");
		}
	}
	sp = 0;
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp != 0) {
		error("sp");
	}
	if (unreachable == 1)
		return;
	if (size == 1)
		printf("\tinr s\n");
	else if (size == 2)
		printf("\tinr s\n\tinr s\n");
	else if (size) {
		printf("\tldb %u\n", size);
		printf("\tadd b,s\n");
	}
	if (func_flags & F_REG(2)) {
		printf("\tlda (s+)\n");
		printf("\txaz\n");
	}
	if (func_flags & F_REG(1)) {
		printf("\tlda (s+)\n");
		printf("\txay\n");
	}
	printf("\tldx (s+)\n");
	printf("\trsr\n");
	unreachable = 1;
}

void gen_label(const char *tail, unsigned n)
{
	printf("L%d%s:\n", n, tail);
	unreachable = 0;
}

unsigned gen_exit(const char *tail, unsigned n)
{
	printf("\tjmp L%d%s\n", n, tail);
	unreachable = 1;
	return 0;
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\tjmp L%d%s\n", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\tjz L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjnz L%d%s\n", n, tail);
}

void gen_switch(unsigned n, unsigned type)
{
	printf("\tldx Sw%u\n", n);
	printf("\tjmp __switch");
	helper_type(type, 0);
	printf("\n");
	unreachable = 1;
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	printf("Sw%d_%d:\n", tag, entry);
	unreachable = 0;
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

void gen_helpcall(struct node *n)
{
	printf("\tjsr __");
}

void gen_helptail(struct node *n)
{
}

void gen_helpclean(struct node *n)
{
}

void gen_data_label(const char *name, unsigned align)
{
	printf("_%s:\n", name);
}

void gen_space(unsigned value)
{
	printf("\t.ds %d\n", value);
}

void gen_text_data(unsigned n)
{
	printf("\t.word T%d\n", n);
}

void gen_literal(unsigned n)
{
	if (n)
		printf("T%d:\n", n);
}

void gen_name(struct node *n)
{
	printf("\t.word _%s+%d\n", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	if (PTR(type)) {
		printf("\t.word %u\n", (unsigned) value);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		printf("\t.byte %u\n", (unsigned) value & 0xFF);
		break;
	case CSHORT:
	case USHORT:
		printf("\t.word %d\n", (unsigned) value & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are big endian */
		printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
		printf("\t.word %d\n", (unsigned) (value & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	printf("\t.setcpu %u\n", cpu);
	printf("\t.code\n");
}

void gen_end(void)
{
}

void gen_tree(struct node *n)
{
	codegen_lr(n);
	printf(";\n");
}

/*
 *	Example size handling. In this case for a system that always
 *	pushes words.
 */
static unsigned get_size(unsigned t)
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

static unsigned get_stack_size(unsigned t)
{
	unsigned n = get_size(t);
	if (n == 1)
		return 2;
	return n;
}

#define T_NREF		(T_USER)		/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)		/* Function call by name */
#define T_NSTORE	(T_USER+2)		/* Store to a C global/static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)		/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)

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
 *	Heuristic for guessing what to put on the right. Anything we can
 *	get into A without trashing B.
 */

static unsigned is_simple(struct node *n)
{
	unsigned op = n->op;

	/* Multi-word objects are never simple */
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;

	/* We can load these directly into a register */
	if (op == T_CONSTANT || op == T_LABEL || op == T_NAME)
		return 10;
	/* We can these directly into a register */
	if (op == T_NREF || op == T_LBREF)
		return 10;
	/* We can these into A in two */
	if (op == T_LOCAL || op == T_ARGUMENT)
		return 5;
	/* We can usually get this into A */
	if (op == T_LREF)
		return 1;
	return 0;
}

/* Chance to rewrite the tree from the top rather than none by node
   upwards. We will use this for 8bit ops at some point and for cconly
   propagation */
struct node *gen_rewrite(struct node *n)
{
	return n;
}

/*
 *	Our chance to do tree rewriting. We don't do much
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned op = n->op;
	unsigned nt = n->type;

	/* Rewrite references into a load operation */
	if (nt == CCHAR || nt == UCHAR || nt == CSHORT || nt == USHORT || PTR(nt)) {
		if (op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += argbase + frame_len;
				squash_right(n, T_LREF);
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
/*		printf(";left %d right %d\n", is_simple(l), is_simple(r)); */
		if (is_simple(l) > is_simple(r)) {
			n->right = l;
			n->left = r;
		}
	}
	return n;
}


unsigned gen_push(struct node *n)
{
	unsigned size = get_stack_size(n->type);
	printf("\tstb (-s)\n");
	if (size == 4) {
		printf("\tlda __hireg\n");
		printf("\tsta (-s)\n");
	}
	/* Our push will put the object on the stack, so account for it */
	sp += get_stack_size(n->type);
	return 1;
}

unsigned op_into_r(char r, struct node *n, unsigned s, const char *b, const char *w)
{
	unsigned v = n->value;

	/* Avoid long stuff for now */
	if (s > 2)
		return 0;

	switch(n->op) {
	case T_NAME:
		printf("\tld%c _%s+%u\n", r, namestr(n->snum), v);
		break;
	case T_LABEL:
		printf("\tld%c T%u+%u\n", r, n->val2, v);
		break;
	case T_ARGUMENT:
		v += argbase + frame_len;
		/* Fall through */
	case T_LOCAL:
		v += sp;
		printf("\tld%c %u\n", r, v);
		printf("\tadd s,%c\n", r);
		break;
	case T_CONSTANT:
		/* Slightly messy as we want to use implicit form
		   for A as it is smaller */
		if (r == 'a') {
			if (s == 1 && (v & 0xFF) == 0) {
				printf("\tclab\n");
				break;
			} else if (s == 2 && (v & 0xFFFF) == 0) {
				printf("\tcla\n");
				break;
			}
		} else if (s == 2 && (v & 0xFFFF) == 0) {
			printf("\tclr %c\n", r);
			break;
		}
		if (s == 1)
			printf("\tld%cb %u\n", r, v & 0xFF);
		else
			printf("\tld%c %u\n", r, v & 0xFFFF);
		break;
	case T_LREF:
		v += sp;
		if (v < 128) {
			if (s == 1)
				printf("\tld%cb %u(s)\n", r, v);
			else
				printf("\tld%c %u(s)\n", r, v);
		} else {
			printf("\tld%c %u\n", r, v);
			printf("\tadd s,%c\n", r);
			if (s == 1)
				printf("\tld%cb (%c)\n", r, r);
			else
				printf("\tld%c (%c)\n", r, r);
		}
		break;	
	case T_NREF:
		if (s == 1)
			printf("\tld%cb (_%s+%u)\n", r, namestr(n->snum), v);
		else
			printf("\tld%c (_%s+%u)\n", r, namestr(n->snum), v);
		break;
	case T_LBREF:
		if (s == 1)
			printf("\tld%cb (T%u+%u)\n", r, n->val2, v);
		else
			printf("\tld%c (T%u+%u)\n", r, n->val2, v);
		break;
	default:
		return 0;
	}
	if (b)
		printf("\t%s\n", (s == 1) ? b : w);
	return 1;
}

unsigned can_op_into_r(char r, struct node *n, unsigned s)
{
	/* Avoid long stuff for now */
	if (s > 2)
		return 0;

	switch(n->op) {
	case T_NAME:
	case T_LABEL:
	case T_ARGUMENT:
	case T_LOCAL:
	case T_CONSTANT:
	case T_LREF:
	case T_NREF:
	case T_LBREF:
		return 1;
	}
	return 0;
}

/* Anything goes providing  B survives so we should look at folding more
   complex expressions when possible */
unsigned op_into_a(struct node *n, unsigned s, const char *b, const char *w)
{
	return op_into_r('a', n, s, b, w);
}

unsigned op_into_b(struct node *n, unsigned s, const char *b, const char *w)
{
	return op_into_r('b', n, s, b, w);
}

unsigned op_into_x(struct node *n, unsigned s, const char *b, const char *w)
{
	/* X only does word sized */
	if (s == 1)
		return 0;
	return op_into_r('x', n, s, b, w);
}

/* Load a reference to an object into a register and return the reg,offset
   pair to use */
int load_register(struct node *n, unsigned s, char *r)
{
	unsigned v = n->value;

	*r = 'x';

	/* Avoid long stuff for now */
	if (s != 2)
		return -1;

	/* Cases we can use off,s directly */

	switch(n->op) {
	case T_ARGUMENT:
		v += argbase + frame_len;
		/* Fall through */
	case T_LOCAL:
		v += sp;
		if (v < 128) {
			*r = 's';
			return v;
		}
		break;
	case T_LREF:
		v += sp;
		if (v < 128) {
			printf("\tldx %u(s)\n", v);
			return 0;
		}
		break;
	}
	/* Try and get it into X */
	if (op_into_x(n, s, 0, 0) == 0)
		return -1;
	/* It's in X offset 0 */
	return 0;
}

unsigned can_load_reg(struct node *n, unsigned s)
{
	/* Avoid long stuff for now */
	if (s > 2)
		return 0;
	switch(n->op) {
	case T_NAME:
	case T_LABEL:
	case T_ARGUMENT:
	case T_LOCAL:
	case T_CONSTANT:
	case T_LREF:
	case T_NREF:
	case T_LBREF:
		return 1;
	default:
		return 0;
	}
	return 1;
}

unsigned condop(struct node *n, const char * o, const char *ou)
{
	struct node *r = n->right;
	unsigned s = get_size(r->type);
	if (s == 4)
		return 0;
	if (r->type & UNSIGNED)
		o = ou;
	if (op_into_a(r, s, "sabb", "sab") == 0)
		return 0;
	printf("\tjsr __%s\n", ou);
	n->flags |= ISBOOL;
	return 1;
}

void repeated_op(unsigned n, const char *op)
{
	while(n--)
		printf("\t%s\n", op);
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 */
unsigned gen_direct(struct node *n)
{
	unsigned v;
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned nr = n->flags & NORETURN;

	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		v = r->value;
		if (v == 1)
			printf("\tinr s\n");
		else {
			printf("\tlda %u\n", v);
			printf("\tadd a,s\n");
		}
		sp -= v;
		return 1;
	case T_EQ:
		return op_into_a(r, s, "stab (b)", "sta (b)");
	case T_PLUS:
		v = r->value;
		if (s <= 2 && r->op == T_CONSTANT && v <= 2) {
			repeated_op(v, s == 1 ? "inrb bl" : "inr b");
			return 1;
		}
		return op_into_a(r, s, "aabb", "aab");
	case T_AND:
		/* TODO: long cases */
		if (r->op == T_CONSTANT && s == 2) {
			v = r->value;
			if (v == 0) {
				printf("\tcla\n");
				return 1;
			}
			if (v == 0xFFFF)
				return 1;
		}
		return op_into_a(r, s, "nabb", "nab");
	case T_OR:
		if (r->op == T_CONSTANT && s == 2) {
			v = r->value;
			if (v == 0)
				return 1;
			if (v == 0xFFFF) {
				printf("\tldb 0xFFFF\n");
				return 1;
			}
		}
		return op_into_a(r, s, "orib al,bl", "ori a,b");
	case T_HAT:
		if (r->op == T_CONSTANT && s == 2) {
			v = r->value;
			if (v == 0)
				return 1;
			if (v == 0xFFFF) {
				printf("\tivr b\n");
				return 1;
			}
		}
		return op_into_a(r, s, "oreb al,bl", "ore a,b");
	case T_MINUS:	/* We have a reverse subtract so just do the
			   simple case */
		if (r->op == T_CONSTANT) {
			v = r->value;
			if (s == 1) {
				if (v <= 2) {
					repeated_op(v, "dcrb b");
					return 1;
				}
				printf("\tldab %u\n", (-v) & 0xFF);
				printf("\taabb\n");
				return 1;
			}
			if (s == 2) {
				if (v <= 2) {
					repeated_op(v, "dcr b");
					return 1;
				}
				printf("\tlda %u\n", (-v) & 0xFFFF);
				printf("\taab\n");
				return 1;
			}
		}
		/* Our subtract is dst = src - dst, but at this point we
		   have the left side in B */
		if (op_into_a(r, s, NULL, NULL)) {
			/* We now want to do B - A into B */
			/* A = B - A */
			printf("\tsub b,a\n");
			/* And into B */
			printf("\txab\n");
			return 1;
		}
		break;
	case T_PLUSEQ:
		if (op_into_a(r, s, NULL, NULL)) {
			if (s == 1) {
				printf("\txfr b,x\n");
				printf("\tldbb (x)\n");
				printf("\taabb\n");
				printf("\tstbb (x)\n");
				return 1;
			}
			if (s == 2) {
				printf("\txfr b,x\n");
				printf("\tldb (x)\n");
				printf("\taab\n");
				printf("\tstb (x)\n");
				return 1;
			}
		}
		break;
	case T_PLUSPLUS:
		/* R is alway a constant */
		if (op_into_a(r, s, NULL, NULL)) {
			if (s == 1) {
				printf("\txfr b,x\n");
				printf("\tldbb (x)\n");
				if (!nr)
					printf("\tstb (-s)\n");
				printf("\taabb\n");
				printf("\tstb (x)\n");
				if (!nr)
					printf("\tldb (s+)\n");
				return 1;
			}
			if (s == 2) {
				printf("\txfr b,x\n");
				printf("\tlda (x)\n");
				if (!nr)
					printf("\tsta (-s)\n");
				printf("\taab\n");
				printf("\tsta (x)\n");
				if (!nr)
					printf("\tlda (s+)\n");
				return 1;
			}
		}
		break;
	case T_MINUSEQ:
		/* We have a reverse subtract so we need the value in B */
		if (can_op_into_r('a', r, s)) {
			printf("\txfr b,x\n");
			op_into_b(r, s, NULL, NULL);
			if (s == 1) {
				printf("\tldab (x)\n");
				printf("\tsabb\n");
				printf("\tstbb (x)\n");
				return 1;
			}
			if (s == 2) {
				printf("\tlda (x)\n");
				printf("\tsab\n");
				printf("\tstb (x)\n");
				return 1;
			}
		}
		break;
	case T_MINUSMINUS:
		/* r is always constant but might be float or oversize */
		if (can_op_into_r('a', r, s)) {
			printf("\txfr b,x\n");
			op_into_b(r, s, NULL, NULL);
			if (s == 1) {
				printf("\tldab (x)\n");
				if (!nr)
					printf("\tsta (-s)\n");
				printf("\tsabb\n");
				printf("\tstbb (x)\n");
				if (!nr)
					printf("\tldb (s+)\n");
				return 1;
			}
			if (s == 2) {
				printf("\tlda (x)\n");
				if (!nr)
					printf("\tsta (-s)\n");
				printf("\tsab\n");
				printf("\tstb (x)\n");
				if (!nr)
					printf("\tldb (s+)\n");
				return 1;
			}
		}
		break;
	/* TODO  - optimized constant forms
	case T_ANDEQ:
	case T_OREQ:
	case T_HATEQ: 
	*/
	case T_LTLT:
		if (r->op == T_CONSTANT && s <= 2) {
			v = r->value;
			if (v >= s * 8) {
				if (s == 1)
					printf("\tclrb bl\n");
				else
					printf("\tclr b\n");
				return 1;
			}
			if (v >= 8) { 
				printf("\txfrb bl,bh\n");
				printf("\tclrb bl\n");
				v -= 8;
			}
			if (v < 3 || opt > 1) {
				repeated_op(v, s == 1 ? "slrb b" : "slr b");
				return 1;
			}
			printf("\tjsr __shl%u\n", v);
			return 1;
		}
		return 0;
	case T_GTGT:
		if (r->op == T_CONSTANT && s <= 2) {
			v = r->value & 15;
			if (v == 0)
				return 1;
			/* No right shift unsigned */
			if (n->type & UNSIGNED) {
				if (v >= 8) {
					printf("\txfrb ah,al\n");
					printf("\tclrb ah\n");
					v -= 8;
					/* High bit is clear so signed is fine */
					if (v > 2)
						printf("\tjsr __shr%u\n", v);
					else
						repeated_op(v, s == 1 ? "srrb b" : "srr b");
				} else {
					/* Do one bit by hand then helper */
					printf("\trl\n");
					printf("\trrr b\n");
					v--;
					if (v > 2)
						printf("\tjsr __shr%u\n", v);
					else
						repeated_op(v, s == 1 ? "srrb b" : "srr b");
				}
				return 0;
			} else {
				if (v >= s * 8) {
					if (s == 1)
						printf("\tclrb bl\n");
					else
						printf("\tclr b\n");
					return 1;
				}
				if (v >= 8) {
					printf("\tjsr __shr8\n");
					v -= 8;
				}
				if (v < 3 || opt > 1) {
					repeated_op(v, s == 1 ? "srrb b" : "srr b");
					return 1;
				}
				printf("\tjsr __shr%u\n", v);
				return 1;
			}
		}
		return 0;
		/* These come out reversed due to the way SAB works */
	case T_EQEQ:
		return condop(n, "cceq", "cceq");
	case T_BANGEQ:
		return condop(n, "ccne", "ccne");
	case T_GT:
		return condop(n, "cclt", "ccltu");
	case T_LTEQ:
		return condop(n, "ccgteq", "ccgtequ");
	case T_LT:
		return condop(n, "ccgt", "ccgtu");
	case T_GTEQ:
		return condop(n, "cclteq", "ccltequ");
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
	return 0;
}

unsigned short_op(struct node *n, unsigned s, const char *opb, const char *op)
{
	char r;
	int off;

	if (s > 2)
		return 0;

	if (s == 1)
		op = opb;

	codegen_lr(n->right);
	off = load_register(n->left, 2, &r);
	if (s == 1) {
		printf("\tldab %u(%c)\n", off, r);
		printf("\t%s\n", op);
		printf("\tstbb %u(%c)\n", off, r);
		return 1;
	}
	if (s == 2) {
		printf("\tlda %u(%c)\n", off, r);
		printf("\t%s\n", op);
		printf("\tstb %u(%c)\n", off, r);
		return 1;
	}
	return 0;
}

/*
 *	Allow the code generator to shortcut trees it knows
 */
unsigned gen_shortcut(struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned nr = n->flags & NORETURN;
	struct node *l = n->left;
	struct node *r = n->right;
	char reg;
	int off;

	if (unreachable)
		return 1;

	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	if (n->op == T_COMMA) {
		l->flags |= NORETURN;
		codegen_lr(l);
		/* Parent determines child node requirements */
		r->flags |= nr;
		codegen_lr(r);
		return 1;
	}

	switch(n->op) {
	case T_EQEQ:
		if (can_load_reg(l, 2)) {
			codegen_lr(r);
			off = load_register(l, 2, &reg);
			if (s == 1)
				printf("stbb %u(%c)\n", off , reg);
			else
				printf("stb %u(%c)\n", off, reg);
			return 1;
		}
		break;
	case T_PLUSPLUS:
		if (!nr && s <= 2 && can_load_reg(l, 2)) {
			codegen_lr(r);
			off = load_register(l, 2, &reg);
			/* TODO: optimize 1 case to use inr */
			if (s == 1) {
				printf("\tldab %u(%c)\n", off, reg);
				printf("\taabb\n");
				printf("\tstbb %u(%c)\n", off, reg);
				if (!nr)
					printf("\txabb\n");
			} else {
				printf("\tlda %u(%c)\n", off, reg);
				printf("\taab\n");
				printf("\tstb %u(%c)\n", off, reg);
				if (!nr)
					printf("\txab\n");
			}
			return 1;
		}
		/* Fall through */
	case T_PLUSEQ:
		if (s <= 2 && can_load_reg(l, 2)) {
			/* Specific optimization possible for single byte add */
			if (r->op == T_CONSTANT && r->value == 1) {
				off = load_register(l, 2, &reg);
				if (s == 1) {
					printf("\tldbb %u(%c)\n", off, reg);
					printf("\tinrb bl\n");
					printf("\tstbb %u(%c)\n", off, reg);
					return 1;
				}
				if (s == 2) {
					printf("\tldb %u(%c)\n", off, reg);
					printf("\tinr b\n");
					printf("\tstb %u(%c)\n", off, reg);
					return 1;
				}
			}
			return short_op(n, s, "aabb", "aab");
		}
		break;
	/* TODO MINUSEQ / MINUSMINUS */
	case T_ANDEQ:
		if (s <= 2 && can_load_reg(l, 2))
			return short_op(n, s, "nabb", "nab");
		break;
	case T_OREQ:
		if (s <= 2 && can_load_reg(l, 2))
			return short_op(n, s, "orib al,bl", "ori a,b");
		break;
	case T_HATEQ:
		if (s <= 2 && can_load_reg(l, 2))
			return short_op(n, s, "oreb al,bl", "ore a,b");
		break;
	}
	return 0;
}

unsigned popop(struct node *n, const char *op)
{
	unsigned s = get_size(n->type);
	if (s == 4)
		return 0;
	printf("\tlda (s+)\n");
	if (n->op == T_AND || n->op == T_PLUS || n->op == T_MINUS)	/* Short forms */
		printf("\t%s%s\n", op, s == 1 ? "b": "");
	else if (s == 1)
		printf("\t%sb al,bl\n", op);
	else
		printf("\t%s a,b\n", op);
	return 1;
}

/* Top of stack is pointer, value in B ops are transitive except minus
   which works out fine due to the rsub nature */
unsigned popeq(struct node *n, const char *o)
{
	unsigned s = get_size(n->type);
	const char *ss = "";
	if (s == 4)
		return 0;
	if (s == 1)
		ss = "b";
		
	printf("\tldx (s+)\n");
	printf("\tlda%s (x)\n", ss);
	if (n->op == T_ANDEQ || n->op == T_PLUSEQ ||
		n->op == T_MINUSEQ)	/* Short forms */
		printf("\t%s%s\n", o, ss);
	else if (s == 1)
		printf("\t%sb al,bl\n", o);
	else
		printf("\t%s a,b\n", o);
		
	printf("\tstb%s (x)\n", ss);
	return 1;
}

unsigned popcond(struct node *n, const char *o, const char *ou)
{
	unsigned s = get_size(n->right->type);
	if (s == 4)
		return 0;
	printf("\tlda (s+)\n");
	if (n->right->type & UNSIGNED)
		o = ou;
	printf("\tsab%s\n", s == 1 ? "b" : "");
	printf("\tjsr __%s\n", o);
	n->flags |= ISBOOL;
	return 1;
}

unsigned gen_cast(struct node *n)
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
	if ((lt & ~UNSIGNED) <= (rt & ~UNSIGNED))
		return 1;
	/* Signed via helper */
	if (!(rt & UNSIGNED))
		return 0;
	if (rs == 1)
		printf("\tclrb ah\n");
	if (ls == 4) {
		printf("\tcla\n");
		printf("\tsta (__hireg)\n");
	}
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned v = n->value;
	unsigned s = get_size(n->type);
	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_FUNCCALL && n->op != T_CALLNAME)
		sp -= get_stack_size(n->left->type);
	switch(n->op) {
	case T_CAST:
		return gen_cast(n);
	case T_EQ:
		/* Long will end up here for now - but eventually we want long lref etc */
		if (s == 4) {
			printf("\tldx (s+)\n");
			printf("\tlda (__hireg)\n");
			printf("\tsta (x)\n");
			printf("\tstb 2(x)\n");
			return 1;
		}
		/* Hard assignments (nothing simple each side) end up here */
		if (s < 4) {
			printf("\tlda (s+)\n");
			if (s == 1)
				printf("\tstbb (a)\n");
			else
				printf("\tstb (a)\n");
			return 1;
		}
		break;
	case T_CONSTANT:
		if (s == 1) {
			v &= 0xFF;
			if (v == 0)
				printf("\tclrb bl\n");
			else
				printf("\tldbb %u\n", v & 0xFF);
			return 1;
		}
		if (s == 2) {
			if (v == 0)
				printf("\tclr b\n");
			else
				printf("\tldb %u\n", v);
			return 1;
		}
		if (s == 4) {
			unsigned vh = n->value >> 16;
			if (vh == 0)
				printf("\tclr b\n");
			else {
				printf("\tldb %u\n", vh);
				printf("\tstb (__hireg)\n");
			}
			if (vh != v) {
				if (v == 0)
					printf("\tclr b\n");
				else
					printf("\tldb %u\n", v & 0xFFFF);
			}
			return 1;
		}
		return 0;
	case T_LABEL:
		printf("\tldb T%u+%u\n", n->val2, v);
		return 1;
	case T_LBREF:
		if (s == 1)
			printf("\tldbb (T%u+%u)\n", n->val2, v);
		else
			printf("\tldb (T%u+%u)\n", n->val2, v);
		return 1;
	case T_LBSTORE:
		if (s == 1)
			printf("\tstbb (T%u+%u)\n", n->val2, v);
		else
			printf("\tstb (T%u+%u)\n", n->val2, v);
		return 1;
	case T_NAME:
		printf("\tldb _%s+%u\n", namestr(n->snum), v);
		return 1;
	case T_NREF:
		if (s == 1)
			printf("\tldbb (_%s+%u)\n", namestr(n->snum), v);
		else
			printf("\tldb (_%s+%u)\n", namestr(n->snum), v);
		return 1;
	case T_NSTORE:
		if (s == 1)
			printf("\tstbb (_%s+%u)\n", namestr(n->snum), v);
		else
			printf("\tstb (_%s+%u)\n", namestr(n->snum), v);
		return 1;
	case T_ARGUMENT:
		v += argbase + frame_len;
		/* Fall through */
	case T_LOCAL:
		v += sp;
		if (v) {
			printf("\tldb %u\n", v);
			printf("\tadd s,b\n");
		} else
			printf("\txfr s,b\n");
		return 1;
	case T_LREF:
		v += sp;
		printf(";lref %u\n", v);
		if (v < 128) {	
			if (s == 1) {
				printf("\tldbb %u(s)\n", v);
				return 1;
			}
			if (s == 2) {
				printf("\tldb %u(s)\n", v);
				return 1;
			}
		}
		/* Do it the longer way. We could one day track A offsetting */
		printf("\tlda %u\n", v);
		printf("\tadd s,a\n");
		if (s == 1)
			printf("\tldbb (a)\n");
		else
			printf("\tldb (a)\n");
		return 1;
	case T_LSTORE:
		v += sp;
		if (v < 128) {	
			if (s == 1) {
				printf("\tstbb %u(s)\n", v);
				return 1;
			}
			if (s == 2) {
				printf("\tstb %u(s)\n", v);
				return 1;
			}
		}
		/* Do it the longer way. We could one day track A offsetting */
		printf("\tlda %u\n", v);
		printf("\tadd s,a\n");
		if (s == 1)
			printf("\tstbb (a)\n");
		else
			printf("\tstb (a)\n");
		return 1;
	case T_DEREF:
		if (s == 1) {
			printf("\tldbb (b)\n");
			return 1;
		}
		if (s == 2) {
			printf("\tldb (b)\n");
			return 1;
		}
		if (s == 4) {
			printf("\tlda (b)\n");
			printf("\tsta (__hireg)\n");
			printf("\tldb 2(b)\n");
			return 1;
		}
		break;
	case T_CALLNAME:
		printf("\tjsr _%s+%u\n", namestr(n->snum), v);
		return 1;
	case T_FUNCCALL:
		printf("\tjsr (b)\n");
		return 1;
	case T_NEGATE:
		if (s < 4) {
			printf("\tivr b\n");
			printf("\tinr b\n");
			return 1;
		}
		break;
	case T_TILDE:
		if (s < 4) {
			printf("\tivr b\n");
			return 1;
		}
		break;
	case T_AND:
		return popop(n, "nab");
	case T_OR:
		return popop(n, "ori");
	case T_HAT:
		return popop(n, "ore");
	case T_PLUS:
		return popop(n, "aab");
	case T_MINUS:
		/* SAB is a reverse subtract (B = A-B) */
		return popop(n, "sab");
	case T_EQEQ:
		return popcond(n, "cceq", "cceq");
	case T_BANGEQ:
		return popcond(n, "ccne", "ccne");
	case T_GT:
		return popcond(n, "ccgt", "ccgtu");
	case T_LTEQ:
		return popcond(n, "cclteq", "ccltequ");
	case T_LT:
		return popcond(n, "cclt", "ccltu");
	case T_GTEQ:
		return popcond(n, "ccgteq", "ccgtequ");
	/* Cases yet to handle : most will want to be helpers mostly */
	case T_STAR:
		if (cpu == 6 && s == 2) {
			printf("\tlda (s+)\n");
			printf("\tmul b,a\n");
			return 1;
		}
		break;
	case T_ANDEQ:
		return popeq(n, "nab");
	case T_OREQ:
		return popeq(n, "ori");
	case T_HATEQ:
		return popeq(n, "ore");
	case T_PLUSEQ:
		return popeq(n, "aab");
	case T_MINUSEQ:
		return popeq(n, "sab");
	case T_PERCENT:
	case T_SLASH:
	case T_BANG:
	case T_BOOL:
	case T_SHLEQ:
	case T_SHREQ:
	case T_STAREQ:
	case T_SLASHEQ:
	case T_PERCENTEQ:
		break;
	}
	return 0;
}
