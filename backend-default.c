#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */

/*
 *	Our chance to do tree rewriting. We don't do much for the 8080
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	return n;
}

/* Export the C symbol */
void gen_export(const char *name)
{
	printf("	.export _%s\n", name);
}

void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
}

/* Generate the stack frame */
void gen_frame(unsigned size)
{
	frame_len = size;
	gen_helpcall();
	printf("enter\n");
	gen_value(UINT, size);
}

void gen_epilogue(unsigned size)
{
	gen_helpcall();
	printf("exit\n");
	gen_value(UINT, size);
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

/* Output whatever goes in front of a helper call */
void gen_helpcall(void)
{
	printf("\tcall ");
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
		printf("\t.word %u\n", (unsigned) value);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		printf("\t.byte %u\n", (unsigned) value & 0xFF);
		break;
	case CINT:
	case UINT:
		printf("\t.word %d\n", (unsigned) value & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
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
	return 0;
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
		gen_helpcall();
		printf("cleanup\n");
		gen_value(UINT, n->right->value);
		return 1;
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	return 0;
}
