/*
 *	1802 thread/bytecode. Mostly we just output bytecode ops (with shifts). For some
 *	operations we output several in order to keep the engine size down. It's a trade off
 *	as the 1802 is slow enough that the more we do per bytecode the better.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"
#include "support1802/1802ops.h"
#include "support1802/1802debug.h"

/* For now assume 8/16bit */
#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

static unsigned opshift;

/*
 *	All setup functions and branches are in page 0. We depend on that
 *	behaviour.
 */
static void byteop_direct(unsigned op)
{
	/* Deal with shifts between the two bytecode blocks */
	if ((op & 0x0100) != opshift) {
		printf("\t.byte 0x00\t; %s\n", opnames[op >> 1]);
		opshift = op & 0x0100;
	}
	printf("\t.byte 0x%02X\t; %s\n", op & 0xFF, opnames[op >> 1]);
}

/*
 *	Ensure we are in page 0 when we drop through a label
 */
static void byteop_reset(void)
{
	if (opshift) {
		printf("\t.byte 0x00\t; sync to page 0\n");
		opshift = 0;
	}
}

static void byteop_label(void)
{
	opshift = 0;
}

static void outconstw(unsigned v)
{
	printf("\t.word %u\n", v & 0xFFFF);
}

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned unreachable;	/* Is the code we are generating reachable ? */

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
 *	Our chance to do tree rewriting. We don't do much for the 8080
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
	if (op == T_DEREF) {
		if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
			if (r->op == T_ARGUMENT)
				r->value += frame_len;
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
				l->value += frame_len;
			squash_left(n, T_LSTORE);
			return n;
		}
	}
	/* Eliminate casts for sign, pointer conversion or same */
	if (op == T_CAST) {
		if (nt == r->type || (nt ^ r->type) == UNSIGNED ||
		 (PTR(nt) && PTR(r->type))) {
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
	printf("_%s:\n", name);
	unreachable = 0;
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size + 4;	/* 2 for the return addr, 2 for the fp save */
	byteop_direct(op_fnenter);
        outconstw(-size);
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	byteop_direct(op_fnexit);
	outconstw(size);
	unreachable = 1;
}

void gen_label(const char *tail, unsigned n)
{
	if (!unreachable)
		byteop_reset();
	unreachable = 0;
	printf("L%d%s:\n", n, tail);
	byteop_label();	/* Always starts from page 0 */
}

unsigned gen_exit(const char *tail, unsigned n)
{
	byteop_direct(op_jump);
	printf("\t.word L%d%s\n", n, tail);
	unreachable = 1;
	return 0;
}

void gen_jump(const char *tail, unsigned n)
{
	byteop_direct(op_jump);
	printf("\t.word L%d%s\n", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	byteop_direct(op_jfalse);
	printf("\t.word L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	byteop_direct(op_jtrue);
	printf("\t.word L%d%s\n", n, tail);
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	if (!unreachable)
		byteop_reset();
	printf("Sw%d_%d:\n", tag, entry);
	/* All the branches hit with byteop at 0, unreach or otherwise */
	byteop_label();
	unreachable = 0;
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

void gen_helpcall(struct node *n)
{
	printf("\t???");
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
	printf("\tds %u\n", value);
}

void gen_text_data(struct node *n)
{
	printf("\t.word T%u\n", n->val2);
}

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
		/* We are big endian (the 1802 doesn't care but the 1805
		   is big endian with LDI RLDI etc */
		printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
		printf("\t.word %d\n", (unsigned) (value & 0xFFFF));
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

void gen_switch(unsigned n, unsigned type)
{
	switch(get_size(type)) {
	case 1:
		byteop_direct(op_switchc);
		break;
	case 2:
		byteop_direct(op_switch);
		break;
	case 4:
		byteop_direct(op_switchl);
		break;
	}
	printf("\t.word Sw%d\n", n);
	unreachable = 1;
}

/* Op with int or long forms */
void byteop(struct node *n, unsigned op, unsigned opl)
{
	if (get_size(n->type) < 4)
		byteop_direct(op);
	else
		byteop_direct(opl);
}

/* Op with int form */
void byteop_i(struct node *n, unsigned op)
{
	byteop_direct(op);
}

/* Op with char form */
void byteop_c(struct node *n, unsigned op, unsigned opl)
{
	unsigned s = get_size(n->type);
	if (s == 4) {
		byteop_direct(opl);
		return;
	}
	/* Ops are ordered so this works */
	if (s == 1)
		op -= 2; 
	byteop_direct(op);
}

/* Op with int or long forms, signed */
void byteop_s(struct node *n, unsigned op, unsigned opl)
{
	/* Select the unsigned versions if needed */
	if (n->type & UNSIGNED)
		op += 2;
	if (get_size(n->type) < 4)
		byteop_direct(op);
	else
		byteop_direct(opl);
}

/* Op with char form */
void byteop_cs(struct node *n, unsigned op, unsigned opl)
{
	unsigned s = get_size(n->type);
	if (s == 4) {
		if (n->type & UNSIGNED)
			op += 2;
		byteop_direct(opl);
		return;
	}
	/* Ops are ordered so this works */
	if (s == 1)
		op -= 2; 
	if (n->type & UNSIGNED)
		op += 4;
	byteop_direct(op);
}

/* EQ ops */
void byteop_eq_c(struct node *n, unsigned op, unsigned opl)
{
	byteop_c(n, op_xxeq, op_xxeql);
	byteop(n->right, op, opl);
	byteop_c(n->right, op_xxeqpost, op_xxeqpostl);
}

void byteop_posteq_c(struct node *n, unsigned op, unsigned opl)
{
	byteop_c(n, op_xxeq, op_xxeql);
	byteop(n->right, op, opl);
	byteop_c(n->right, op_xxeqpost, op_xxeqpostl);
}

void byteop_eq_cs(struct node *n, unsigned op, unsigned opl)
{
	byteop_c(n, op_xxeq, op_xxeql);
	byteop_s(n->right, op, opl);
	byteop_c(n->right, op_xxeqpost, op_xxeqpostl);
}

/* Conditions */

void byteop_cc(struct node *n, unsigned op, unsigned opl)
{
	unsigned s = get_size(n->right->type);
	if (s == 4)
		byteop_direct(opl);
	else
		byteop_direct(op);
	n->flags |= ISBOOL;
}


void byteop_cc_s(struct node *n, unsigned op, unsigned opl)
{
	unsigned s = get_size(n->right->type);
	if (s == 4) {
		if (n->right->type & UNSIGNED)
			opl += 2;
		byteop_direct(opl);
	} else {
		if (n->right->type & UNSIGNED)
			op += 2;
		byteop_direct(op);
	}
	n->flags |= ISBOOL;
}

void byteop_neg_cc(struct node *n, unsigned op, unsigned opl)
{
	byteop_cc(n, op, opl);
	byteop_direct(op_not);
	n->flags |= ISBOOL;
}


void byteop_neg_cc_s(struct node *n, unsigned op, unsigned opl)
{
	byteop_cc_s(n, op, opl);
	byteop_direct(op_not);
	n->flags |= ISBOOL;
}

void outsym(struct node *n)
{
	switch(n->op) {
	case T_NREF:
	case T_NSTORE:
	case T_NAME:
	case T_CALLNAME:
		gen_name(n);
		break;
	case T_ARGUMENT:
		printf("\t/word T%u+%u\n", n->val2, (unsigned)n->value + frame_len);
		break;
	case T_LREF:
	case T_LSTORE:
	case T_LOCAL:
		printf("\t/word T%u+%u\n", n->val2, (unsigned)n->value);
		break;
	case T_LBREF:
	case T_LBSTORE:
	case T_LABEL:
	case T_CONSTANT:
		gen_value(n->type, n->value);
		break;
	default:
		fprintf(stderr, "%x\n", n->op);
		error("os");
	}
}

static void outconst_size(struct node *n, unsigned long v)
{
	unsigned s= get_size(n->type);
	switch(s) {
	case 1:
		printf("\t.byte %u\n", (unsigned)v & 0xFF);
		break;
	case 2:
		printf("\t.word %u\n", (unsigned)v & 0xFFFF);
		break;
	case 4:
		printf("\t.word %u\n", (unsigned)(n->value >> 16) & 0xFFFF);
		printf("\t.word %u\n", (unsigned)v & 0xFFFF);
		break;
	}
}

unsigned gen_push(struct node *n)
{
	/* Our push will put the object on the stack, so account for it */
	/* Can't use helper as we might be doing a push of the result of
	   a cast */
	byteop_c(n, op_push, op_pushl);
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
	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		printf(";cleanup %d\n", (unsigned)r->value);
		if (r->value) {
			byteop_i(n, op_cleanup);
			outconstw(r->value);
		}
		return 1;
	case T_PLUS:
		if (r->op == T_CONSTANT && s == 2) {
			byteop_i(n, op_plusconst);
			outconstw(r->value);
			return 1;
		}
		return 0;
	case T_MINUS:
		if (r->op == T_CONSTANT && s == 2) {
			byteop_i(n, op_plusconst);
			outconstw(-r->value);
			return 1;
		}
		return 0;
	case T_PLUSPLUS:
		byteop_c(n, op_postinc, op_postincl);
		outconst_size(n, r->value);
		return 1;
	case T_MINUSMINUS:
		byteop_c(n, op_postinc, op_postincl);
		outconst_size(n, -r->value);
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
	struct node *l = n->left;
	struct node *r = n->right;
	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	if (n->op == T_COMMA) {
		l->flags |= NORETURN;
		codegen_lr(l);
		/* Parent determines child node requirements */
		r->flags |= (n->flags & NORETURN);
		codegen_lr(r);
		return 1;
	}
	if (unreachable)
		return 1;
	return 0;
}

/* Turn rt into lt */
static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned ls;
	unsigned rs;

	if (PTR(rt))
		rt = USHORT;
	if (PTR(lt))
		lt = USHORT;

	if (rt == FLOAT) {
		if (lt & UNSIGNED)
			byteop_i(n, op_f2ul);
		else
			byteop_i(n, op_f2l);
		return 1;
	}
	if (lt == FLOAT) {
		if (rt & UNSIGNED)
			byteop_i(n, op_ul2f);
		else
			byteop_i(n, op_l2f);
		return 1;
	}
	ls = get_size(lt);
	rs = get_size(rt);

	/* Size shrink is free */
	if (ls <= rs)
		return 1;
	/* Handle these directly as cast typing is a bit different to the rest */
	if (rt & UNSIGNED) {
		if (rs == 1)
			byteop_i(n, op_extuc);
		else
			byteop_i(n, op_extu);
		return 1;
	}
	if (rs == 1)
		byteop_i(n, op_extc);
	else
		byteop_i(n, op_ext);
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned v = n->value;
	unsigned nr = n->flags & NORETURN;

	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
#if 0
	/* We have a frame pointer */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_FUNCCALL && n->op != T_CALLNAME)
		sp -= get_stack_size(n->left->type);
#endif

	switch(n->op) {
	case T_NREF:
		byteop_c(n, op_nref, op_nrefl);
		outsym(n);
		return 1;
	case T_LBREF:
		byteop_c(n, op_nref, op_nrefl);
		outsym(n);
		return 1;
	case T_LREF:
		if (nr)
			return 1;
		byteop_c(n, op_lref, op_lrefl);
		outconstw(n->value);
		return 1;
	case T_NSTORE:
		byteop_c(n, op_nstore, op_lstorel);
		outsym(n);
		return 1;
	case T_LBSTORE:
		byteop_c(n, op_nstore, op_nstorel);
		outsym(n);
		return 1;
	case T_LSTORE:
		byteop_c(n, op_lstore, op_lstorel);
		outconstw(v);
		return 1;
	case T_CALLNAME:
		byteop_i(n, op_callfname);
		outsym(n);
		return 1;
	case T_ARGUMENT:
		/* Turn argument into local effectively */
		v += frame_len;
		/* Fall through */
	case T_LOCAL:
		/* We use a frame pointer not sp so no +sp */
		byteop_i(n, op_local);
		outconstw(v);
		return 1;
	case T_SHLEQ:
		byteop_eq_c(n, op_shl, op_shll);
		return 1;
	case T_SHREQ:
		byteop_eq_cs(n, op_shr, op_shrl);
		return 1;
	case T_EQEQ:
		byteop_cc(n, op_cceq, op_cceql);
		return 1;
	case T_LTLT:
		byteop(n, op_shl, op_shll);
		return 1;
	case T_GTGT:
		byteop_s(n, op_shr, op_shrl);
		return 1;
	case T_PLUSEQ:
		byteop_eq_c(n, op_plus, op_plusl);
		return 1;
	case T_MINUSEQ:
		byteop_eq_c(n, op_minus, op_minusl);
		return 1;
	case T_SLASHEQ:
		byteop_eq_cs(n, op_div, op_divl);
		return 1;
	case T_HATEQ:
		byteop_eq_c(n, op_xor, op_xorl);
		return 1;
	case T_BANGEQ:
		byteop_neg_cc(n, op_cceq, op_cceql);
		return 1;
	case T_OREQ:
		byteop_eq_c(n, op_or, op_orl);
		return 1;
	case T_ANDEQ:
		byteop_eq_c(n, op_band, op_bandl);
		return 1;
	case T_STAREQ:
		byteop_eq_c(n, op_mul, op_mull);
		return 1;
	case T_PERCENTEQ:
		byteop_eq_c(n, op_rem, op_reml);
		return 1;
	case T_AND:
		byteop(n, op_band, op_bandl);
		return 1;
	case T_STAR:
		byteop(n, op_mul, op_mull);
		return 1;
	case T_SLASH:
		byteop_s(n, op_div, op_divl);
		return 1;
	case T_PERCENT:
		byteop_s(n, op_rem, op_reml);
		return 1;
	case T_PLUS:
		byteop(n, op_plus, op_plusl);
		return 1;
	case T_MINUS:
		byteop(n, op_minus, op_minusl);
		return 1;
	case T_HAT:
		byteop(n, op_xor, op_xorl);
		return 1;
	case T_LT:
		byteop_cc_s(n, op_cclt, op_ccltl);
		return 1;
	case T_GT:
		byteop_neg_cc_s(n, op_cclteq, op_cclteql);
		return 1;
	case T_LTEQ:
		byteop_cc_s(n, op_cclteq, op_cclteql);
		return 1;
	case T_GTEQ:
		byteop_neg_cc_s(n, op_cclt, op_ccltl);
		return 1;
	case T_OR:
		byteop(n, op_or, op_orl);
		return 1;
	case T_TILDE:
		byteop(n, op_cpl, op_cpll);
		return 1;
	case T_BANG:
		byteop(n, op_not, op_notl);
		return 1;
	case T_EQ:
		byteop_c(n, op_assign, op_assignl);
		return 1;
	case T_DEREF:
		byteop_c(n, op_deref, op_derefl);
		return 1;
	case T_NEGATE:
		byteop(n, op_negate, op_negatel);
		return 1;
	case T_FUNCCALL:
		byteop_i(n, op_callfunc);
		return 1;
	case T_NAME:
	case T_LABEL:
		byteop_i(n, op_const);
		outsym(n);
		return 1;
	case T_CAST:
		gen_cast(n);
		return 1;
	case T_CONSTANT:
		byteop_c(n, op_const, op_constl);
		outsym(n);
		return 1;
	case T_BOOL:
		/* Bool uses the type of the right side */
		if (!(n->right->flags & ISBOOL))
			byteop(n->right, op_bool, op_booll);
		return 1;
	case T_ARGCOMMA:
		/* Pass it down */
		return 0;
	}
	fprintf(stderr, "No rule for %x\n", n->op);
	return 0;
}
