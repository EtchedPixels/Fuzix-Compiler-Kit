/*
 *	General glue between the backend and 680x code generator
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"
#include "backend-6800.h"

unsigned cpu_has_d;		/* 16bit ops and 'D' are present */
unsigned cpu_has_xgdx;		/* XGDX is present */
unsigned cpu_has_abx;		/* ABX is present */
unsigned cpu_has_pshx;		/* Has PSHX PULX */
unsigned cpu_has_y;		/* Has Y register */
unsigned cpu_has_lea;		/* Has LEA. For now 6809 but if we get to HC12... */
unsigned cpu_is_09;		/* Bulding for 6x09 so a bit different */
unsigned cpu_pic;		/* Position independent output (6809 only) */

const char *jmp_op = "jmp";
const char *jsr_op = "jsr";
const char *pic_op = "";

/*
 *	State for the current function
 */
unsigned frame_len;		/* Number of bytes of stack frame */
unsigned argbase;		/* Argument offset in current function */
unsigned sp;			/* Stack pointer offset tracking */
unsigned unreachable;		/* Code following an unconditional jump */

/* Export the C symbol */
void gen_export(const char *name)
{
	printf("	.export _%s\n", name);
}

void gen_segment(unsigned s)
{
	switch(s) {
	case A_CODE:
		puts("\t.code");
		break;
	case A_DATA:
		puts("\t.data");
		break;
	case A_LITERAL:
		puts("\t.literal");
		break;
	case A_BSS:
		puts("\t.bss");
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
	argbase = ARGBASE;
	/* Only occurs on 6809 */
	if (func_flags & F_REG(1)) {
		puts("\tpshs u");
		argbase += 2;
	}
	/* TODO: there is an optimization trick here for 09 where you
	   can use a pshs combining the pshs u to make some size of frame */
	frame_len = size;
	adjust_s(-size, 0);
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp)
		error("sp");
	adjust_s(size, (func_flags & F_VOIDRET) ? 0 : 1);
	if (func_flags & F_REG(1))
		/* 6809 only */
		puts("\tpuls u,pc");
	/* TODO: we are asssuming functions clean up their args if not
	   vararg so this will have to change */
	else if (argsize == 0 || cpu_has_d || (func_flags & F_VARARG))
		puts("\trts");
	else if (argsize <= 8)
		printf("\t%s __cleanup%u\n", jmp_op, argsize);
	else {
		/* Icky - can we do better remembering AB is live for
		   non void funcs */
		printf("\t%s __cleanup\n\t.word %u\n", jmp_op, argsize);
	}
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
	printf("\t%s L%d%s\n", jmp_op, n, tail);
	unreachable = 1;
	return 0;
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\t%s L%d%s\n", jmp_op, n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	if (cpu_is_09)
		printf("\tbeq L%d%s\n", n, tail);
	else
		printf("\tjeq L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	if (cpu_is_09)
		printf("\tbne L%d%s\n", n, tail);
	else
		printf("\tjne L%d%s\n", n, tail);
}

void gen_switch(unsigned n, unsigned type)
{
	printf("\tldx #Sw%u\n\t%s __switch", n, jmp_op);
	helper_type(type, 0);
	putchar('\n');
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n\t.word %d\n", n, size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
	invalidate_all();
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

/* True if the helper is to be called C style */
static unsigned c_style(struct node *np)
{
	register struct node *n = np;
	/* Assignment is done asm style. No other float used asm helpers
	  should show up as helpcalls but if they do they need to be
	  listed here */
	if (n->op == T_EQ || n->op == T_DEREF)
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
	invalidate_all();
	printf("\t%s __", jsr_op);
}

void gen_helptail(struct node *n)
{
}

void gen_helpclean(struct node *n)
{
	if (c_style(n)) {
		unsigned s = 0;
		if (n->left) {
			s += get_size(n->left->type);
			/* gen_node already accounted for removing this thinking
			   the helper did the work, adjust it back as we didn't */
			sp += s;
		}
		s += get_size(n->right->type);
		/* No helper uses varargs */
		sp -= s;
		/* 6800 expects called code to clean up */
		if (cpu != 6800)
			adjust_s(s, 1);
		/* C style ops that are ISBOOL didn't set the bool flags */
		if (n->flags & ISBOOL)
			puts("\ttstb");
	}
}

void gen_data_label(const char *name, unsigned align)
{
	printf("_%s:\n", name);
}

void gen_space(unsigned value)
{
	printf("\t.ds %d\n", value);
}

void gen_text_data(struct node *n)
{
	printf("\t.word T%d\n", n->val2);
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
	switch(cpu) {
	case 6309:
	case 6809:
		cpu_pic = 1;
		cpu_is_09 = 1;
		cpu_has_d = 1;
		cpu_has_pshx = 1;
		cpu_has_y = 1;
		cpu_has_abx = 1;
		cpu_has_xgdx = 1;
		cpu_has_lea = 1;
		jmp_op = "bra";	/* Maybe a choice will be needed for jmp v bra/lbra ? */
		jsr_op = "lbsr";
		pic_op = ",pcr";
		break;
	case 6811:
		cpu_has_y = 1;
	case 6303:
		cpu_has_xgdx = 1;
		/* Fall through */
	case 6803:
		cpu_has_d = 1;
		cpu_has_abx = 1;
		cpu_has_pshx = 1;
	case 6800:
		break;
	}
	/* For the moment. Needs adding to assembler for 6809 v 6309 */
	if (cpu != 6809 && cpu != 6811)
		printf("\t.setcpu %u\n", cpu);
	puts("\t.code");
}

void gen_end(void)
{
}

void gen_tree(struct node *n)
{
	codegen_lr(n);
	puts(";");
}

