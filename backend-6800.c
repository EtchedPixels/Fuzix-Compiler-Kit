/*
 *	6800: original in the line of processors ending with 68HC11
 *	- Only has 8bit operations
 *	- Cannot push x (hack to pop it)
 *	- Cannot  move between accumulators and X except via memory
 *	- Can copy X to and from S but can only move to/from accumulator
 *	  via memory
 *	- Some useful flag behaviour on late devices is not present.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

#define ARGBASE	2	/* Bytes between arguments and locals if no reg saves */

#define T_NREF		(T_USER)		/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)		/* Function call by name */
#define T_NSTORE	(T_USER+2)		/* Store to a C global/static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)		/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)
#define T_RREF		(T_USER+7)
#define T_RSTORE	(T_USER+8)
#define T_RDEREF	(T_USER+9)		/* *regptr */
#define T_REQ		(T_USER+10)		/* *regptr = */
#define T_RDEREFPLUS	(T_USER+11)		/* *regptr++ */
#define T_REQPLUS	(T_USER+12)		/* *regptr++ =  */
#define T_LSTREF	(T_USER+13)		/* reference via a local ptr to struct field */
#define T_LSTSTORE	(T_USER+14)		/* store ref via a local ptr to struct field */

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned argbase;	/* Argument offset in current function */
static unsigned unreachable;	/* Code following an unconditional jump */
static unsigned label_count;	/* Used for internal labels X%u: */

/*
 *	Helpers for code generation and trackign
 */

/*
 *	Size handling
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
	return get_size(t);
}

void repeated_op(unsigned n, const char *op)
{
	while(n--)
		printf("\t%s\n", op);
}


/* Tracking: TODO */

void invalidate_all(void)
{
}

/* 16bit constant load */
void load_d_const(uint16_t n)
{
	unsigned hi,lo;
	/* TODO: track AB here and see if we can use existing values */
	lo = n & 0xFF;
	hi = n >> 8;
	if (hi == 0)
		printf("\tclra\n");
	else
		printf("\tlda #%d\n", hi);

	if (lo == 0)
		printf("\tclrb\n");
	else if (lo == hi) {
		printf("\ttab\n");
		return;
	} else
		printf("\tldb #%d\n", lo);
}

void load_b_const(uint16_t n)
{
	/* TODO: tracking */
	if (n == 0)
		printf("\tclrb\n");
	else
		printf("\tldb #%u\n", n & 0xFF);
}


void add_d_const(uint16_t n)
{
	/* TODO: tracking */
	if (n == 0)
		return;
	if (n & 0xFF) { 
		printf("\taddb #%u\n", n & 0xFF);
		printf("\tadca #%u\n", n >> 8);
		return;
	}
	printf("\tadda #%u\n", n >> 8);
}

void add_b_const(uint16_t n)
{
	/* TODO: tracking */
	if (n == 0)
		return;
	printf("\taddb #%u\n", n & 0xFF);
}

void move_s_d(void)
{
	printf("\tsts @tmp\n");
	printf("\tldaa @tmp\n");
	printf("\tldab @tmp+1\n");
}

void move_d_s(void)
{
	printf("\tsts @tmp\n");
	printf("\tldaa @tmp\n");
	printf("\tldab @tmp+1\n");
}

/* Get D into X (may trash D) */
void make_x_d(void)
{
	printf("\tstaa @tmp\n\tstab @tmp+1\n");
	printf("\tldx @tmp\n");
}

void pop_x(void)
{
	/* Must remember this trashes X, or could make it smart
	   when we track and use offsets of current X then ins ins */
	/* Easier said than done on a 6800 */
	printf("\ttsx\n\tldx ,x\n\tins\n\tins\n");
}

void adjust_s(int n, unsigned save_d)
{
	/* It costs us 16 bytes to modify S by adjusting values. 20 if
	   we must save the working registers */
	if (n == 0)
		return;
	/* Forms where ins/des are always best */
	if (n >=0 && n <= 4) {
		repeated_op(n, "ins");
		return;
	}
	if (n < 0 && n >= -4) {
		repeated_op(-n, "des");
		return;
	}
	if (optsize) {
		if (n > 0 && n <= 255) {
			printf("\tjsr __addsp8\n");
			printf("\t.byte %u\n", n & 0xFF);
			return;
		}
		if (n <0 && n >= -255) {
			printf("\tjsr __subsp8\n");
			printf("\t.byte %u\n", n & 0xFF);
			return;
		}
		printf("\tjsr __modsp16\n");
		printf("\t.word %u\n", n);
		return;
	}
	if (n >=0 && n <= 18 + 4 * save_d) {
		repeated_op(n, "ins");
		return;
	}
	if (n < 0 && n >= -4 - 18 - 4 * save_d) {
		repeated_op(-n, "des");
		return;
	}
	/* Inline */
	if (save_d)
		printf("\tpshb\n\tpsha\n");
	move_s_d();
	add_d_const(n);
	move_d_s();
	if (save_d)
		printf("\tpulb\n\tpula\n");
}

void op8_on_ptr(const char *op, unsigned off)
{
	printf("\t%sb %u,x\n", op, off);
}

/* Do the low byte first in case it's add adc etc */
void op16_on_ptr(const char *op, const char *op2, unsigned off)
{
	/* Big endian */
	printf("\t%sb %u,x\n", op, off + 1);
	printf("\t%sa %u,x\n", op2, off);
}

void op32_on_ptr(const char *op, const char *op2, unsigned off)
{
	printf("\t%sb %u,x\n", op, off + 3);
	printf("\t%sa %u,x\n", op2, off + 2);
	printf("\tpshb\n\tpsha");
	printf("\tlda @hireg\n\tldb @hireg+1\n");
	printf("\t%sb %u,x\n", op2, off + 1);
	printf("\t%sa %u,x\n", op2, off);
	printf("\tsta @hireg\n\tstb @hireg+1\n");
	printf("\tpula\n\tpulb\n");
}

void uniop8_on_ptr(const char *op, unsigned off)
{
	printf("\t%s %u,x\n", op, off);
}

void uniop16_on_ptr(const char *op, unsigned off)
{
	printf("\t%s %u,x\n", op, off + 1);
	printf("\t%s %u,x\n", op, off);
}

void uniop32_on_ptr(const char *op, unsigned off)
{
	printf("\t%s %u,x\n", op, off + 3);
	printf("\t%s %u,x\n", op, off + 2);
	printf("\t%s %u,x\n", op, off + 1);
	printf("\t%s %u,x\n", op, off);
}

unsigned make_local_ptr(unsigned off, unsigned rlim)
{
	off += sp;
	/* TODO X tracking and range checks */
	printf("\ttsx\n");
	return off;
}

/* Get pointer to the top of stack. We can optimize this in some cases
   when we track but it will be limited. The 6800 is quite weak on ops
   between register so we sometimes need to build ops against top of stack */
unsigned make_tos_ptr(void)
{
	printf("\ttsx\n");
	return 0;
}

unsigned op8_on_node(struct node *r, const char *op, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		off = make_local_ptr(v + off, 255);
		op8_on_ptr(op, off);
		break;
	case T_CONSTANT:
		printf("\t%sb #%u\n", op, v + off);
		break;
	case T_LBSTORE:
	case T_LBREF:
		printf("\t%sb T%u+%u\n", op, r->val2, v + off);
		break;
	case T_LABEL:
		printf("\t%sb #<T%u+%u\n", op, r->val2, v + off);
		break;
	case T_NSTORE:
	case T_NREF:
		printf("\t%sb _%s+%u\n", op, namestr(r->snum), v + off);
		break;
	case T_NAME:
		printf("\t%sb #<_%s+%u\n", op, namestr(r->snum), v + off);
		break;
	/* case T_RREF:
		printf("\t%sb @__reg%u\n", v);
		break; */
	default:
		return 0;
	}
	return 1;
}

/* Do the low byte first in case it's add adc etc */
unsigned op16_on_node(struct node *r, const char *op, const char *op2, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		off = make_local_ptr(v + off, 254);
		op16_on_ptr(op, op2, off);
		break;
	case T_CONSTANT:
		printf("\t%sa #>%u\n", op, v + off);
		printf("\t%sb #<%u\n", op, v + off);
		break;
	case T_LBSTORE:
	case T_LBREF:
		printf("\t%sb T%u+%u\n", op, r->val2, v + off + 1);
		printf("\t%sa T%u+%u\n", op2, r->val2, v + off);
		break;
	case T_LABEL:
		printf("\t%sb #<T%u+%u\n", op, r->val2, v + off);
		printf("\t%sa #>T%u+%u\n", op2, r->val2, v + off);
		break;
	case T_NSTORE:
	case T_NREF:
		printf("\t%sb _%s+%u\n", op, namestr(r->snum), v + off + 1);
		printf("\t%sa _%s+%u\n", op2, namestr(r->snum), v + off);
		break;
	case T_NAME:
		printf("\t%sb #<_%s+%u\n", op, namestr(r->snum), v + off);
		printf("\t%sa #>_%s+%u\n", op2, namestr(r->snum), v + off);
		break;
	/* case T_RREF:
		printf("\t%sb @__reg%u\n", v);
		break; */
	default:
		return 0;
	}
	return 1;
}

void op32_on_node(struct node *n, const char *op, const char *op2, unsigned off)
{
	/* TODO */
}

unsigned write_op(struct node *r, const char *op, const char *op2, unsigned off)
{
	unsigned s = get_size(r->type);
	if (s == 2)
		return op16_on_node(r, op, op2, off);
	if (s == 1)
		return op8_on_node(r, op, off);
	return 0;
}

unsigned uniop8_on_node(struct node *r, const char *op, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		off = make_local_ptr(v + off, 255);
		uniop8_on_ptr(op, off);
		break;
	case T_LBSTORE:
	case T_LBREF:
		printf("\t%s T%u+%u\n", op, r->val2, v + off);
		break;
	case T_NSTORE:
	case T_NREF:
		printf("\t%s _%s+%u\n", op, namestr(r->snum), v + off);
		break;
	/* case T_RREF:
		printf("\t%sb @__reg%u\n", v);
		break; */
	default:
		return 0;
	}
	return 1;
}

/* Do the low byte first in case it's add adc etc */
unsigned uniop16_on_node(struct node *r, const char *op, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		off = make_local_ptr(v + off, 254);
		uniop16_on_ptr(op, off);
		break;
	case T_LBSTORE:
	case T_LBREF:
		printf("\t%s T%u+%u\n", op, r->val2, v + off + 1);
		printf("\t%s T%u+%u\n", op, r->val2, v + off);
		break;
	case T_NSTORE:
	case T_NREF:
		printf("\t%s _%s+%u\n", op, namestr(r->snum), v + off + 1);
		printf("\t%s _%s+%u\n", op, namestr(r->snum), v + off);
		break;
	/* case T_RREF:
		printf("\t%sb @__reg%u\n", v);
		break; */
	default:
		return 0;
	}
	return 1;
}


unsigned write_uni_op(struct node *n, const char *op, unsigned off)
{
	unsigned s = get_size(n->type);
	if (s == 2)
		return uniop16_on_node(n, op, off);
	if (s == 1)
		return uniop8_on_node(n, op, off);
	return 0;
	
}

void op8_on_tos(const char *op)
{
	unsigned off = make_tos_ptr();
	printf("\t%sb %u,x\n", op, off);
	printf("\tins\n");
}

void op16_on_tos(const char *op, const char *op2)
{
	unsigned off = make_tos_ptr();
	printf("\t%sb %u,x\n", op, off + 1);
	printf("\t%sa %u,x\n", op, off);
	printf("\tins\n");
	printf("\tins\n");
}

unsigned write_tos_op(struct node *n, const char *op, const char *op2)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	if (s == 2)
		op16_on_tos(op, op2);
	else
		op8_on_tos(op);
	return 1;
}

void uniop8_on_tos(const char *op)
{
	unsigned off = make_tos_ptr();
	printf("\t%s %u,x\n", op, off);
	printf("\tins\n");
}

void uniop16_on_tos(const char *op)
{
	unsigned off = make_tos_ptr();
	printf("\t%s %u,x\n", op, off + 1);
	printf("\t%s %u,x\n", op, off);
	printf("\tins\n");
	printf("\tins\n");
}


unsigned write_tos_uniop(struct node *n, const char *op)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	if (s == 2)
		uniop16_on_tos(op);
	else
		uniop8_on_tos(op);
	return 1;
}

/* See if we can easily get the value we want into X rather than D. Must
   not harm D in the process. We can make this smarter over time if needed.
   Might be worth passing if we can trash D as it will help make_local_ptr
   later, and will be true for some load cases */
unsigned can_load_x_with(struct node *r, unsigned off)
{
	switch(r->op) {
	case T_LREF:
	case T_CONSTANT:
	case T_LBREF:
	case T_LABEL:
	case T_NREF:
	case T_NAME:
	case T_RREF:
	case T_LOCAL:
	case T_ARGUMENT:
		return 1;
	}
	return 0;
}

void load_x_with(struct node *r, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_ARGUMENT:
		v += argbase;
	case T_LOCAL:
		v += sp;
		make_local_ptr(v + off, 0);
		break;
	case T_LREF:
		v += sp;
		off = make_local_ptr(v + off, 254);
		printf("\tldx %u,x\n", off);
		break;
	case T_CONSTANT:
		printf("\tldx #%u\n", v + off);
		break;
	case T_LBREF:
		printf("\tldx T%u+%u\n", r->val2, v + off);
		break;
	case T_LABEL:
		printf("\tldx #T%u+%u\n", r->val2, v + off);
		break;
	case T_NREF:
		printf("\tldx _%s+%u\n", namestr(r->snum), v + off);
		break;
	case T_NAME:
		printf("\tldx #<_%s+%u\n", namestr(r->snum), v + off);
		break;
	/* case T_RREF:
		printf("\tldx @__reg%u\n", v);
		break; */
	default:
		error("lxw");
	}
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
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned op = n->op;
	unsigned nt = n->type;

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
	/* For now leave long types out of this */
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

/* Export the C symbol */
void gen_export(const char *name)
{
	printf("	.export _%s\n", name);
}

void gen_segment(unsigned s)
{
	switch(s) {
	case A_CODE:
		printf("\t.text\n");
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
	invalidate_all();
	printf("_%s:\n", name);
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size;
	adjust_s(-size, 0);
	argbase = ARGBASE;
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp)
		error("sp");
	adjust_s(size, (func_flags & F_VOIDRET) ? 0 : 1);
	/* TODO: we are asssuming functions clean up their args if not
	   vararg so this will have to change */
	printf("\trts\n");
	unreachable = 1;
}

void gen_label(const char *tail, unsigned n)
{
	invalidate_all();
	unreachable = 0;
	printf("L%d%s:\n", n, tail);
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
	printf("\tjeq L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjne L%d%s\n", n, tail);
}

void gen_switch(unsigned n, unsigned type)
{
	gen_helpcall(NULL);
	printf("switch");
	helper_type(type, 0);
	printf("\n\t.word Sw%d\n", n);
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

void gen_helpcall(struct node *n)
{
	invalidate_all();
	printf("\tcall __");
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
		/* We are little endian */
		printf("\t.word %d\n", (unsigned) (value & 0xFFFF));
		printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
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


unsigned gen_push(struct node *n)
{
	unsigned size = get_size(n->type);
	/* Our push will put the object on the stack, so account for it */
	sp += get_stack_size(n->type);
	switch(size) {
	case 1:
		printf("\tpshb\n");
		return 1;
	case 2:
		printf("\tpshb\n\tpsha\n");
		return 1;
	case 4:
		printf("\tpshb\n\tpsha\n");
		printf("\tlda @tmp+1\n");
		printf("\tpsha\n");
		printf("\tlda @tmp\n");
		printf("\tpsha\n");
		return 1;
	}
	return 0;
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
	unsigned nr = n->flags & NORETURN;

	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		sp -= r->value;
		if (n->val2) /* Varargs */
			adjust_s(r->value, (func_flags & F_VOIDRET) ? 0 : 1);
		return 1;
	case T_PLUS:
		/* So we can track this common case later */
		if (r->op == T_CONSTANT) {
			if (s == 2) {
				add_d_const(r->value);
				return 1;
			}
			if (s == 1) {
				add_b_const(r->value);
				return 1;
			}
		}
		return write_op(r, "add", "adc", 0);
	case T_MINUS:
		if (r->op == T_CONSTANT) {
			if (s == 2) {
				add_d_const(-r->value);
				return 1;
			}
			if (s == 1) {
				add_b_const(-r->value);
				return 1;
			}
		}
		return write_op(r, "sub", "sbc", 0);
	case T_AND:
		/* TODO: optimize const cases for logic */
		return write_op(r, "and", "and", 0);
	case T_OR:
		/* TODO: optimize const cases for logic */
		return write_op(r, "ora", "ora", 0);
	case T_HAT:
		/* TODO: optimize const cases for logic */
		return write_op(r, "eor", "eor", 0);
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

	switch(n->op) {
	/* Writes of 0 to an object we can use clr for providing the
	   result is not then re-used */
	case T_LSTORE:
	case T_LBSTORE:
	case T_NSTORE:
		/* Optimizations for constants */
		printf(";nr %d\n", nr);
		if (nr && r->op == T_CONSTANT && r->value == 0) {
			printf(";try uniop clr\n");
			if (write_uni_op(n, "clr", 0))
				return 1;
		}
		return 0;
	}
	return 0;
}

/*
 *	Try and build an op where we load X with the pointer,
 *	AB with the data and call a helper. Some of these may also
 *	benefit from inline forms later. 32bit also works as the
 *	value ends up in @hireg/ab which is all safe from the load of
 *	the X pointer.
 */

unsigned do_xeqop(struct node *n, const char *op)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	if (!can_load_x_with(n->left, 0))
		return 0;
	/* Get the value part into AB */
	codegen_lr(n->right);
	/* Load X (lval of the eq op) up (doesn't disturb AB) */
	load_x_with(n->left, 0);
	helper_s(n, op);
	return 1;
}

/*
 *	Allow the code generator to shortcut trees it knows
 */
unsigned gen_shortcut(struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned nr = n->flags & NORETURN;
	/* Don't generate unreachable code */
	if (unreachable)
		return 1;
	switch(n->op) {
	case T_DEREF:
		/* Our right hand side is the thing to deref. See if we can
		   get it into X instead */
		if (can_load_x_with(n->right, 0)) {
			load_x_with(n->right, 0);
			switch(s) {
			case 1:
				printf("\tldab ,x\n");
				return 1;
			case 2:
				printf("\tldaa ,x\n");
				printf("\tldab 1,x\n");
				return 1;
			case 4:
				printf("\tldaa ,x\n");
				printf("\tldab 1,x\n");
				printf("\tstaa @hireg\n");
				printf("\tstab @hireg+1\n");
				printf("\tldaa 2,x\n");
				printf("\tldab 3,x\n");
				return 1;
			default:
				error("sdf");
			}
		}
		return 0;
	case T_EQ:	/* Our left is the address */
		if (can_load_x_with(n->left, 0)) {
			if (n->right->op == T_CONSTANT && nr && n->right->value == 0) {
				/* We can optimize thing = 0 for the case
				   we don't also need the value */
				load_x_with(n->left, 0);
				write_uni_op(n, "clr", 0);
				return 1;
			}
			codegen_lr(n->right);
			load_x_with(n->left, 0);
			switch(s) {
			case 1:
				printf("\tstab ,x\n");
				return 1;
			case 2:
				printf("\tstaa ,x\n");
				printf("\tstabab 1,x\n");
				return 1;
			case 4:
				printf("\tstaa 2,x\n");
				printf("\tstab 3,x\n");
				if (!nr)
					printf("\tpshb\n\tpsha\n");
				printf("\tldaa @hireg\n");
				printf("\tldab @hireg+1\n");
				printf("\tstaa 0,x\n");
				printf("\tstab 1,x\n");
				if (!nr)
					printf("\tpula\n\tpulb\n");
				return 1;
			default:
				error("seqf");
			}
		}
		return 0;
	case T_PLUSEQ:
		return do_xeqop(n, "xpluseq");
	case T_MINUSEQ:
		return do_xeqop(n, "xminuseq");
	case T_PLUSPLUS:
		return do_xeqop(n, "xplusplus");
	case T_MINUSMINUS:
		return do_xeqop(n, "xminusminus");
	case T_STAREQ:
		return do_xeqop(n, "xmuleq");
	case T_SLASHEQ:
		return do_xeqop(n, "xdiveq");
	case T_PERCENTEQ:
		return do_xeqop(n, "xremeq");
	case T_SHLEQ:
		return do_xeqop(n, "xshleq");
	case T_SHREQ:
		return do_xeqop(n, "xshreq");
	case T_ANDEQ:
		return do_xeqop(n, "xandeq");
	case T_OREQ:
		return do_xeqop(n, "xoreq");
	case T_HATEQ:
		return do_xeqop(n, "xhateq");
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned nr = n->flags & NORETURN;
	unsigned v;

	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_stack_size(n->left->type);
	v = n->value;
	switch (n->op) {
	case T_CALLNAME:
		invalidate_all();
		printf("\tcall _%s+%u\n", namestr(n->snum), v);
		return 1;
	case T_DEREF:
		/* TODO: rewrite deref(plus(thing, const)) as DEREFPLUS and combine */
		/* TODO: tracking */
		make_x_d();
		if (s == 1) {
			op8_on_ptr("lda", 0);
			return 1;
		}
		if (s == 2) {
			op16_on_ptr("lda", "lda", 0);
			return 1;
		}
		break;
	case T_EQ:	/* Assign - ToS is address, working value is value */
		/* Again rewriting the plus version would be good */
		if (s == 1) {
			pop_x();
			op8_on_ptr("sta", 0);
			return 1;
		}
		if (s == 2) {
			pop_x();
			op16_on_ptr("sta", "sta", 0);
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
		return 0;
	case T_LABEL:
	case T_NAME:
	case T_LREF:
	case T_NREF:
	case T_LBREF:
		return write_op(n, "lda", "lda", 0);
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
		return write_op(n, "sta", "sta", 0);
	case T_ARGUMENT:
		v += argbase;
	case T_LOCAL:
		/* For v != 0 case it would be more efficient to load
		   const then add @tmp/tmp+1 TODO */
		move_s_d();
		v += sp;
		add_d_const(v);
		return 1;
	/* Single argument ops we can do directly */
	case T_TILDE:
		if (s == 4)
			return 0;
		if (s == 2)
			printf("\tcoma\n");
		printf("\tcomb\n");
		return 1;
	case T_NEGATE:
		if (s == 2) {
			printf("\tcoma\n");
			printf("\tcomb\n");
			add_d_const(1);
			return 1;
		}
		if (s == 1) {
			printf("\tcomb\n");
			add_b_const(1);
			return 1;
		}
		return 0;
	/* Double argument ops we can handle easily */
	case T_PLUS:
		return write_tos_op(n, "add", "adc");
	case T_AND:
		return write_tos_op(n, "and", "and");
	case T_OR:
		return write_tos_op(n, "ora", "ora");
	case T_HAT:
		return write_tos_op(n, "eor", "eot");
	}
	return 0;
}
