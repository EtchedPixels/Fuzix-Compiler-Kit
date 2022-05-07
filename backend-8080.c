#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

/* FIXME: wire options in backend to this */
static int cpu = 8085;

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */

#define T_NVAL		(T_USER)
#define T_CALLNAME	(T_USER+1)

/*
 *	Our chance to do tree rewriting. We don't do much for the 8080
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *r = n->right;
	/* Rewrite references into a load operation */
	if (n->type == CINT || n->type == UINT || PTR(n->type)) {
		if (n->op == T_DEREF) {
#if 0
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				n->op = T_LREF;
				n->snum = r->snum;
				n->value = r->value;
				free_node(r);
				n->right = NULL;
				return n;
			}
#endif
			if (r->op == T_NAME) {
				n->op = T_NVAL;
				n->snum = r->snum;
				n->value = r->value;
				free_node(r);
				n->right = NULL;
				return n;
			}
		}
	}
	/* Rewrite function call of a name into a new node so we can
	   turn it easily into call xyz */
	if (n->op == T_FUNCCALL && r->op == T_NAME) {
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

/* Generate the function prologue - may want to defer this until
   gen_frame for the most part */
void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
}

/* Generate the stack frame */
/* TODO: defer this to statements so we can ld/push initializers */
void gen_frame(unsigned size)
{
	frame_len = size;
	sp += size;
	printf(";make frame now sp %d\n", sp);

	printf("; frame %d\n", size);
	if (size > 10) {
		printf("\tlxi h,%d\n", -size);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		return;
	}
	if (size & 1) {
		printf("\tdcr sp\n");
		size--;
	}
	while(size) {
		printf("\tpush h\n");
		size -= 2;
	}
}

void gen_epilogue(unsigned size)
{
	printf("; eframe %d sp %d\n", size, sp);
	if (sp != size)
		error("sp");
	/* Return in DE so we can adjust the stack more easily. The caller
	   will xchg it back after cleanup */
	sp -= size;
	if (cpu == 8085 && size <= 255 && size > 4) {
		printf("\tldsi %d\n", size);
		printf("\txchg\n");
		printf("\tsphl\n");
		printf("\tret\n");
		return;
	}
	printf("\txchg\n");
	if (size > 10) {
		printf("\tlxi h,0x%x\n", (uint16_t) - size);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		printf("\tret\n");
		return;
	}
	if (size % 1) {
		printf("\tinr sp\n");
		size--;
	}
	while (size) {
		printf("\tpop b\n");
		size -= 2;
	}
	printf("\tret\n");
}

void gen_label(const char *tail, unsigned n)
{
	printf("L%d%s:\n", n, tail);
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\tjmp L%d%s\n", n, tail);
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\tjz L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjnz L%d%s\n", n, tail);
}

void gen_helpcall(void)
{
	printf("\tcall ");
}

/* This is still being worked on */
void gen_switch_begin(unsigned n, unsigned type)
{
}

void gen_switch(unsigned n)
{
}

void gen_case(unsigned type)
{
}

/* TODO: Need to pass alignment */
void gen_data(const char *name)
{
	printf("\t.data\n");
	printf("_%s:\n", name);
}

/* TODO: Need to pass alignment */
void gen_bss(const char *name)
{
	printf("\t.data\n");
	printf("_%s:\n", name);
}

void gen_code(void)
{
	printf("\t.code\n");
}

void gen_space(unsigned value)
{
	printf("\t.ds %d\n", value);
}

void gen_text_label(unsigned n)
{
	printf("\t.word T%d\n", n);
}

void gen_name(struct node *n)
{
	printf("\t.word _%s+%d\n", namestr(n->snum), n->value);
}

void gen_value(unsigned type, unsigned long value)
{
	if (PTR(type)) {
		printf(".word %u\n", (unsigned) value);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		printf(".byte %u\n", (unsigned) value & 0xFF);
		break;
	case CINT:
	case UINT:
		printf(".word %d\n", (unsigned) value & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
		/* We are little endian */
		printf(".word %d\n", (unsigned) (value & 0xFFFF));
		printf(".word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
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

/*
 *	Try and generate shorter code for stuff we can directly access
 */

/*
 *	Return 1 if the node can be turned into direct access. The VOID check
 *	is a special case we need to handle stack clean up of void functions.
 */
static unsigned access_direct(struct node *n)
{
	/* We can direct access integer or smaller types that are constants
	   global/static or string labels */
	if (n->op != T_CONSTANT && n->op != T_NAME && n->op != T_LABEL && n->op != T_NVAL)
		return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CINT)
		return 0;
	return 1;
}

/*
 *	Get something that passed the access_direct check into de. Could
 *	we merge this with the similar hl one in the main table ?
 */

static void load_de_with(struct node *n)
{
	switch(n->op) {
	case T_NAME:
		printf("\tlxi d, _%s+%d\n", namestr(n->snum), n->value);
		break;
	case T_LABEL:
		printf("\tlxi d, T%d\n", n->value);
		break;
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		printf("\tlxi d, %d\n", n->value);
		break;
	case T_NVAL:
		/* We know it is int or pointer */
		printf("\txchg\n");
		printf("\tlhld (_%s+%d)\n", namestr(n->snum), n->value);
		printf("\txchg\n");
		break;
	default:
		error("ldew");
	}
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 *
 *	The 8080 is pretty basic so there isn't a lot we turn around here. As
 *	proof of concept we deal with the add case. Other processors may be
 *	able to handle a lot more.
 *
 *	If your processor is good at subtracts you may also want to rewrite
 *	constant on the left subtracts in the rewrite rules into some kind of
 *	rsub operator.
 */
unsigned gen_direct(struct node *n)
{
	/* We only deal with simple cases for now */
	if (n->right && !access_direct(n->right))
		return 0;

	switch (n->op) {
	case T_CLEANUP:
		/* CLEANUP is special and needs to be handled directly */
		/* TODO - optimizations like inx sp and pops */
		if (n->right->value) {
			printf("\tlxi h, %d\n", n->right->value);
			printf("\tdad sp\n");
			printf("\tsphl\n");
			sp -= n->right->value;
		}
		/* The call return is in DE at this point due to stack juggles
		   so put it into HL */
		printf("\txchg\n");
		return 1;
	case T_PLUS:
		/* LHS is in HL at the moment, end up with the result in HL */
		load_de_with(n->right);
		printf("\tdad d\n");
		return 1;
	}
	return 0;
}

static unsigned get_size(unsigned t)
{
	if (PTR(t))
		return 2;
	if (t == CINT || t == UINT)
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

/* Stack the node which is currently in the working register */
unsigned gen_push(struct node *n)
{
	unsigned size = get_stack_size(n->type);

	/* Our push will put the object on the stack, so account for it */
	sp += size;

	switch(size) {
	case 2:
		printf("\tpush h\n");
		return 1;
	case 4:
		printf("\txchg\n\tlhld hireg\n\tpush h\n\tpush d\n");
		return 1;
	default:
		return 0;
	}
}


unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);

	/* An operation with a left hand node will have the left stacked
	   and the operation will consume it so adjust the stack.

	   The exception to this is comma and the function call nodes
	   as we leave the arguments pushed for the function call */

	if (n->left && n->op != T_COMMA && n->op != T_CALLNAME)
		sp -= get_size(n->left->type);

	switch (n->op) {
		/* Load from a name */
	case T_NVAL:
		printf("\tlhld _%s+%d\n", namestr(n->snum), n->value);
		return 1;
		/* Call a function by name */
	case T_CALLNAME:
		printf("\tcall _%s+%d\n", namestr(n->snum), n->value);
		return 1;
	case T_EQ:
		if (size == 2) {
			if (cpu == 8085)
				printf("\txchg\n\tpop h\n\tshlx\n");
			else
				printf("\txchg\n\tpop h\n\tmov m,e\n\tinx h\n\tmov m,d");
			return 1;
		}
		break;
	case T_DEREF:
		if (size == 2) {
			if (cpu == 8085)
				printf("\tlhlx\n");
			else
				printf("\tmov e,m\n\tinx h\n\tmov d,m\n\txchg\n");
			return 1;
		}
		break;
	case T_FUNCCALL:
		printf("\tcall callhl\n");
		return 1;
	case T_LABEL:
		/* Used for const strings */
		printf("\tlxi h,T%d\n", n->value);
		return 1;
	case T_CONSTANT:
		switch(n->type) {
		case CLONG:
		case ULONG:
			printf("lxi h,%u\n", ((n->value >> 16) & 0xFFFF));
			printf("shld hireg\n");
		/* For readability */
		case UCHAR:
		case UINT:
			printf("\tlxi h,%u\n", (n->value & 0xFFFF));
			return 1;
		case CCHAR:
		case CINT:
			printf("\tlxi h,%d\n", (n->value & 0xFFFF));
			return 1;
		}
		break;
	case T_NAME:
		printf("\tlxi h,");
		printf("_%s+%d\n", namestr(n->snum), n->value);
		return 1;
	case T_LOCAL:
		/* We already adjusted sp so allow for this */
		if (cpu == 8085 && n->value + sp + size <= 255) {
			printf("\tldsi %d\n", n->value + sp + size);
		} else {
			printf("\tlxi h,%d\n", n->value + sp + size);
			printf("\tdad sp\n");
		}
		return 1;
	case T_ARGUMENT:
		/* We already adjusted sp so allow for this */
		if (cpu == 8085 && n->value + 2 + frame_len + sp + size <= 255) {
			printf("ldsi %d\n", n->value + sp + size);
		} else {
			printf("\tlxi h,%d\n", n->value + size + 2 + frame_len + sp);
			printf("\tdad sp\n");
		}
		return 1;
	}
	return 0;
}
