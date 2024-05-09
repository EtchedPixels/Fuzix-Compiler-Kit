/*
 *	General backend helpers for Z80 except for code generator
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"
#include "backend-z80.h"


/* Export the C symbol */
void gen_export(const char *name)
{
	printf("	.export _%s\n", name);
}

void gen_segment(unsigned segment)
{
	switch(segment) {
	case A_CODE:
		printf("\t.%s\n", codeseg);
		break;
	case A_DATA:
		printf("\t.data\n");
		break;
	case A_BSS:
		printf("\t.bss\n");
		break;
	case A_LITERAL:
		printf("\t.literal\n");
		break;
	default:
		error("gseg");
	}
}

/* Generate the function prologue - may want to defer this until
   gen_frame for the most part */
void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
	unreachable = 0;
}

/* Generate the stack frame */
/* TODO: defer this to statements so we can ld/push initializers */
void gen_frame(unsigned size,  unsigned aframe)
{
	frame_len = size;
	sp = 0;
	use_fp = 0;

	if (size || (func_flags & (F_REG(1)|F_REG(2)|F_REG(3))))
		func_cleanup = 1;
	else
		func_cleanup = 0;

	argbase = ARGBASE;

	/* In banked mode the arguments are two bytes further out */
	if (cpufeat & 1)
		argbase += 2;

	if (func_flags & F_REG(1)) {
		printf("\tpush bc\n");
		argbase += 2;
	}
	if (func_flags & F_REG(2)) {
		printf("\tpush ix\n");
		argbase += 2;
	}
	if (func_flags & F_REG(3)) {
		printf("\tpush iy\n");
		argbase += 2;
	} else {
		/* IY is free use it as a frame pointer ? */
		if (!optsize && size > 4) {
			argbase += 2;
			printf("\tpush iy\n");
			/* Remember we need to restore IY */
			func_flags |= F_REG(3);
			use_fp = 1;
		}
	}
	/* If we are building a frame pointer we need to do this work anyway */
	if (use_fp) {
		printf("\tld iy,0x%x\n", (uint16_t) - size);
		printf("\tadd iy,sp\n");
		printf("\tld sp,iy\n");
		return;
	}
	if (size > 10) {
		printf("\tld hl,0x%x\n", (uint16_t) -size);
		printf("\tadd hl,sp\n");
		printf("\tld sp,hl\n");
		return;
	}
	if (size & 1) {
		printf("\tdec sp\n");
		size--;
	}
	while(size) {
		printf("\tpush hl\n");
		size -= 2;
	}
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp != 0)
		error("sp");

	/* Return in HL, does need care on stack. TOOD: flag void functions
	   where we can burn the return */
	sp -= size;

	/* This can happen if the function never returns or the only return
	   is a by a ret directly (ie from a function without locals) */
	if (unreachable)
		return;

	if (size > 10) {
		unsigned x = func_flags & F_VOIDRET;
		if (!x)
			printf("\tex de,hl\n");
		printf("\tld hl,0x%x\n", (uint16_t)size);
		printf("\tadd hl,sp\n");
		printf("\tld sp,hl\n");
		if (!x)
			printf("\tex de,hl\n");
	} else {
		if (size & 1) {
			printf("\tinc sp\n");
			size--;
		}
		while (size) {
			printf("\tpop de\n");
			size -= 2;
		}
	}
	if (func_flags & F_REG(3))
		printf("\tpop iy\n");
	if (func_flags & F_REG(2))
		printf("\tpop ix\n");
	if (func_flags & F_REG(1))
		printf("\tpop bc\n");
	printf("\tret\n");
	unreachable = 1;
}

void gen_label(const char *tail, unsigned n)
{
	unreachable = 0;
	printf("L%u%s:\n", n, tail);
}

/* A return statement. We can sometimes shortcut this if we have
   no cleanup to do */
unsigned gen_exit(const char *tail, unsigned n)
{
	if (func_cleanup) {
		gen_jump(tail, n);
		return 0;
	}
	else {
		printf("\tret\n");
		unreachable = 1;
		return 1;
	}
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\tjr L%u%s\n", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\tjr %s,L%u%s\n", ccflags + 2, n, tail);
	ccflags = ccnormal;
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjr %c%c,L%u%s\n", ccflags[0], ccflags[1], n, tail);
	ccflags = ccnormal;
}

void gen_cleanup(unsigned v)
{
	/* CLEANUP is special and needs to be handled directly */
	sp -= v;
	if (v > 10) {
		/* This is more expensive, but we don't often pass that many
		   arguments so it seems a win to stay in HL */
		/* TODO: spot void function and skip ex de,hl */
		printf("\tex de,hl\n");
		printf("\tld hl,0x%x\n", v);
		printf("\tadd hl,sp\n");
		printf("\tld sp,hl\n");
		printf("\tex de,hl\n");
	} else {
		while(v >= 2) {
			printf("\tpop de\n");
			v -= 2;
		}
		if (v)
			printf("\tinc sp\n");
	}
}

/*
 *	Helper handlers. We use a tight format for integers but C
 *	style for float as we'll have C coded float support if any
 */

/* True if the helper is to be called C style */
static unsigned c_style(struct node *np)
{
	register struct node *n = np;
	/* Assignment is done asm style */
	if (n->op == T_EQ)
		return 0;
	/* Float ops otherwise are C style */
	if (n->type == FLOAT)
		return 1;
	n = n->right;
	if (n && n->type == FLOAT)
		return 1;
	return 0;
}

void gen_helpcall(struct node *n)
{
	/* Check both N and right because we handle casts to/from float in
	   C call format */
	if (c_style(n))
		gen_push(n->right);
	printf("\tcall __");
}

void gen_helpclean(struct node *n)
{
	unsigned s;

	if (c_style(n)) {
		s = 0;
		if (n->left) {
			s += get_size(n->left->type);
			/* gen_node already accounted for removing this thinking
			   the helper did the work, adjust it back as we didn't */
			sp += s;
			}
		s += get_size(n->right->type);
		gen_cleanup(s);
		/* C style ops that are ISBOOL didn't set the bool flags */
		if (n->flags & ISBOOL)
			printf("\txor a\n\tcp l\n");
	}
}

void gen_switch(unsigned n, unsigned type)
{
	printf("\tld de,Sw%u\n", n);
	printf("\tjp __switch");
	helper_type(type, 0);
	printf("\n");
	unreachable = 1;
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%u:\n", n);
	printf("\t.word %u\n", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
	printf("Sw%u_%u:\n", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%u_%u\n", tag, entry);
}

void gen_data_label(const char *name, unsigned align)
{
	printf("_%s:\n", name);
}

void gen_space(unsigned value)
{
	printf("\t.ds %u\n", value);
}

void gen_text_data(unsigned n)
{
	printf("\t.word T%u\n", n);
}

/* The label for a literal (currently only strings)
   TODO: if we add other literals we may need alignment here */

void gen_literal(unsigned n)
{
	if (n)
		printf("T%u:\n", n);
}

void gen_name(struct node *n)
{
	printf("\t.word _%s+%u\n", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	unsigned w = WORD(value);
	if (PTR(type)) {
		printf("\t.word %u\n", w);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		printf("\t.byte %u\n", BYTE(w));
		break;
	case CSHORT:
	case USHORT:
		printf("\t.word %u\n", w);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		printf("\t.word %u\n", w);
		printf("\t.word %u\n", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	/* For now.. */
	printf("\t.z80\n");
}

void gen_end(void)
{
}

