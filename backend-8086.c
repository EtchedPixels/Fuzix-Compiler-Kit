#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned argbase = 4;	/* Will vary once we add reg vars */
static unsigned unreachable;

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
	fprintf(stderr, "type %x\n", t);
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

#define T_CALLNAME	(T_USER+0)		/* Function call by name */
#define T_NREF		(T_USER+1)		/* Load of C global/static */
#define T_LBREF		(T_USER+2)		/* Ditto for labelled strings or local static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_NSTORE	(T_USER+4)		/* Store to a C global/static */
#define T_LBSTORE	(T_USER+5)
#define T_LSTORE	(T_USER+6)

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
 *	Our chance to do tree rewriting. We don't do much
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned op = n->op;

	if (op == T_DEREF) {
		if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
			if (r->op == T_ARGUMENT)
				r->value += argbase + frame_len;
			squash_right(n, T_LREF);
			return n;
		}
#if 0		
		if (r->op == T_REG) {
			squash_right(n, T_RREF);
			return n;
		}
#endif		
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
#if 0		
		if (l->op == T_REG) {
			squash_left(n, T_RSTORE);
			return n;
		}
#endif		
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

void gen_prologue(const char *name)
{
	unreachable = 0;
	printf("_%s:\n", name);
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size;
	/* TODO: push si/di if reg args */
	printf("\tpush bp\n");
	printf("\tmov bp,sp\n");
	if (size)
		printf("\tsub sp,%d\n", size);
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	/* TODO: pop si/di if reg args */
	if (!unreachable) {
		printf("\tmov sp,bp\n");
		printf("\tpop bp\n");
		printf("\tret\n");
	}
	unreachable = 1;
}

void gen_label(const char *tail, unsigned n)
{
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
	printf("\tjz L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjnz L%d%s\n", n, tail);
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
	unsigned s = get_stack_size(n->type);
	if (s == 4)
		printf("\npush dx\n");
	printf("\tpush ax\n");
	return 1;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 */
unsigned gen_direct(struct node *n)
{
	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		if (n->right->value)
			printf("\tadd sp,%u\n", (unsigned)(n->right->value & 0xFFFF));
		return 1;
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
	if (unreachable)
		return 1;
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned nr = n->flags & NORETURN;
	unsigned v = n->value;
	const char *name;

	switch(n->op) {
	case T_CALLNAME:
		printf("\tcall_%s+%u\n", namestr(n->snum), v);
		return 1;
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		if (nr)
			return 1;
		printf("\tlea ax,%u[bp]\n", v);
		return 1;
	case T_LABEL:
		if (nr)
			return 1;
		printf("\tmov ax,T%u+%u\n", n->val2, v);
		return 1;
	case T_NAME:
		if (nr)
			return 1;
		printf("\tmov ax,_%s+%u\n", namestr(n->snum), v);
		return 1;
	case T_NREF:
		name = namestr(n->snum);
		switch(size) {
		case 1:
			printf("\tmov al,_%s+%u\n", name, v);
			return 1;
		case 2:
			printf("\tmov ax,_%s+%u\n", name, v);
			return 1;
		case 4:
			printf("\tmov ax,_%s+%u\n", name, v);
			printf("\tmov dx,_%s+%u\n", name, v +2);
			return 1;
		}
		break;
	case T_LBREF:
		switch(size) {
		case 1:
			printf("\tmov al,T%u+%u\n", n->val2, v);
			return 1;
		case 2:
			printf("\tmov ax,T%u+%u\n", n->val2, v);
			return 1;
		case 4:
			printf("\tmov ax,T%u+%u\n", n->val2, v);
			printf("\tmov dx,T%u+%u\n", n->val2, v +2);
			return 1;
		}
		break;
	case T_LREF:
		switch(size) {
		case 1:
			printf("\tmov al,%u[bp]\n", v);
			return 1;
		case 2:
			printf("\tmov ax,%u[bp]\n", v);
			return 1;
		case 4:
			printf("\tmov ax,%u[bp]\n", v);
			printf("\tmov dx,%u[bp]\n", v + 2);
			return 1;
		}
		break;
	case T_NSTORE:
		name = namestr(n->snum);
		switch(size) {
		case 1:
			printf("\tmov _%s+%u, al\n", name, v);
			return 1;
		case 2:
			printf("\tmov _%s+%u, ax\n", name, v);
			return 1;
		case 4:
			printf("\tmov _%s+%u, ax\n", name, v);
			printf("\tmov _%s+%u, dx\n", name, v + 2);
			return 1;
		}
		break;
	case T_LBSTORE:
		switch(size) {
		case 1:
			printf("\tmov T%u+%u, al\n", n->val2, v);
			return 1;
		case 2:
			printf("\tmov T%u+%u, ax\n", n->val2, v);
			return 1;
		case 4:
			printf("\tmov T%u+%u, ax\n", n->val2, v);
			printf("\tmov T%u+%u, dx\n", n->val2, v + 2);
			return 1;
		}
		break;
	case T_LSTORE:
		switch(size) {
		case 1:
			printf("\tmov %u[bp],al\n", v);
			return 1;
		case 2:
			printf("\tmov %u[bp],ax\n", v);
			return 1;
		case 4:
			printf("\tmov %u[bp],ax\n", v);
			printf("\tmov %u[bp],dx\n", v + 2);
			return 1;
		}
		break;
	case T_CONSTANT:
		if (nr)
			return 1;
		switch(size) {
		case 1:
			printf("\tmov al,%u\n", v & 0xFF);
			break;
		case 4:
			printf("\tmov dx,%u\n", (unsigned)((n->value >> 16) & 0xFFFF));
		case 2:
			printf("\tmov ax,%u\n", v);
		}
		return 1;
	case T_EQ:
		/* This will not get used much once all the optimziations are in,
		   just for complex expressions each side */
		printf("\tmov bx,ax\n");
		switch(size) {
		case 1:
			printf("\tmov [bx],al\n");
			break;
		case 2:
			printf("\tpop ax\n");
			printf("\tmov [bx],ax\n");
			break;
		case 4:
			printf("\tpop ax\n");
			printf("\tpop dx\n");
			printf("\tmov [bx],ax\n");
			printf("\tmov 2[bx],dx\n");
			break;
		}
		return 1;
	case T_DEREF:
		printf("\tmov bx,ax\n");
		switch(size) {
		case 1:
			printf("\tmov al,[bx]\n");
			break;
		case 2:
			printf("\tmov ax,[bx]\n");
			break;
		case 4:
			printf("\tmov ax,[bx]\n");
			printf("\tmov ax,2[bx]\n");
			break;
		}
		return 1;
	case T_PLUS:
		printf("\tpop bx\n");
		switch(size) {
		case 1:
			printf("\tadd al,bl\n");
			break;
		case 2:
			printf("\tadd ax,bx\n");
			break;
		case 4:
			printf("\tadd ax,bx\n");
			printf("\tpop bx\n");
			printf("\tadc dx,bx\n");
			break;
		}
		return 1;
	}
	return 0;
}
