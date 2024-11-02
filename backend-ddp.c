/*
 *	The DDP316/516
 *
 *	Word machine with an upward growing work stack and a downward growing
 *	main C stack
 *
 *	Registers
 *	A 		16bit accumulator
 *	B		Can be swapped back and forth with A
 *			We use it for the upper half of a long and scratch
 *	X		Index. Can be loaded/saved but no maths ops
 *	@SP		Page 0 variable holding the stack pointer
 *	@FP		Frame pointer (not useful except in call return)
 *
 *	TODO:
 *	-	All the += *= etc operations
 *	-	Sort out all the bytepointer bits for char type ops
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of words of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned argbase;	/* Argument offset */
static unsigned unreachable;	/* Is code currently unreachable */

#define ARGBASE	2

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */

static unsigned is_bytepointer(unsigned t)
{
	/* A char ** is a word pointer, a char * is a byte pointer */
	if (PTR(t) != 1)
		return 0;
	t = BASE_TYPE(t) & ~UNSIGNED;
	if (t == CCHAR || t == VOID)
		return 1;
	return 0;
}

/*
 *	As a word machine our types matter for pointer conversions
 *	and we cannot blithly throw them away as most byte machines
 *	can. In the Nova case we need to shift it left or right between
 *	byte pointers (void, char *, unsigned char *) and other types
 */
static unsigned pointer_match(unsigned t1, unsigned t2)
{
	if (!PTR(t1) || !PTR(t2))
		return 0;
	if (is_bytepointer(t1) == is_bytepointer(t2))
		return 1;
	return 0;
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
struct node *gen_rewrite_node(register struct node *n)
{
	register struct node *l = n->left;
	register struct node *r = n->right;
	register unsigned op = n->op;
	register unsigned nt = n->type;

	/* TODO: implement derefplus as we can fold small values into
	   a deref pattern */

	/* Rewrite references into a load operation.
	   Char is weird as we then use byte pointers and helpers so avoid */
	if (nt == CSHORT || nt == USHORT || PTR(nt) || nt == CLONG || nt == ULONG || nt == FLOAT) {
		if (op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				/* Offsets are in bytes, we are a word machine */
				if (r->op == T_ARGUMENT)
					r->value += (argbase + frame_len) * 2;
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
					l->value += (argbase + frame_len) * 2;
				squash_left(n, T_LSTORE);
				return n;
			}
		}
	}
	/* Eliminate casts for sign, pointer conversion or same */
	if (op == T_CAST) {
		if (nt == r->type || (nt ^ r->type) == UNSIGNED ||
		 (pointer_match(nt, r->type))) {
			free_node(n);
			r->type = nt;
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

/*
 *	Byte sizes although we are a word machine
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


static void repeated_op(unsigned n, char *op)
{
	while(n--)
		printf("\t%s\n", op);
}

/* Load A with a constant */
void load_a(unsigned n)
{
	n = WORD(n);
	if (n == 0)
		puts("\tcra");
	else if (n == 1)
		puts("\tld1");
	else
		printf("\tlda =%u\n", n);
}

/* Load a word or dword via X */
void load_via_x(struct node *n, int off)
{
	unsigned s = get_size(n->type);
	if (s == 4)
		printf("\tlda @%d,1\n\tiab", off - 1);
	printf("\tlda @%d,1\n", off);
}

/* Store a word or dword via X */
void store_via_x(struct node *n, unsigned off)
{
	unsigned s = get_size(n->type);
	printf("\tsta @%u,1\n", off);
	/* TODO: when NR don't do final iab */
	if (s == 4)
		printf("\tiab\n\tsta @%u1,1\n\tiab\n", off + 1);
}

static unsigned xstate;
static unsigned xstate_sp;
static struct node xnode;

static unsigned xmatch(unsigned op, register struct node *n, unsigned *off)
{
	unsigned v = *off;
	if (op != xnode.op || n->val2 != xnode.val2)
		return 0;
	/* Same object but is it the same position */
	v = *off - xnode.value / 2;	/* Working in words */
	/* Range check. TODO - longs limit is 254 */
	if (v < 0)
		return 0;
	if (v > 255)
		return 0;
	*off = v;
	return 1;
}

/* Try and generate the code for an operation. */
/* TODO: teach range checks about long/float */
/* Not valid for byte pointers */
static unsigned make_x_point(struct node *n, unsigned *off)
{
	unsigned v = n->value / 2;
	*off = 0;

	switch(n->op) {
	case T_LREF:
	case T_LSTORE:
		v += sp;
		if (v > 255)		/* Out of range */
			return 0;
		if (xstate == T_LOCAL) {
			v += sp - xstate_sp;
			if (v > 255)
				return 0;
		} else {
			puts("\tldx @sp");
			xstate = T_LOCAL;
			xstate_sp = sp;
		}
		*off = v;
		return 1;
	case T_LBREF:
	case T_LBSTORE:
		if (!xmatch(T_LABEL, n, &v)) {
			printf("\tldx =T%u+%u\n", n->val2, v);
			xstate = T_LABEL;
			memcpy(&xnode, n, sizeof(struct node));
		}
		return 1;
	case T_NREF:
	case T_NSTORE:
		if (!xmatch(T_NAME, n, &v)) {
			printf("\tldx =_%s+%u\n", namestr(n->val2), v);
			xstate = T_NAME;
			memcpy(&xnode, n, sizeof(struct node));
		}
		return 1;
	}
	return 0;
}

static unsigned can_make_x(struct node *n)
{
	unsigned v = n->value / 2;
	unsigned s = get_size(n->type);

	if (s == 4)
		s = 254;
	else
		s = 255;

	switch(n->op) {
	case T_ARGUMENT:
		v += frame_len;
	case T_LOCAL:
		v += sp;
		if (v > s)		/* Out of range */
			return 0;
		return 1;
	case T_LABEL:
		return 1;
	case T_NAME:
		return 1;
	}
	return 0;
}

/* TODO: byte pointers */
static unsigned make_x(struct node *n, unsigned *off)
{
	unsigned v = n->value / 2;
	unsigned s = get_size(n->type);

	if (s == 4)
		s = 254;
	else
		s = 255;

	*off = 0;

	switch(n->op) {
	case T_ARGUMENT:
		v += frame_len;
	case T_LOCAL:
		v += sp;
		if (v > s)		/* Out of range */
			return 0;
		if (xstate == T_LOCAL) {
			v += sp - xstate_sp;
			if (v > s)
				return 0;
		} else {
			puts("\tldx @sp");
			xstate = T_LOCAL;
			xstate_sp = sp;
		}
		*off = v;
		return 1;
	case T_LABEL:
		if (!xmatch(T_LABEL, n, &v)) {
			printf("\tldx =T%u+%u\n", n->val2, v);
			xstate = T_LABEL;
			memcpy(&xnode, n, sizeof(struct node));
		}
		return 1;
	case T_NAME:
		if (!xmatch(T_NAME, n, &v)) {
			printf("\tldx =_%s+%u\n", namestr(n->val2), v);
			xstate = T_NAME;
			memcpy(&xnode, n, sizeof(struct node));
		}
		return 1;
	}
	return 0;
}

static void modified_x(void)
{
	xstate = 0;
}

static void make_x_a(void)
{
	puts("\tsta @tmp\n\tldx @tmp");
	xstate = 0;
}

/*
 *	Pop is thankfully simple
 */
static void pop_x(void)
{
	puts("\tldx *@sp\n\tirs @sp");	/* Won't skip */
	xstate = 0;
}

static void pop_a(void)
{
	puts("\tlda *@sp\n\tirs @sp");	/* Won't skip */
}

/* TODO: byte pointers */
static unsigned can_make_a(struct node *n)
{
	/* Things we can get into A without using X */
	switch(n->op) {
	case T_CONSTANT:
		return 1;
	case T_ARGUMENT:
	case T_LOCAL:
	case T_LABEL:
	case T_NAME:
	case T_LBREF:
	case T_NREF:
		return 1;
	}
	return 0;
}

/* TODO: byte pointers */
static unsigned make_a(struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned v = n->value / 2;

	switch(n->op) {
	case T_CONSTANT:
		if (s == 4) {
			load_a(n->value >> 16);
			puts("\tiab");
		}
		load_a(n->value);
		return 1;
	case T_ARGUMENT:
		v += frame_len;
	case T_LOCAL:
		v += sp;
		load_a(v);
		puts("\tadd @sp");
		return 1;
	case T_LABEL:
		printf("\tlda =T%u+%u\n", n->val2, v);
		return 1;
	case T_NAME:
		printf("\tlda =_%s+%u\n", namestr(n->val2), v);
		return 1;
	case T_LBREF:
		printf("\tlda T%u+%u\n", n->val2, v);
		return 1;
	case T_NREF:
		printf("\tlda _%s+%u\n", namestr(n->val2), v);
		return 1;
	}
	return 0;
}

/* Try and generate the code for an operation. TODO X tracking */
static unsigned op_direct(const char *op, struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned v = n->value;
	unsigned off;

	/* For now */
	if (s != 2 || is_bytepointer(n->type))
		return 0;

	switch(n->op) {
	case T_CONSTANT:
		printf("\t%s =%u\n", op, v);
		return 1;
	case T_LABEL:
		printf("\t%s =T%u+%u\n", op, n->val2, v / 2);
		return 1;
	case T_NAME:
		printf("\t%s =_%s+%u\n", op, namestr(n->val2), v / 2);
		return 1;
	case T_LBREF:
		printf("\t%s T%u+%u\n", op, n->val2, v / 2);
		return 1;
	case T_NREF:
		printf("\t%s _%s+%u\n", op, namestr(n->val2), v / 2);
		return 1;
	}
	if (make_x_point(n, &off)) {
		printf("\t%s @%u,1\n", op, off);
		return 1;
	}
	return 0;
}

static unsigned op_a_tmp(unsigned s, const char *op, const char *pre)
{
	if (s != 2)
		return 0;
	puts("\tsta @tmp");
	if (pre)
		puts(pre);
	pop_a();
	printf("%s @tmp\n", op);
	return 1;
}

/* Boolify the word in A. We implement CCONLY as indicating
   Z or NZ is all that is needed */
void boolnot(struct node *n)
{
	if (n->flags & ISBOOL)
		puts("\txra =@one\n");
	else {
		/* Either A is 0 and should be 1, or A is NZ and should be 0 */
		puts("\trcb\n\tsnz\n\tscb\n\tcra\n\taca");
		n->flags |= ISBOOL;
	}
}

void boolify(struct node *n)
{
	if (n->flags & ISBOOL)
		return;
	/* 0 is 0, anything else is 1 */
	if (!(n->flags & CCONLY))
		puts("\tsze\n\tld1");
	n->flags |= ISBOOL;
}

/* Turn carry to boolean */
void boolc(struct node *n)
{
	puts("\tcra\n\taca");
	n->flags |= ISBOOL;
}

void boolnc(struct node *n)
{
	puts("\tcra\n\tssc\n\tld1");
	n->flags |= ISBOOL;
}

/* Subtract 16bit const */
void subconst(unsigned v)
{
	v = WORD(v);

	if (v == 0xFFFF)
		puts("\taoa");		/* Same as add one */
	else if (v == 0xFFFE)
		puts("\ta2a");
	else if (v)
		printf("\tsub =%u\n", v);
}

/* Simple word ops on A or BA */
unsigned wordop(unsigned s, const char *op)
{
	if (s == 1)
		return 0;
	if (s == 2)
		printf("\t%s\n", op);
	if (s == 4)
		printf("\tiab\n\t%s\n\tiab\n", op);
	return 1;
}

void gen_prologue(const char *name)
{
	unreachable = 0;
	printf("_%s:\n", name);
	modified_x();
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned aframe)
{
	size = (size + 1) / 2;
	frame_len = size;
	sp = 0;
	if (size)
		printf("\tlda @sp\n\tsub =%u\n\tsta @sp\n", size);
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp)
		error("sp");
	if (unreachable)
		return;
	puts("\tjmp *@pret");
	unreachable = 1;
}

void gen_label(const char *tail, unsigned n)
{
	unreachable = 0;
	modified_x();
	printf("L%d%s:\n", n, tail);
}

unsigned gen_exit(const char *tail, unsigned n)
{
	puts("\tjmp *@pret");
	unreachable = 1;
	return 1;
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\tjmp L%d%s\n", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\tsnz\n");
	printf("\tjmp L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tsze\n");
	printf("\tjmp L%d%s\n", n, tail);
}

void gen_switch(unsigned n, unsigned type)
{
	gen_helpcall(NULL);
	printf("switch");
	helper_type(type, 0);
	printf("\n\t.word Sw%d\n", n);
	unreachable = 1;
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
}

void gen_case(unsigned tag, unsigned entry)
{
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
	modified_x();
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

/* We will need to distinguish fast helpers via @zp and slow
   helpers */
void gen_helpcall(struct node *n)
{
	printf("\tjst *@pcall\n\t.word ");
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
	/* In words */
	printf("\t.ds %d\n", (value + 1) / 2);
}

void gen_text_data(struct node *n)
{
	if (is_bytepointer(n->type))
		printf("\t.byteptr T%d\n", n->val2);
	else
		printf("\t.word T%d\n", n->val2);
}

void gen_literal(unsigned n)
{
	if (n)
		printf("T%d:\n", n);
}

void gen_name(struct node *n)
{
	if (is_bytepointer(n->type))
		printf("\t.byteptr _%s+%d\n", namestr(n->snum), WORD(n->value));
	else
		printf("\t.word _%s+%d\n", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	unsigned v = value;
	if (PTR(type)) {
		if (is_bytepointer(type))
			printf("\t.byteptr %u\n", v);
		else
			printf("\t.word %u\n", v);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		printf("\t.byte %u\n", v & 0xFF);
		break;
	case CSHORT:
	case USHORT:
		printf("\t.word %d\n", v & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		printf("\t.word %d\n", v);
		printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

/* Byte constants for helpers are written as words */
void gen_wvalue(unsigned type, unsigned long value)
{
	unsigned v = value & 0xFFFF;
	if (PTR(type)) {
		if (is_bytepointer(type))
			printf("\t.byteptr %u\n", v);
		else
			printf("\t.word %u\n", v);
		return;
	}
	switch (type) {
		/* Bytes alone are word aligned on the left of the word */
	case CCHAR:
	case UCHAR:
	case CSHORT:
	case USHORT:
		printf("\t.word %u\n", v);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are big endian - software choice */
		printf("\t.word %u\n", (unsigned) ((value >> 16) & 0xFFFF));
		printf("\t.word %u\n", v);
		break;
	default:
		error("unsuported type");
	}
}

static void node_word(struct node *n)
{
	unsigned v = n->value;
	if (n->op == T_CONSTANT) {
		gen_wvalue(n->type, n->value);
		return;
	}
	if (is_bytepointer(n->type))
		printf("\t.byteptr ");
	else
		printf("\t.word ");
	if (n->op == T_NAME)
		printf("_%s+%u\n", namestr(n->snum), v);
	else if (n->op == T_LABEL)
		printf("T%u+%u\n", n->val2, v);
	else
		error("nw");
}

static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned ls = get_size(lt);

	if (PTR(lt) && PTR(rt)) {
		if (is_bytepointer(lt) && !is_bytepointer(rt)) {
			puts("\tlgl 1");
			return 1;
		}
		if (is_bytepointer(rt) && !is_bytepointer(lt)) {
			puts("\tlgr 1");
			return 1;
		}
	}
	/* We treat all pointers cast to non pointer types as byte pointers. We have a max
	   32KW address space so there is no wrapping problem and this makes more bytedamaged
	   code work properly out of the box */
	if (PTR(rt) && !PTR(lt) && !is_bytepointer(rt))
		puts("\tlgl 1");

	/* From this point on treat pointers as USHORT */
	if (PTR(lt))
		lt = USHORT;
	if (PTR(rt))
		rt = USHORT;
	/* If only the sign differs we don't need to do any work */
	if (((lt ^ rt) & ~UNSIGNED) == 0)
		return 1;

	if (lt == FLOAT || rt == FLOAT)
		return 0;

	/* Do unsigned expansion with clears */
	if (rt & UNSIGNED) {
		if (rt == UCHAR)
			puts("\tcal");
		if (ls == 4)
			puts("\tiab\n\tcra\n\tiab");
		return 1;
	}
	/* Signed is more interesting */
	if (rt == CCHAR) {
		if (ls == 4)
			puts("\trtl\n\tcpy\n\tcal\n\tiab\n\tcm1\n\tcma\n\tiab\n\tsrc\n\tsrc\n\tand = 0xFF00");
		else
			puts("\trtl\n\tcpy\t\n\tcal\n\tsrc\n\tand =0xFF00");
		return 1;
	}
	puts("\tcpy\n\tiab\n\tcm1\ncma\n\tiab");
	return 1;
}

static unsigned gen_shift(struct node *n, const char *o16u , const char *o16s, const char *o32u, const char *o32s)
{
	unsigned u = n->type & UNSIGNED;
	unsigned s = get_size(n->type);
	const char *p = o16s;

	if (s == 4) {
		if (u)
			p = o32u;
		else
			p = o32s;
	} else if (u)
		p = o16u;
	if (p == NULL)
		return 0;

	/* Patch the instruction into tmp2 and run @tmp. tmp2 is followed
	   by a jmp *@tmp so we basically build an instruction to run */
	puts("\tand =31\n\tadd =OP%s\n\tsta @tmp2");
	pop_a();
	if (s == 4) {
		puts("\tiab");
		pop_a();
	}
	/* We need the A and B swapped for this bit */
	puts("\tjst @tmp");	/* Now the instruction to execute */
	if (s == 4)
		puts("\tiab\n");
	return 1;
}

/* Does the simpler ops we can do. If flip is set we need to do the operation
   as [X] op A */
static unsigned gen_eqop(struct node *n, const char *op, unsigned flip)
{
	unsigned s = get_size(n->type);
	if (s != 2)	/* no char long or float */
		return 0;
	/* At this point the right is in A and the ptr is on stack */
	pop_x();
	/* X is now the pointer */
	if (flip)
		puts("\tima @0,1");	/* Swap the values over */
	printf("\t%s @0,1\n", op);	/* Do the operation */
	store_via_x(n, 0);
	return 1;
}

static unsigned gen_eqshortcut(struct node *n, const char *op, unsigned flip)
{
	unsigned s = get_size(n->type);
	unsigned off;
	if (s != 2)	/* no char long or float */
		return 0;
	/* Left is the pointer, right the value */
	if (!can_make_x(n->left))
		return 0;
	codegen_lr(n->right);		/* Get the value to use for modification */
	make_x(n->left, &off);		/* Create the pointer */
	if (flip)
		printf("\tima @%u,1\n", off);	/* Swap the values over */
	printf("\t%s @%u,1\n", op, off);	/* Do the operation */
	store_via_x(n, off);
	return 1;
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

/*
 *	Stacking values and arguments is messy. Most of our work goes into
 *	not doing it in the first place.
 */
unsigned gen_push(struct node *n)
{
	unsigned s = get_stack_size(n->type);
	/* Our push will put the object on the stack, so account for it */
	sp += s / 2;
	if (s == 4)
		puts("\tjst @push4");
	else
		puts("\tjst @push");
	return 1;
}


/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 */
unsigned gen_direct(struct node *n)
{
	struct node *r = n->right;
	unsigned s = get_size(n->type);
	unsigned off;
	unsigned v;

	if (r)
		v = r->value;

	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		v /= 2;
		sp -= v;
		if (v <= 4)
			repeated_op(v, "irs @sp");
		else {
			if (n->type != VOID)
				puts("\tsta @tmp");
			printf("\tlda @sp\n\tadd =%u\n\tsta @sp\n", v);
			if (n->type != VOID)
				puts("\tlda @tmp");
		}
		return 1;
	case T_PLUS:
		if (op_direct("add", r))
			return 1;
		break;
	/* TODO: const optim */
	case T_AND:
		if (op_direct("and", r))
			return 1;
		break;
	case T_HAT:
		if (op_direct("xor", r))
			return 1;
		break;
	case T_STAR:
		if ((n->type & UNSIGNED) && op_direct("mpy", r))
			return 1;
	/* Stuff we need to do in steps but can do if we can point X properly */
		break;
	/* TODO: const forms */
	case T_OR:
		if (s == 2 && make_x_point(r, &off)) {
			printf("\tsta @tmp\n\txra @%u,1\n\tima @tmp\n\tana @%u,1\n\tadd @tmp\n", off, off);
			return 1;
		}
		break;
	case T_SLASH:
		if (s == 2 && (n->type & UNSIGNED)) {
			if (r->type == T_CONSTANT) {
				printf("\tiab\n\tcra\n\tiab\tdiv =%u\n", v);
				return 1;
			}
			if (make_x_point(r, &off)) {
				printf("\tiab\n\tcra\n\tiab\n\tdiv @%u,1\n", off);
				return 1;
			}
		}
		break;
	case T_PERCENT:
		if (s == 2 && (n->type & UNSIGNED)) {
			if (r->type == T_CONSTANT) {
				printf("\tiab\n\tcra\n\tiab\tdiv =%u\n\tiab\n", v);
				return 1;
			}
			if (make_x_point(r, &off)) {
				printf("\tiab\n\tcra\n\tiab\n\tdiv @%u,1\n\tiab\n", off);
				return 1;
			}
		}
		break;
	/* Constant shifts */
	case T_LTLT:
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (s <= 2)
				printf("\tlgl %u\n", v & 15);
			else
				printf("\tiab\n\tlll %u\n\tiab\n", v & 15);
			return 1;
		}
		return 0;
	/* Try and do the shifts we know how to */
	case T_GTGT:
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (n->type & UNSIGNED) {
				if (s <= 2)
					printf("\tlgl %u\n", v & 15);
				else
					printf("\tiab\n\tlrl %u\n\tiab\n", v & 15);
			} else {
				if (s == 2)
					printf("\tars %u\n", v & 15);
				else	/* TODO: we have no useful ARS for long */
					return 0;
			}
			return 1;
		}
		return 0;

	/* Condition codes */
	case T_EQEQ:
		if (s == 2) {
			if (r->op == T_CONSTANT) {
				subconst(v);
				boolnot(n);
				return 1;
			}
			if (make_x_point(r, &off)) {
				printf("\tsub %u,1\n", off);
				boolnot(n);
				return 1;
			}
		}
		break;
	case T_BANGEQ:
		if (s == 2) {
			if (r->op == T_CONSTANT) {
				subconst(v);
				boolify(n);
				return 1;
			}
			if (make_x_point(r, &off)) {
				printf("\tsub %u,1\n", off);
				boolify(n);
				return 1;
			}
		}
		break;
	case T_LT:
		if (s == 2) {
			if (r->op == T_CONSTANT && v == 0) {
				if (n->type & UNSIGNED) {
					load_a(0);
					return 1;
				}
				/* Copy the sign to carry */
				puts("\tcpy");
				boolc(n);
				return 1;
			}
			/* A < B is A <= B - 1 and we know B is not 0 */
			if (r->op == T_CONSTANT && (n->type & UNSIGNED)) {
				subconst(v - 1);
				boolc(n);
				return 1;
			}
		}
		break;
	case T_GTEQ:
		if (s == 2) {
			if (r->op == T_CONSTANT && v == 0) {
				if (n->type & UNSIGNED) {
					load_a(1);
					return 1;
				}
				/* Copy the sign to carry */
				puts("\tcpy");
				boolnc(n);
				return 1;
			}
			/* A >= B is A > B - 1 and we know B is not 0 */
			if (r->op == T_CONSTANT && (n->type & UNSIGNED)) {
				subconst(v - 1);
				boolnc(n);
				return 1;
			}
		}
		break;
	case T_LTEQ:
		if (s == 2 && (n->type & UNSIGNED)) {
			if (r->op == T_CONSTANT) {
				if (v == 0) {
					/* Unsigned <= 0 is == 0 */
					boolnot(n);
					return 1;
				}
				subconst(v);
				boolc(n);
				return 1;
			}
			if (make_x_point(r, &off)) {
				printf("\tsub %u,1\n", off);
				boolc(n);
				return 1;
			}
		}
		break;
	case T_GT:
		if (s == 2 && (n->type & UNSIGNED)) {
			if (r->op == T_CONSTANT) {
				if (v == 0) {
					/* Unsigned >= 0 is != 0 */
					boolify(n);
					return 1;
				}
				subconst(v);
				boolnc(n);
				return 1;
			}
			if (make_x_point(r, &off)) {
				printf("\tsub %u,1\n", off);
				boolnc(n);
				return 1;
			}
		}
		break;
	/* TODO: eqops on directly loadable values */
#if 0
	/* Need to think about the X aspect and also a shortcut
	   form so we can make_x / do op on x for the simple cases */
	case T_PLUSEQ:
		return gen_eqop(n, "add", 0);
	case T_MINUSEQ:
		return gen_eqop(n, "sub", 1);
	case T_STAREQ:
		if (n->type & UNSIGNED)
			return gen_eqop(n, "mpy", 0);
		return 0;
	case T_ANDEQ:
		return gen_eqop(n, "and", 0);
	case T_HATEQ:
		return gen_eqop(n, "xor", 0);
#endif
	/* The harder ones left to do */
	case T_OREQ:
	case T_SLASHEQ:
	case T_PERCENTEQ:
	case T_SHLEQ:
	case T_SHREQ:
	/* These are harder */
	case T_PLUSPLUS:
	case T_MINUSMINUS:
		return 0;
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

/*
 *	Allow the code generator to shortcut trees it knows
 */
unsigned gen_shortcut(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned nr = n->flags & NORETURN;
	unsigned v;
	unsigned off;
	unsigned s = get_size(n->type);

	if (r)
		v = r->value;

	/* Unreachable code we can shortcut into nothing ..bye.. */
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
	/* assignment we try and re-arrange as our stack costs are so high */
	case T_EQ:
		/* If the pointer side is simple then get the value and store it */
		if (can_make_x(l)) {
			codegen_lr(r);
			make_x(l, &off);
			store_via_x(n, off);
			return 1;
		}
		if (can_make_a(r)) {
			codegen_lr(l);
			make_x_a();
			make_a(r);
			store_via_x(n, 0);
			return 1;
		}
		return 0;
	case T_DEREF:
		/* Shortcut easy long loads via X */
		if (s == 4 && can_make_x(r)) {
			make_x(r, &off);
			load_via_x(n, off);
			return 1;
		}
		return 0;
	case T_MINUSMINUS:
		if (s != 2 || !can_make_x(l))
			return 0;
		codegen_lr(r);
		/* So we can use addition just complement it */
		printf("\ttca\n");
		/* A is now the value */
		make_x(l, &off);
		/* X is now the pointer */
		if (!nr)
			printf("\tima %u,1\n\tsta @tmp\n", off);
		printf("\tadd %u,1\n", off);
		printf("\tsta %u,1\n", off);
		if (!nr)
			puts("\tlda @tmp");
		return 1;
	case T_PLUSPLUS:
		if (s != 2 || !can_make_x(l))
			return 0;
		/* We can make X our working value without mashing A */
		if (nr && r->op == T_CONSTANT && v <= 4) {
			make_x(l, &off);
			while(v--)
				printf("\tirs %u,x\n\tnop\n", off);
			return 1;
		}
		codegen_lr(r);
		/* A is now the value */
		make_x(l, &off);
		/* X is now the pointer */
		if (!nr)
			printf("\tima %u,1\n\tsta @tmp\n", off);
		printf("\tadd %u,1\n", off);
		printf("\tsta %u,1\n", off);
		if (!nr)
			puts("\tlda @tmp");
		return 1;
	case T_MINUSEQ:
		if (s != 2 || !can_make_x(l))
			return 0;
		codegen_lr(r);
		/* So we can use addition just complement it */
		printf("\ttca\n");
		/* A is now the value */
		make_x(l, &off);
		/* X is now the pointer */
		printf("\tadd %u,1\n", off);
		printf("\tsta %u,1\n", off);
		return 1;
	case T_PLUSEQ:
		if (s != 2 || !can_make_x(l))
			return 0;
		/* We can make X our working value without mashing A */
		if (nr && r->op == T_CONSTANT && v <= 4) {
			make_x(l, &off);
			while(v--)
				printf("\tirs %u,x\n\tnop\n", off);
			return 1;
		}
		codegen_lr(r);
		/* A is now the value */
		make_x(l, &off);
		/* X is now the pointer */
		printf("\tadd %u,1\n", off);
		printf("\tsta %u,1\n", off);
		return 1;
	case T_STAREQ:
		if (n->type & UNSIGNED)
			return gen_eqshortcut(n, "mpy", 0);
		return 0;
	case T_ANDEQ:
		return gen_eqshortcut(n, "and", 0);
	case T_HATEQ:
		return gen_eqshortcut(n, "xor", 0);
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned v = n->value;
	unsigned s = get_size(n->type);
	unsigned off;

	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_FUNCCALL && n->op != T_CALLNAME)
		sp -= get_stack_size(n->left->type) / 2;
	switch(n->op) {
	case T_CALLNAME:
		printf("\tjst @pcall\n\t.word _%s+%u\n", namestr(n->snum), v);
		return 1;
	/* TODO: T_FUNCCALL */
	case T_CAST:
		return gen_cast(n);
	case T_CONSTANT:
	case T_LOCAL:
	case T_LABEL:
	case T_NAME:
		make_a(n);
		return 1;
	case T_LREF:
	case T_NREF:
	case T_LBREF:
		/* TODO: track X use and suppress reloads */
		/* TODO: how to handle out of page range loads */
		make_x_point(n, &off);
		load_via_x(n, off);
		return 1;
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
		make_x_point(n, &off);
		store_via_x(n, off);
		return 1;
	case T_EQ:
		pop_x();
		store_via_x(n, 0);
		return 1;
	case T_DEREF:
		if (s == 4) {
			make_x_a();
			load_via_x(n, 0);
			return 1;
		}
		if (s == 2) {
			puts("\tsta @tmp\n\tlda *@tmp");
			return 1;
		}
		break;
	case T_BOOL:
		if (s == 2) {
			boolify(n);
			return 1;
		}
		break;
	case T_BANGEQ:
		if (s == 2) {
			boolnot(n);
			return 1;
		}
		break;
	/* Simple ops */
	case T_TILDE:
		return wordop(s, "cma");
	case T_NEGATE:
		return wordop(s, "tca");
	/* Unstacking ops we can do easily */
	case T_PLUS:
		return op_a_tmp(s, "add", NULL);
	case T_MINUS:
		return op_a_tmp(s, "sub", NULL);
	case T_STAR:
		if (n->type & UNSIGNED)
			return op_a_tmp(s, "mpy", NULL);
		break;
	case T_SLASH:
		if (n->type & UNSIGNED)
			return op_a_tmp(s, "div", "\tclra\n\tiab\n");
		break;
	case T_PERCENT:
		if (n->type & UNSIGNED) {
			if (op_a_tmp(s, "div", "\tclra\n\tiab\n")) {
				puts("\tiab");
				return 1;
			}
		}
	case T_AND:
		return op_a_tmp(s, "and", NULL);
	case T_HAT:
		return op_a_tmp(s, "xor", NULL);
	/* OR is harder */
	case T_OR:
		if (s == 2) {
			puts("\tsta @tmp2");
			pop_a();
			puts("\txra @tmp\n\tima @tmp2\n\tana @tmp\n\tadd @tmp2");
			return 1;
		}
		break;
	case T_LTLT:
		return gen_shift(n, "lgl", "lgl", "lll", "lll");
	case T_GTGT:
		return gen_shift(n, "ars", "lgr", "lrl", NULL);
	/* We do the eq ops as a load/op/store pattern in general */
	case T_PLUSEQ:
		return gen_eqop(n, "add", 0);
	case T_MINUSEQ:
		return gen_eqop(n, "sub", 1);
	case T_STAREQ:
		if (n->type & UNSIGNED)
			return gen_eqop(n, "mpy", 0);
		return 0;
	case T_ANDEQ:
		return gen_eqop(n, "and", 0);
	case T_HATEQ:
		return gen_eqop(n, "xor", 0);
	/* The harder ones left to do */
	case T_OREQ:
	case T_SLASHEQ:
	case T_PERCENTEQ:
	case T_SHLEQ:
	case T_SHREQ:
	/* These are harder */
	case T_PLUSPLUS:
	case T_MINUSMINUS:
		return 0;
	}
	return 0;
}
