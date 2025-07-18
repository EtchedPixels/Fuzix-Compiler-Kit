/*
 *	INS8070 Backend
 *
 *	EA	-	16bit accumulator
 *	T	-	various scratch uses, 2nd helper argument
 *	P0	-	program counter
 *	P1	-	stack pointer (offsets but no inc/dec)
 *	P2	-	working pointer (usable as pointer)
 *	P3	-	maybe use for reg var ?
 *
 *	FFxx addressing (akin to zero page on some other processors) is used
 *	for some internal state only
 *
 *	Oddities
 *	- Flags are not directly testable with branches. Instead you
 *	  have to move or and them into A
 *	- There is no 16bit indirect addressing mode, only pointer relative
 *	  8bit (including via PC and SP). JMP/JSR appear to be exceptions
 *	  but are really immediate loads or PLI of a constant.
 *	- Autoindexing is sort of like the usual (r+) and (-r) forms of other
 *	  processors but the size of movement is encoded in the instruction so
 *	  can be used in many ways (eg folding together *x++)
 *
 *	TODO:
 *	- Fold together all the condition helpers for byte/word
 *	- Rewrite x++ and --x forms to use autoindexing
 *	- Byte sized ++/-- optimizations
 *	   (in particular using E as a save reg for bytesized -- to avoid tmp)
 *	- Shift >> or << 16 optimization (and maybe >=16 as it's then a
 *	  standard sized shift if we set up right for >> unsigned)
 *	- Implement register tracking (in progress)
 *		Need to do pointer tracking an EA node tracking for ptr
 *		and word sized refs.
 *		Need to enable LREF/LSTORE etc for dword
 *		Constant tracking needs putting back
 *	- Optimised pushing for args (push long, push lref)
 *	- Spot the *foo++ case and build a p2 based ++ op for it
 *		ld p2,1,p1 ; st ea,@2,p2; st p2,1,p1 etc
 *		ld p2,1,p1 ; add ea,@2,p2; st p2,1,p1 etc
 *	- CCONLY
 *	- Comparisons using CCONLY
 *	- Peepholes
 *	- Can we do some equality type comparisons better inline with something
 *	  like sub ea,blah jsr bool/bang ?
 *	- Enable register variables with p3 (and maybe one day T) - needs p3
 *	  use in helpers cleaning up first
 *	- Would it make more sense to use p3 as hireg
 *		We can xch ea,p3 when working on halves
 *		We can ld p3,=foo (but not indexed forms)
 *		If we track p3 as upper value we can also do stuff
 *		like ld p3,ea ld ea,p3 for half words, and we can >> 16 << 16
 *		similarly
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"
#include "backend-byte.h"

#define ARGBASE		2

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

#define T_NREF		(T_USER)		/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)		/* Function call by name */
#define T_NSTORE	(T_USER+2)		/* Store to a C global/static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)		/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)
#define T_DEREFPLUS	(T_USER+7)		/* *(thing + offset) */
#define T_LDEREF	(T_USER+8)		/* *local + offset */
#define T_LEQ		(T_USER+9)		/* *local + offset = n*/
#define T_EQPLUS	(T_USER+10)		/* *(ac + n) = m */

static void load_ptr_ea(unsigned n);
static void load_ea_ptr(unsigned n);

#define O_MODIFY	0			/* Operation that modifies */
#define O_LOAD		1			/* Operation that loads */
#define O_STORE		2			/* Operation that stores */

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned unreachable;	/* Track unreachable code state */
static unsigned func_cleanup;	/* Zero if we can just ret out */
static unsigned label;		/* Used for local branches */

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

/* We work in words on stack not because of alignment or sizing but
   because we have no easy "throw a byte of stack without mashing EA */
static unsigned get_stack_size(unsigned t)
{
	unsigned sz = get_size(t);
	if (sz == 1)
		return 2;
	return sz;
}

/*
 *	Register tracking (not all yet done)
 *	Need to sort out xch ea,pn and constants for one
 */

static uint8_t a_value;
static uint8_t e_value;
static uint16_t t_value;
static uint16_t hireg_value;
static unsigned a_valid;
static unsigned e_valid;
static unsigned t_valid;
static unsigned ea_valid;
static unsigned hireg_valid;
static struct node ea_node;
static struct node t_node;
static struct node p_node[2];	/* P2 and P3 */
static unsigned p_valid[2];

static void invalidate_a(void)
{
	a_valid = 0;
	ea_valid = 0;
}

static void invalidate_e(void)
{
	e_valid = 0;
	ea_valid = 0;
}

static void invalidate_ea(void)
{
	e_valid = 0;
	a_valid = 0;
	ea_valid = 0;
}

static void set_a(uint8_t n)
{
	a_valid = 1;
	a_value = n;
	ea_valid = 0;
}

static void set_e(uint8_t n)
{
	e_valid = 1;
	e_value = n;
	ea_valid = 0;
}

static void set_ea(unsigned n)
{
	a_valid = 1;
	e_valid = 1;
	a_value = n;
	e_value = n >> 8;
	ea_valid = 0;	/* No valid node */
}

static unsigned map_op(register unsigned op)
{
	switch(op) {
	case T_LSTORE:
		op = T_LREF;
	case T_LREF:
		break;
	case T_LBSTORE:
		op = T_LBREF;
	case T_LBREF:
		break;
	case T_NSTORE:
		op = T_NREF;
	case T_NREF:
		break;
	case T_NAME:
	case T_LABEL:
	case T_LOCAL:
		break;
	case T_CONSTANT:
		break;
	/* Don't do other matches for now */
	default:
		return 0;
	}
	return op;
}

static void set_ea_node(struct node *n)
{
	a_valid = 0;
	e_valid = 0;
	ea_valid = 1;
	memcpy(&ea_node, n, sizeof(ea_node));
	ea_node.op = map_op(ea_node.op);
//	printf(";set_ea_node from %02X to %02X\n", n->op, ea_node.op);
}

static unsigned is_ea_node(register struct node *n)
{
	unsigned op;
	if (ea_valid == 0)
		return 0;
	op = map_op(n->op);
//	printf(";is_ea_node op %04X node op %04X, val %08lX, node val %08lX\n",
//		op, ea_node.op, n->value, ea_node.value);
	if (ea_node.op == op && ea_node.value == n->value &&
		ea_node.type == n->type && ea_node.snum == n->snum &&
		ea_node.val2 == n->val2)
		return 1;
	return 0;
}

static void adjust_a(unsigned n)
{
	a_value += n;
	ea_valid = 0;
}

static void adjust_ea(unsigned n)
{
	unsigned t = a_value + BYTE(n);
	a_value = t;
	e_value += BYTE(n >> 8);
	if (t & 0x0100)
		e_value++;
}

static void set_t(unsigned n)
{
	t_value = n;
	t_valid = 1;
}

static void set_t_node(struct node *n)
{
	t_valid = 2;
	memcpy(&t_node, n, sizeof(ea_node));
}

static void invalidate_t(void)
{
	t_valid = 0;
}

static void invalidate_hireg(void)
{
	hireg_valid = 0;
}

static void invalidate_all(void)
{
	t_valid = 0;
	hireg_valid = 0;
	p_valid[0] = 0;
	p_valid[1] = 0;
	invalidate_ea();
//	printf(";invalidate all\n");
}

static void flush_writeback(void)
{
	/* TODO: need to wipe NREF/LBREF etc */
	invalidate_all();
}

static unsigned ea_is(unsigned v)
{
	v &= 0xFFFF;
	if (a_valid && e_valid && a_value == (v & 0xFF) && e_value == (v >> 8))
		return 1;
	return 0;
}

static void load_ea_hireg(void)
{
	if (hireg_valid) {
		if (ea_is(hireg_value))
			return;
		puts("\tld ea,:__hireg");
		set_ea(hireg_value);
//		printf(";load ea hireg ea now %d %d %02X%02X\n", e_valid,
//			a_valid, e_value, a_value);
	} else {
		puts("\tld ea,:__hireg");
		invalidate_ea();
	}
}

static void store_ea_hireg(void)
{
	uint16_t val = (e_value << 8) | a_value;
	/* hireg already holds this value */
//	printf(";st ea hireg\n");
	if (a_valid && e_valid) {
		if (hireg_valid && val == hireg_value)
			return;
		puts("\tst ea,:__hireg");
		/* If EA is known then hireg is now known */
		hireg_value = (e_value << 8) | a_value;
		hireg_valid = 1;
//		printf(";hireg now %04X\n", val);
	} else {
		puts("\tst ea,:__hireg");
		hireg_valid = 0;
	}
}

static void invalidate_ptr(unsigned p)
{
	if (p >= 2)
		p_valid[p - 2] = 0;
}

static void load_t_ea(void)
{
	if (t_valid == 1 && ea_is(t_value))
		return;
	if (ea_valid)
		set_t_node(&ea_node);
	else if (e_valid && a_valid)
		set_t((e_value << 8) | a_value);
	else
		invalidate_t();
	puts("\tld t,ea");
}

void load_ea_t(void)
{
	if (t_valid == 1 && ea_is(t_value))
		return;
	if (t_valid == 2)
		set_ea_node(&t_node);
	else if (t_valid == 1)
		set_ea(t_value);
	else
		invalidate_ea();
	puts("\tld ea,t");
}

/* Also need a find_ptr that turns LSTORE/LREF to LOCAL etc so we can look
   for the object pointer itself to optimise stuff like ld ea,T%u into
   ld ea,p3 */
static unsigned find_ref(register struct node *n)
{
	unsigned op = n->op;
	switch(op) {
	case T_LSTORE:
		op = T_LREF;
	case T_LREF:
		break;
	case T_LBSTORE:
		op = T_LBREF;
	case T_LBREF:
		break;
	case T_NSTORE:
		op = T_NREF;
	case T_NREF:
		break;
	default:
		return 0;
	}
	/* TODO: if value is not quite the same check if in off range and
	   still use it */
	if (p_valid[0] && p_node[0].op == op && p_node[0].value == n->value &&
		p_node[0].val2 == n->val2 && p_node[0].snum == n->snum)
		return 2;
	if (p_valid[1] && p_node[1].op == op && p_node[1].value == n->value &&
		p_node[1].val2 == n->val2 && p_node[1].snum == n->snum)
		return 3;
	return 0;
}

static void set_ptr_ref(unsigned p, struct node *n)
{
	if (p >= 2) {
		p -= 2;
		p_valid[p] = 1;
		memcpy(p_node + p, n, sizeof(struct node));
	}
}

/*
 *	Rewriting
 */

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
 *	Heuristic for guessing what to put on the right. This is very
 *	processor dependent. For 8070 we are quite limited especially
 *	with static/global.
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
	/* We can load this directly into a register but may need a
	   pointer register */
	if (op == T_NREF || op == T_LBREF)
		return 1;
	return 0;
}

struct node *gen_rewrite(struct node *top)
{
	byte_label_tree(top, BTF_RELABEL);
	return top;
}

/*
 *	Our chance to do tree rewriting. We don't do much for the 8070
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned op = n->op;
	unsigned nt = n->type;
	unsigned off;

	/* Convert references with an offset into a new node so we can make proper use of the
	   indexing on the 8070 */
	if (op == T_DEREF || op == T_DEREFPLUS) {
		if (op == T_DEREF)
			n->value = 0;	/* So we can treat deref/derefplus together */
		if (r->op == T_PLUS) {
			off = n->value + r->right->value;
			/* FIXME: 127 or 125 if size 4 */
			if (r->right->op == T_CONSTANT && off < 127) {
				n->op = T_DEREFPLUS;
				free_node(r->right);
				n->right = r->left;
				n->value = off;
				free_node(r);
				/* We might then rewrite this again */
				return gen_rewrite_node(n);
			}
		}
	}
	if ((op == T_DEREF || op == T_DEREFPLUS) && r->op == T_LREF) {
		/* At this point r->value is the offset for the local */
		/* n->value is the offset for the ptr load */
		r->val2 = n->value;		/* Save the offset so it is squashed in */
		squash_right(n, T_LDEREF);	/* n->value becomes the local ref */
		return n;
	}
	if ((op == T_EQ || op == T_EQPLUS) && l->op == T_LREF) {
		/* At this point r->value is the offset for the local */
		/* n->value is the offset for the ptr load */
		l->val2 = n->value;		/* Save the offset so it is squashed in */
		squash_left(n, T_LEQ);	/* n->value becomes the local ref */
		return n;
	}
	/* Need to adapt the make_ptr_ref can_make_ptr_ref functions to
	   take an additional offset to make this work */
	if (op == T_EQ || op == T_EQPLUS) {
		if (op == T_EQ)
			n->value = 0;	/* So we can treat deref/derefplus together */
		if (l->op == T_PLUS) {
			off = n->value + l->right->value;
			if (l->right->op == T_CONSTANT && off < 127) {
				n->op = T_EQPLUS;
				free_node(l->right);
				n->left = l->left;
				n->value = off;
				free_node(l);
				/* We might then rewrite this again */
				return gen_rewrite_node(n);
			}
		}
	}
	/* Rewrite references into a load operation */
	if (op == T_DEREF) {
		if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
			if (r->op == T_ARGUMENT)
				r->value += ARGBASE + frame_len;
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
				l->value += ARGBASE + frame_len;
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
	/* Commutive operations. We can swap the sides over on these */
	if (op == T_AND || op == T_OR || op == T_HAT || op == T_STAR || op == T_PLUS) {
/*		printf(";left %d right %d\n", is_simple(n->left), is_simple(n->right)); */
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
	/* Annoyingly we can't use autoindex on SP */
	frame_len = size;
	sp = 0;

	if (size || func_flags & F_REG(1))
		func_cleanup = 1;
	else
		func_cleanup = 0;

	/* For ease of cleanup just leave a padding byte */
	if (size & 1) {
		size++;
		sp = 1;
	}

	if (size > 10) {
		load_ea_ptr(1);
		printf("\tsub ea,=%d\n", size);
		invalidate_ea();
		load_ptr_ea(1);
		return;
	}
	while(size) {
		puts("\tpush ea");
		size -= 2;
	}
	invalidate_all();
}

void gen_cleanup(unsigned size, unsigned save)
{
	sp -= size;
	if (size > 10 + 2 * save) {
		if (save)
			load_t_ea();
		load_ea_ptr(1);
		printf("\tadd ea,=%d\n", size);
		invalidate_ea();
		load_ptr_ea(1);
		if (save)
			load_ea_t();
		else
			invalidate_all();
	} else while(size) {
		puts("\tpop p2");
		invalidate_ptr(2);
		size -= 2;
	}
}

/* TODO: no void save / restore EA or return in a P reg or T .. decisions */
void gen_epilogue(unsigned size, unsigned argsize)
{
	unsigned x = func_flags & F_VOIDRET;
	/* Reverse effect of packing byte */
	if (size & 1) {
		size++;
		sp--;
	}
	if (sp)
		error("sp");
	if (unreachable)
		return;
	gen_cleanup(size, !x);
	printf("\tret\n");
	unreachable = 1;
}

void gen_label(const char *tail, unsigned n)
{
	invalidate_all();
	unreachable = 0;
	printf("L%d%s:\n", n, tail);
}

/* TODO: tidy this up for no arg case */
unsigned gen_exit(const char *tail, unsigned n)
{
	unreachable = 1;
	if (func_cleanup) {
		printf("\tjmp L%d%s\n", n, tail);
		return 0;
	}
	printf("\tret\n");
	return 1;
}

void gen_jump(const char *tail, unsigned n)
{
	flush_writeback();
	printf("\tjmp L%d%s\n", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	flush_writeback();
	printf("\tjz L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	flush_writeback();
	printf("\tjnz L%d%s\n", n, tail);
}

void gen_switch(unsigned n, unsigned type)
{
	flush_writeback();
	invalidate_all();
	printf("\tld p2,=Sw%d\n", n);
	printf("\tjmp __switch");
	helper_type(type, 0);
	putchar('\n');
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
	flush_writeback();
	invalidate_all();
	printf("\tjsr __");
}

void gen_helptail(struct node *n)
{
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
		gen_cleanup(s, 1);
		/* Caution - for future optimizations:
		   C style ops that are ISBOOL didn't set the bool flags */
	}
	/* E must be 0 as EA = 0/1 */
	if (n->flags & ISBOOL)
		set_e(0);
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
	/* Our push will put the object on the stack, so account for it */
	unsigned size = get_stack_size(n->type);
	sp += size;
	switch(size) {
	case 1:
		puts("\tpush a");
		break;
	case 2:
		puts("\tpush ea");
		break;
	case 4:
		load_t_ea();
		load_ea_hireg();
		puts("\tpush ea");
		load_ea_t();
		puts("\tpush ea");
		break;
	default:
		return 0;
	}
	return 1;
}

/* Load a pointer from EA and track if it's P2/3 */
static void load_ptr_ea(unsigned n)
{
	printf("\tld p%u, ea\n", n);
	if (ea_valid)
		set_ptr_ref(n, &ea_node);
	else
		invalidate_ptr(n);
}

/* Load EA from a pointer and track if it's P2/3 */
static void load_ea_ptr(unsigned n)
{
	printf("\tld ea,p%u\n", n);

	if (n >= 2 && p_valid[n - 2])
		set_ea_node(p_node - 2 + n);
	else
		invalidate_ea();
}

/* Get a constant into the working register and track it */
static void load_ea(unsigned sz, unsigned long v)
{
	unsigned vw = v & 0xFFFF;
//	printf(";load ea %d %08lX\n", sz, v);
	if (sz == 1) {
		vw &= 0xFF;
		if (a_valid && vw == a_value)
			return;
		if (e_valid && vw == e_value)
			puts("ld a,e");
		else
			printf("\tld a,=%ld\n", v & 0xFF);
		set_a(vw);
	}
	else if (sz == 2) {
		if (ea_is(vw))
			return;
		if (t_valid == 1 && vw == t_value)
			puts("\tld ea,t");
		/* Getting a word out of hireg is cheaper than const load */
		else if (hireg_valid && vw == hireg_value)
			load_ea_hireg();
		/* If we can generate the pair by loading A into E or vice
		   versa then do so - mostly useful for FFFF and 0000 */
		else if ((vw >> 8) == (vw & 0xFF)) {
			if (a_valid && (vw & 0xFF) == a_value)
				puts("\tld e,a");
			else if (e_valid && (vw & 0xFF) == e_value)
				puts("\tld a,e");
			else
				printf("\tld ea,=%u\n", vw);
			set_ea(vw);
			return;
		} else if (e_valid && e_value == (vw >> 8))
			printf("\tld a,=%u\n", vw & 0xFF);
		else
			printf("\tld ea,=%u\n", vw);
		set_ea(vw);
	} else {
		load_ea(2, v >> 16);
		store_ea_hireg();
		load_ea(2, vw);
	}
}

static void load_e(unsigned v)
{
	if (e_valid && e_value == v)
		return;
	if (a_valid && a_value == v)
		puts("\tld e,a");
	else
		printf("\txch a,e\n\tld a,=%u\n\txch a,e\n", v);
	set_e(v);
}

/* Track exchanges we use */
static void xch_a_e(void)
{
	unsigned t;
	puts("\txch a,e");
	t = a_valid;
	a_valid = e_valid;
	e_valid = t;
	t = a_value;
	a_value = e_value;
	e_value = t;
}

static void xch_ea_p2(void)
{
	struct node tmp;
	unsigned t;
	puts("\txch ea,p2");
	t = p_valid[1];
	memcpy(&tmp, &p_node, sizeof(struct node));
	memcpy(&p_node, &ea_node, sizeof(struct node));
	memcpy(&ea_node, &tmp, sizeof(struct node));
	p_valid[1] = ea_valid;
	set_ea_node(&tmp);
	if (t == 0)
		invalidate_ea();
}

static void load_t(unsigned v)
{
	if (t_valid == 1 && t_value == v)
		return;
	if (ea_is(v))
		load_t_ea();
	else
		printf("\tld t,=%d\n", v & 0xFFFF);
	set_t(v);
}

static void repeated_op(const char *op, unsigned n)
{
	while(n--)
		puts(op);
}

static void discard_words(unsigned s)
{
	printf(";discard words %u\n", s);
	s = (s + 1) >> 1;
	while(s--)
		puts("\tpop p2");
	invalidate_ptr(2);
}

/* TODO:
   For 8bit:
   	shifts if fastest, MPY if not
   For 16bit signed/unsigned
   	shifts if fastest, MPY if constant positive
   	left shift and MPY by constant >> 1 if constant even
   else helper
 */

static unsigned gen_fast_mul(unsigned sz, unsigned value)
{
	if (value < 2) {
		if (value == 0)
			load_ea(sz, 0);
		return 1;
	}
	if (sz > 2)
		return 0;
	if (!(value & (value - 1))) {
		/* Do 8bits of shift by swapping the register about */
		if (value >= 256) {
			if (sz == 2)
				xch_a_e();
			load_ea(1, 0);
			value >>= 8;
		}
		/* For each power of two just shift left */
		while(value > 1) {
			value >>= 1;
			if (sz == 2)
				puts("\tsl ea");
			else
				puts("\tsl a");
		}
		invalidate_ea();
		return 1;
	}

	invalidate_ea();
	if (sz == 1) {
		load_t(value);
		puts("\tmpy ea,t");
		invalidate_t();
		invalidate_ea();
		load_ea_t();
		return 1;
	}
	/* Constant on right is positive - can use mpy */
	if (!(value & 0x8000)) {
		load_t(value);
		puts("\tmpy ea,t");
		invalidate_t();
		invalidate_ea();
		load_ea_t();
		return 1;
	}
	/* Can't shift to avoid problem - use helper */
	if (value & 1)
		return 0;
	/* Shift to keep right side positive */
	puts("\tsl ea");
	load_t(value >> 1);
	puts("\tmpy ea,t");
	invalidate_t();
	invalidate_ea();
	load_ea_t();
	return 1;
}

static unsigned gen_fast_div(unsigned s, struct node *n)
{
	unsigned v = WORD(n->right->value);
	if (v == 1)
		return 1;
	/* TODO */
	return 0;
}

/*
 *	Reference state for memory references
 */

static char ref_buf[64];
static unsigned ref_op;
static unsigned long ref_value;


/*
 *	Make p2 point to something. Used for operations that are done on
 *	an address (+= /= etc)
 */

static unsigned can_make_ptr_ref(struct node *n)
{
	switch(n->op) {
	case T_CONSTANT:
	case T_NAME:
	case T_LABEL:
	case T_ARGUMENT:
	case T_LOCAL:
		return 1;
	default:
		return 0;
	}
}


static unsigned make_ptr_ref(struct node *n, unsigned off)
{
	unsigned v = WORD(n->value);
	unsigned sz = get_size(n->type);

	ref_op = n->op;

	v += off;

	/* TODO check if in P2 or EA already */
	switch(n->op) {
	case T_CONSTANT:	/* Weird case but trivial */
		printf("\tld p2,=%u\n", v);
		/* Will do the right thing */
		ref_op = T_NREF;
		break;
	case T_NAME:
		printf("\tld p2,=_%s+%u\n", namestr(n->snum), v);
		ref_op = T_NREF;
		break;
	case T_LABEL:
		printf("\tld p2,=T%u+%u\n", n->val2, v);
		ref_op = T_LBREF;
		break;
	case T_ARGUMENT:
		v += ARGBASE + frame_len;
	case T_LOCAL:
		ref_op = T_LREF;
		v += sp;
		if (v < 128 - sz) {
			snprintf(ref_buf, sizeof(ref_buf), "%u+%%u,p1", v);
			return 1;
		}
		xch_ea_p2();
		load_ea_ptr(1);
		printf("\tadd ea,=%u\n", v);
		invalidate_ea();
		xch_ea_p2();
		break;
	default:
		return 0;
	}
	if (off == 0)
		set_ptr_ref(2, n);
	else
		invalidate_ptr(2);
	strcpy(ref_buf, "%u,p2");
	return 1;
}

/* Everything on the 8070 basically comes down to
	operator accumulator,offset,ptr
   or	operator accumulator,=constant

   This routine figures out how to build the reference to the
   desired object and prepare p2 if necessary

   Must preserve EA if told to, must preserve T always */

static unsigned can_make_src_ref(struct node *n)
{
	switch(n->op) {
	case T_CONSTANT:
	case T_NAME:
	case T_LABEL:
	case T_NREF:
	case T_LBREF:
	case T_LREF:
		return 1;
	default:
		return 0;
	}
}

static unsigned can_make_dst_ref(struct node *n)
{
	switch(n->op) {
	case T_CONSTANT:
	case T_NAME:
	case T_LABEL:
	case T_NSTORE:
	case T_LBSTORE:
	case T_LSTORE:
		return 1;
	default:
		return 0;
	}
}

static unsigned ref_needs_p2(struct node *n)
{
	unsigned v = WORD(n->value);
	unsigned s = get_size(n->type);

	switch(map_op(n->op)) {
	case T_LREF:
		v += sp;
		if (v + s >= 128)
			break;
		/* fall through */
	case T_NAME:
	case T_LABEL:
	case T_CONSTANT:
		return 0;
	}
	return 1;
}

static unsigned make_ref(struct node *n, unsigned keep_ea)
{
	unsigned v = WORD(n->value);
	unsigned s = get_size(n->type);
	unsigned p;

	p = find_ref(n);
	/* Already there */
	if (p == 2) {
		ref_op = T_NREF;	/* Works for this case */
		strcpy(ref_buf, "%u,p2");
		return 1;
	}

	ref_op = map_op(n->op);
	switch(ref_op) {
	case T_CONSTANT:
		/* Constants are special */
		ref_value = n->value;
		return 1;
	case T_NAME:
		snprintf(ref_buf, sizeof(ref_buf), "=%%s_%s+%u+%%u", namestr(n->snum), v);
		return 1;
	case T_LABEL:
		snprintf(ref_buf, sizeof(ref_buf), "=%%sT%u+%u+%%u", n->val2, v);
		return 1;
	case T_ARGUMENT:
	case T_LOCAL:
		/* Local is complicated */
		return 0;
	case T_NREF:
		printf("\tld p2,=_%s+%u\n", namestr(n->snum), v);
		break;
	case T_LBREF:
		printf("\tld p2,=T%u+%u\n", n->val2, v);
		break;
	case T_LREF:
		/* Simple lref */
		v += sp;
		if (v + s < 128) {
			snprintf(ref_buf, sizeof(ref_buf), "%u+%%u,p1", v);
			return 1;
		}
		/* Need to build p2 for it and preserve EA if needd */
		if (keep_ea)
			xch_ea_p2();
		load_ea_ptr(1);
		printf("\tadd ea,=%u\n", v);
		invalidate_ea();
		if (keep_ea)
			xch_ea_p2();
		else
			load_ptr_ea(2);
		break;
	default:
		return 0;
	}
	set_ptr_ref(2, n);
	strcpy(ref_buf, "%u,p2");
	return 1;
}

static void make_ref_sp(void)
{
	ref_op = T_NREF;
	strcpy(ref_buf, "%u,p1");
}

static void make_ref_p2(unsigned off)
{
	ref_op = T_NREF;
	snprintf(ref_buf, sizeof(ref_buf), "%u+%%u,p2", off);
}

static void make_ref_tmp(void)
{
	ref_op = T_NREF;
	strcpy(ref_buf, ":__tmp");
}

static void make_ref_constant(unsigned long v)
{
	ref_op = T_CONSTANT;
	ref_value = v;
}

static unsigned ref_constant(unsigned off, unsigned size)
{
	off *= 8;
	if (size == 2)
		return WORD(ref_value >> off);
	else
		return BYTE(ref_value >> off);
}

/* Perform an operation between A and the reference */
static void do_op8(const char *op, unsigned off)
{
	unsigned v;

	printf("\t%s a,", op);
	switch(ref_op) {
	case T_NAME:
	case T_LABEL:
		if (off > 1)
			puts("=0");
		else if (off == 1)
			printf(ref_buf, ">", off);
		else
			printf(ref_buf, "<", off);
		break;
	case T_LREF:
	case T_NREF:
	case T_LBREF:
		printf(ref_buf, off);
		break;
	case T_CONSTANT:
		v = ref_constant(off, 1);
		printf("=%u", v);
		break;
	default:
		error("do8");
	}
	putchar('\n');
}

/* Perform an operation between EA and the reference */
static void do_op16(const char *op, unsigned off, unsigned s)
{
	unsigned v;

	if (s > 1)
		printf("\t%s ea,", op);
	else
		printf("\t%s a,", op);

	switch(ref_op) {
	case T_NAME:
	case T_LABEL:
		if (off == 2)
			printf("=0");
		else
			printf(ref_buf, "", off);
		break;
	case T_LREF:
	case T_NREF:
	case T_LBREF:
	case T_LOCAL:
	case T_ARGUMENT:
		printf(ref_buf, off);
		break;
	case T_CONSTANT:
		v = ref_constant(off, 2);
		printf("=%u", v);
		break;
	default:
		error("do16");
	}
	putchar('\n');
}

static unsigned op16(const char *op, unsigned size, unsigned ldst, unsigned nr)
{
	if (ldst == O_STORE)
		flush_writeback();
	if (ldst == O_LOAD) {
		if (size == 4) {
			do_op16(op, 2, 2);
			invalidate_ea();
			store_ea_hireg();
			do_op16(op, 0, 2);
			return 0;
		}
		do_op16(op, 0, size);
		return 1;
	}
	if (nr) {
		if (size == 4) {
			do_op16(op, 0, 2);
			load_ea_hireg();
			do_op16(op, 2, 2);
			if (ldst != O_STORE)
				store_ea_hireg();
			return 0;
		}
		do_op16(op, 0, size);
		return 1;
	}
	if (size == 4) {
		do_op16(op, 0, 2);
		load_t_ea();
		load_ea_hireg();
		do_op16(op, 2, 2);
		if (ldst != O_STORE)
			store_ea_hireg();
		load_ea_t();
		return 0;
	}
	do_op16(op, 0, size);
	return 1;
}

static void do_op8pair(const char *op, unsigned off, unsigned size, unsigned nr)
{
	do_op8(op, off);
	if (size > 1) {
		if (!nr)
			xch_a_e();
		do_op8(op, off + 1);
		if (!nr)
			xch_a_e();
	}
}

static void op8(const char *op, unsigned size, unsigned nr)
{
	do_op8pair(op, 0, size, nr);
	if (size == 4) {
		if (!nr)
			load_t_ea();
		load_ea_hireg();
		do_op8pair(op, 2, size, nr);
		if (!nr) {
			store_ea_hireg();
			load_ea_t();
		}
	}
}

struct logic {
	const char *op;
	unsigned set16;
	unsigned null16;
	unsigned set8;
	unsigned null8;
};

static const struct logic logic[] = {
	/* Op    Set16,  Null16 Set8  Null8 */
	{ "and", 0x0000, 0xFFFF, 0x00, 0xFF },		/* AND */
	{ "or",  0xFFFF, 0x0000, 0xFF, 0x00 },		/* OR */
	{ "xor", 0x0000, 0x0000, 0x00, 0x00 }		/* XOR */
};

/* Logic operations we can optimize bits of with constant forms */
static void oplogic16(unsigned off, unsigned size, unsigned lt)
{
	const struct logic *l = logic + lt;
	unsigned v;

	v = ref_constant(off, 2);
	if (v == l->set16) {
		load_ea(2, l->set16);
		return;
	}

	v = ref_constant(off, 1);
	if (v != l->null8) {
		/* As cheap as loading anyway */
		do_op8(l->op, off);
		invalidate_a();
	}
	if (size > 1) {
		v = ref_constant(off + 1, 1);
		if (v != l->null8) {
			invalidate_e();
			xch_a_e();
			do_op8(l->op, off + 1);
			xch_a_e();
		}
	}
}

static unsigned oplogic(unsigned size, unsigned lt)
{
	const struct logic *l = logic + lt;
	unsigned v;

	if (ref_op != T_CONSTANT) {
		op8(l->op, size, 0);
		return 1;
	}
	v = ref_constant(0, 2);
	if (v != l->null16)
		oplogic16(0, size, lt);
	v = ref_constant(2, 2);

	if (size <= 2)
		return 1;

	if (v != l->null16) {
		load_t_ea();
		load_ea_hireg();
		oplogic16(2, size, lt);
		store_ea_hireg();
		load_ea_t();
	}
	return 1;
}

static unsigned eqlogic(register struct node *n, unsigned s, unsigned lt)
{
	/* We can do better than the main code generator as we can
	   often shove the left hand side directly into p2, or use off,p1 */
	if (can_make_ptr_ref(n->left)) {
		codegen_lr(n->right);
		make_ptr_ref(n->left, 0);
		/* Do the op between EA and the pointer */
		oplogic(s, lt);
		op16("st", s, O_STORE, (n->flags & NORETURN));
		return 1;
	}
	return 0;
}

/*
 *	EA holds the right, top of stack the left
 *	Set up and call a helper so we don't have messy stack
 *	mangling.
 */
static unsigned helper_stack(struct node *n, const char *op, unsigned t)
{
	unsigned s = get_size(t);
	if (s > 2)
		return 0;
	make_ref_tmp();
	op16("st", s, O_STORE, 0);
	puts("\tpop ea");
	gen_helpcall(n);
	printf("%s", op);
	helper_type(t, t & UNSIGNED);
	gen_helptail(n);
	putchar('\n');
	return 1;
}

/* Condition codes use the child node type as their own type is always int
   for the boolean result */
static unsigned cc_helper_stack(struct node *n, const char *op)
{
	n->flags |= ISBOOL;
	return helper_stack(n, op, n->right->type);
}

static unsigned op16_direct(struct node *n, const char *op, unsigned size, unsigned nr)
{
	/* Can we reference it */
	if (make_ref(n->right, 1) == 0)
		return 0;
	op16(op, size, O_MODIFY, nr);
	return 1;
}

/* TODO: what is the best way to do this, also do we do CCONLY handling
   using z/nz on A and just let A be 0x80 or 0x00 */
/* True if EA >= const */
static void uns_gteq_const(unsigned s)
{
	op16("sub", s, O_MODIFY, 1);
	invalidate_ea();

	if (ref_op == T_CONSTANT) {
		if (s == 2)
			adjust_ea(WORD(-ref_value));
		else
			adjust_a(BYTE(-ref_value));
	}
	if (optsize) {
		puts("\tjsr __unsignedcomp");
		set_e(0);
	} else {
		/* Now play games to get borrow flag */
		load_ea(2, 0);
		puts("\trrl a");/* Borrow is now top bit of A */
		puts("\tsl ea");/* Into low bit of E */
		xch_a_e();	/* Into low bit of A */
	}
	invalidate_a();
}

/* True if sign != overflow flag, that is if EA < const */
static void s_lt_const(unsigned s)
{
	op16("sub", s, O_MODIFY, 1);
	invalidate_ea();
	if (ref_op == T_CONSTANT) {
		if (s == 2)
			adjust_ea(WORD(-ref_value));
		else
			adjust_a(BYTE(-ref_value));
	}
	invalidate_a();
	if (optsize) {
		if (s == 1) 
			puts("\tjsr __signed8comp");
		else
			puts("\tjsr __signedcomp");
		set_e(0);
	} else {
		if (s == 1)
			xch_a_e();
		puts("\tld a,s");
		puts("\tsl a");		/* O is in top bit */
		puts("\txor a,e");	/* O xor sign of result */
		puts("\tsl ea");	/* And shuffle into EA as 0/1 */
		load_ea(1, 0);
		xch_a_e();
		puts("\tand a,=1");
	}
}


static unsigned gen_gtlt_op(struct node *n, unsigned z, unsigned gt, unsigned is_byte)
{
	register struct node *r = n->right;
	register unsigned w = WORD(r->value);
	unsigned s;

	if (z && r->op != T_CONSTANT)
		return 0;

	s = get_size(r->type);
	if (s > 2)
		return 0;

	if (!can_make_src_ref(r))
		return 0;

	make_ref(r, 0);


	if (is_byte)
		s = 1;

	printf(";sign %u z %u gt %u val %u\n", r->type & UNSIGNED, z, gt, w);
	if (r->type & UNSIGNED) {
		if (z && w == 0xFFFF)
			load_ea(2, !gt);
		else {
			if (z) {
				printf(";make ref constant %u\n", w + z);
				make_ref_constant(w + z);
			}
			uns_gteq_const(s);
			if (!gt)
				puts("\txor a,=1");
		}
	} else {
		if (z && w == 0x7FFF)
			load_ea(2, !gt);
		else {
			if (z)
				make_ref_constant(w + z);
			s_lt_const(s);
			if (gt)
				puts("\txor a,=1");
		}
	}
	n->flags |= ISBOOL;
	return 1;
}

static unsigned gen_eq_op(struct node *n, unsigned eq, unsigned is_byte)
{
	struct node *r = n->right;
	unsigned s = get_size(n->type);
	s = get_size(r->type);
	if (is_byte)
		s = 1;
	if (s > 2)
		return 0;
	if (!make_ref(r, 0))
		return 0;
	op16("sub", s, O_MODIFY, r->value);
	invalidate_ea();
	if (s == 2)
		puts("\tor a,e\n");
	printf("\tbz X%u\n", ++label);
	load_ea(2,1);
	printf("X%u:\n", label);
	invalidate_a();
	if (eq)
		puts("\txor a,=1");
	n->flags |= ISBOOL;
	return 1;
}

void shift_left(unsigned s, unsigned v)
{
	if (v >= 8) {
		v -= 8;
		puts("\tld e,a\n\tld a,=0");
	}
	if (s == 1) {
		repeated_op("\tsl a", v);
		invalidate_a();
	} else {
		repeated_op("\tsl ea", v);
		invalidate_ea();
	}
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 *
 *	We can do an awful lot of things this way and also for the stuff
 *	we cannot do this way shortcut a bunch of stuff via T to avoid stack
 *	traffic.
 */
unsigned gen_direct(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned v;
	unsigned op;
	unsigned nr = n->flags & NORETURN;
	unsigned is_byte = (n->flags & (BYTETAIL | BYTEOP)) == (BYTETAIL | BYTEOP);

	if (r)
		v = r->value;

	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		/* Need to decide who cleans up non vararg calls */
//		printf(";cleanup %u sp was %u now %u\n",
//			v, sp, sp - v);
		/* TODO: , 0 if called fn was void or result nr */
		gen_cleanup(v, 1);
		return 1;
	case T_EQ:
		n->value = 0;
	case T_EQPLUS:
		/* TODO: want to fold the add ea,= into the ref offset
		   ideally, but need to adjust helpers for that */
		printf(";eqplus consider r->op = %04X\n", r->op);
		if (can_make_src_ref(r) == 0)
			return 0;
		printf(";eqplus direct\n");
		if (WORD(n->value))
			printf("\tadd ea,=%u\n", WORD(n->value));
		if (ref_needs_p2(r)) {
			printf(";eqplus direct need p2\n");
			puts("\tpush ea");
			make_ref(r, 0);
			op16("ld", s, O_LOAD, 1);
			set_ea_node(r);
			puts("\tpop p2");
			invalidate_ptr(2);
		} else {
			printf(";eqplus direct not p2\n");
			invalidate_ea();
			invalidate_ptr(2);
			xch_ea_p2();
			make_ref(r, 0);
			op16("ld", s, O_LOAD, 1);
			set_ea_node(r);
		}
		printf(";eqplus assign\n");
		flush_writeback();
		make_ref_p2(0);
		op16("st", s, O_STORE, nr);
		return 1;
	case T_PLUS:
		if (s > 2)
			return 0;
		if (op16_direct(n, "add", s, nr) == 0)
			return 0;
		if (ref_op == T_CONSTANT) {
			if (s == 2)
				adjust_ea(WORD(ref_value));
			else
				adjust_a(BYTE(ref_value));
		} else
			invalidate_ea();
		return 1;
	case T_MINUS:
		if (s > 2)
			return 0;
		if (op16_direct(n, "sub", s, nr) == 0)
			return 0;
		if (ref_op == T_CONSTANT) {
			if (s == 2)
				adjust_ea(WORD(ref_value));
			else
				adjust_a(BYTE(ref_value));
		} else
			invalidate_ea();
		return 1;
	case T_STAR:	/* Multiply is a complicated mess */
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			/* Try shifts and MUL op inlined */
			if (s <= 2 && gen_fast_mul(s, v))
				return 1;
		}
		if (!can_make_src_ref(r))
			return 0;
		load_t_ea();
		make_ref(r, 0);
		op16("ld", s, O_LOAD, 1);
		/* MPY requires one side is 0 top bit sigh */
		puts("\tjsr __mpyfix");
		invalidate_ea();
		return 1;
	case T_SLASH:
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (s == 2 && (n->type & UNSIGNED) && r->op == T_CONSTANT && v == 256) {
				load_ea(1,0);
				xch_a_e();
				return 1;
			}
			if (gen_fast_div(s, n))
				return 1;
			/* Can we use the div instruction ? */
			/* This is true only for unsigned and positvie
			    divisor */
			if ((n->type & UNSIGNED) && !(v & 0x8000)) {
				invalidate_t();
				invalidate_ea();
				printf("\tld t,=%u\n\tdiv ea,t\n", v);
				return 1;
			}
		}
		/* TODO */
		break;
	case T_PERCENT:
		/* Mod 256 we can do easily */
		if (s == 2 && (r->type & UNSIGNED) && r->op == T_CONSTANT && v == 256) {
			load_e(0);
			return 1;
		}
		/* TODO */
		break;
	case T_AND:
		if (make_ref(r, 1) == 0)
			return 0;
		oplogic(s, 0);	/* 0 - AND */
		return 1;
	case T_OR:
		if (make_ref(r, 1) == 0)
			return 0;
		oplogic(s, 1);	/* 1 - OR */
		return 1;
	case T_HAT:
		if (make_ref(r, 1) == 0)
			return 0;
		oplogic(s, 2);	/* 2 - XOR */
		return 1;
	case T_EQEQ:
		return gen_eq_op(n, 1, is_byte);
	case T_BANGEQ:
		return gen_eq_op(n, 0, is_byte);
	case T_GT:
		return gen_gtlt_op(n, 1, 1, is_byte);
	case T_GTEQ:
		return gen_gtlt_op(n, 0, 1, is_byte);
	case T_LT:
		return gen_gtlt_op(n, 0, 0, is_byte);
	case T_LTEQ:
		return gen_gtlt_op(n, 1, 0, is_byte);
	case T_LTLT:
		/* TODO track shift result */
		if (r->op == T_CONSTANT) {
			if (s == 4) {
				if (v > 31)
					return 1;
				if (v >= 16) {
					shift_left(2, v - 16);
					store_ea_hireg();
					load_ea(2, 0);
					return 1;
				}
				return 0;
			}
			if (v > 15)
				return 1;
			if (v >= 8 && s == 1)
				return 1;
			shift_left(s, v);
			return 1;
		}
		break;
	case T_GTGT:
		if (s > 2)
			return 0;
		if (r->op != T_CONSTANT)
			return 0;
		if (v >= 16 && (n->type & UNSIGNED) && s == 4) {
			/* >> by over 16 we can shortcut nicely */
			load_ea_hireg();
			xch_ea_p2();
			load_ea(2, 0);
			store_ea_hireg();
			xch_ea_p2();
			v -= 16;
			s = 2;
		}
		/* TODO track shift result */
		if ((n->type & UNSIGNED) && s <= 2) {
			if (v > 15)
				return 1;
			if (v >= 8) {
				if (s == 1)
					return 1;
				v -= 8;
				/* No ld e,=0 so.. */
				load_ea(1, 0);
				xch_a_e();
			}
			if (s == 1)
				repeated_op("\tsr a", v);
			else
				repeated_op("\tsr ea", v);
			invalidate_ea();
			return 1;
		}
		break;
	case T_MINUSMINUS:
		v = -v;
	case T_PLUSPLUS:
		/* Complex ++/-- right is always constant, EA holds address */
		if (s > 2)
			return 0;
		load_ptr_ea(2);
		make_ref_p2(0);
		op16("ld", s, O_LOAD, 1);
		if (!nr)
			load_t_ea();
		make_ref_constant(v);
		op16("add", s, O_MODIFY, 1);
		make_ref_p2(0);
		op16("st", s, O_STORE, nr);
		if (!nr) {
			make_ref_constant(v);
			load_ea_t();
		}
		invalidate_ea();
		return 1;
	case T_ANDEQ:
		/* (EA) &= r */
		op = 0;
		goto doeleq;
	case T_OREQ:
		op = 1;
		goto doeleq;
	case T_HATEQ:
		op = 2;
	doeleq:
		/* EA holds the pointer */
		if (s > 2)
			return 0;
		/* If we can't make a reference to the value on the right we
		   give up */
		if (can_make_src_ref(r) == 0)
			return 0;
		/* If the reference uses p2 then go the long way */
		if (ref_needs_p2(r))
			return 0;
		puts("\txch ea, p2");
		make_ref_p2(0);
		op16("ld", s, O_LOAD, 1);
		make_ref(r, 1);
		oplogic(s, op);
		make_ref_p2(0);
		if (op16("st", s, O_STORE, nr))
			set_ea_node(n->left);
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
	unsigned s = get_size(n->type);
	unsigned nr = n->flags & NORETURN;
	unsigned se = n->flags & SIDEEFFECT;

	if (unreachable)
		return 1;

	switch(n->op) {
	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	case T_COMMA:
		l->flags |= NORETURN;
		codegen_lr(l);
		r->flags |= (n->flags & NORETURN);
		codegen_lr(r);
		return 1;
	case T_PLUSPLUS:	/* Always constant right */
		if (s > 2 || make_ptr_ref(l, 0) == 0)
			return 0;
		op16("ld", s, O_LOAD, 1);
		if (!nr)
			load_t_ea();
		if (s == 2)
			printf("\tadd ea, =%u\n", WORD(r->value));
		else
			printf("\tadd a, =%u\n", BYTE(r->value));
		op16("st", s, O_STORE, 1);
		if (!nr)
			load_ea_t();
		return 1;
	case T_MINUSMINUS:	/* Always constant right */
		if (s > 2 || make_ptr_ref(l, 0) == 0)
			return 0;
		op16("ld", s, O_LOAD, 1);
		if (!nr)
			load_t_ea();
		if (s == 2)
			printf("\tadd ea, =%u\n", WORD(-r->value));
		else
			printf("\tadd a, =%u\n", BYTE(-r->value));
		op16("st", s, O_STORE, 1);
		if (!nr)
			load_ea_t();
		return 1;
	case T_PLUSEQ:
		/* Use increment and load */
		if (s == 1 && r->op == T_CONSTANT && BYTE(r->value) == 1) {
			if (make_ptr_ref(l, 0)) {
				op8("ild", 1, 1);
				return 1;
			}
		}
		if (s > 2 || can_make_ptr_ref(l) == 0)
			return 0;
		/* If the right is easy then use the gen_direct path */
		if (can_make_src_ref(r))
			return 0;
		codegen_lr(r);		/* Value to add */
		make_ptr_ref(l, 0);
		op16("add", s, O_MODIFY, 1);
		op16("st", s, O_STORE, nr);
		if (!nr)
			set_ea_node(n);
		return 1;
	case T_MINUSEQ:
		if (s == 1 && r->op == T_CONSTANT && BYTE(r->value) == 1) {
			if (make_ptr_ref(l, 0)) {
				op8("dld", 1, 1);
				return 1;
			}
		}
		if (s != 2 || can_make_ptr_ref(l) == 0)
			return 0;
		/* If the right is easy then use the gen_direct path */
		if (can_make_src_ref(r))
			return 0;
		codegen_lr(r);
		make_ref_tmp();
		op16("st", s, O_STORE, 0);
		make_ptr_ref(l, 0);
		op16("ld", s, O_LOAD, 1);
		make_ref_tmp();
		op16("sub", s, O_MODIFY, 1);
		make_ptr_ref(l, 0);
		op16("st", s, O_STORE, nr);
		if (!nr)
			set_ea_node(n);
		return 1;
	case T_STAREQ:
		if (s > 2 || can_make_ptr_ref(l) == 0)
			return 0;
		if (r->op != T_CONSTANT)
			return 0;
		if (s == 2 && r->value & 0x8000)
			return 0;
		/* Can be done via mpy ea, t */
		make_ptr_ref(l, 0);
		make_ref_p2(0);
		if (s == 16)
			load_ea(2,0);
		op16("ld", s, O_LOAD, 1);
		if (!gen_fast_mul(s, r->value)) {
			load_t(r->value);
			puts("\tmpy ea,t");
			invalidate_ea();
			invalidate_t();
			load_ea_t();
		}
		op16("st", s, O_STORE, nr);
		return 1;
	case T_MINUS:
		if (nr && !se)
			return 1;
		if (s > 2 || can_make_src_ref(r) == 0)
			return 0;
		codegen_lr(l);	/* Write code to get the working value */
		make_ref(r, 1);	/* Keep EA and make reference */
		op16("sub", s, O_MODIFY, nr);
		invalidate_ea();
		return 1;
	case T_ANDEQ:
		return eqlogic(n, s, 0);
	case T_OREQ:
		return eqlogic(n, s, 1);
	case T_HATEQ:
		return eqlogic(n, s, 2);
	case T_EQ:
		n->value = 0;
	case T_EQPLUS:
		/* Handle the case of simple=complex */
		if (can_make_ptr_ref(l)) {
			printf(";eqplus short\n");
			codegen_lr(r);
			printf(";eqplus short l\n");
			make_ptr_ref(l, WORD(n->value));
			op16("st", s, O_STORE, nr);
			return 1;
		}
		break;
	}
	return 0;
}

static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned ls;

	if (PTR(rt))
		rt = USHORT;
	if (PTR(lt))
		lt = USHORT;

	/* Floats and stuff handled by helper */
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		return 0;

	/* No type casting needed as computing byte sized */
	if (n->flags & BYTEOP)
		return 1;

	ls = get_size(lt);

	/* Size shrink is free */
	if ((lt & ~UNSIGNED) <= (rt & ~UNSIGNED))
		return 1;
	/* Don't do the harder ones */
	if (!(rt & UNSIGNED) || ls > 2)
		return 0;
	load_e(0);
	return 1;
}

static void pop_p2(void)
{
	puts("\tpop p2");
	invalidate_ptr(2);
}

unsigned logic_ptr_op(unsigned sz, unsigned n, unsigned nr)
{
	pop_p2();
	make_ref_p2(0);
	oplogic(sz, n);
	op16("st", sz, O_STORE, nr);
	return 1;
}

unsigned gen_node(struct node *n)
{
	struct node *r = n->right;
	unsigned sz = get_size(n->type);
	unsigned v;
	int off;
	unsigned nr = n->flags & NORETURN;
	unsigned is_byte = (n->flags & (BYTETAIL | BYTEOP)) == (BYTETAIL | BYTEOP);
	unsigned se = n->flags & SIDEEFFECT;

	/* We adjust sp so track the pre-adjustment one too when we need it */

	v = n->value;

	/* An operation with a left hand node will have the left stacked
	   and the operation will consume it so adjust the stack.

	   The exception to this is comma and the function call nodes
	   as we leave the arguments pushed for the function call */

	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_stack_size(n->left->type);

	switch(n->op) {
		/* We need a pointer to all objects so these come out
		   the same */
	case T_LREF:
	case T_NREF:
	case T_LBREF:
		/* Kill unused ref if non volatile */
		if (nr && !se)
			return 1;
		/* Already loaded and not volatile */
		if (!se && is_ea_node(n))
			return 1;
		if (make_ref(n, 0) == 0)
			return 0;
		if (op16("ld", sz, O_LOAD, nr))
			set_ea_node(n);
		return 1;
	case T_NSTORE:
	case T_LBSTORE:
	case T_LSTORE:
		/* Already is ? */
		if (!se && is_ea_node(n))
			return 1;
		if (make_ref(n, 1) == 0)
			return 0;
		flush_writeback();
		if (op16("st", sz, O_STORE, nr))
			set_ea_node(n);
		return 1;
	case T_CALLNAME:
		flush_writeback();
		invalidate_all();
		printf("\tjsr _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_EQ:
		n->value = 0;
	case T_EQPLUS:
		printf(";eqplus hard\n");
		/* *TOS = (hireg:)EA */
		off = WORD(n->value);
		pop_p2();
		make_ref_p2(off);
		flush_writeback();
		op16("st", sz, O_STORE, nr);
		return 1;
	case T_DEREF:
	case T_DEREFPLUS:
		/* This is an odd one. We can't randomly assume a deref is
		   safe to do byte size if forced bytesize if the source is
		   volatile */
		if (nr && !se)
			return 1;
		if (!se && is_byte)
			sz = 1;
		/* Might be able to be smarter here */
		flush_writeback();
		load_ptr_ea(2);
		make_ref_p2(v);
		op16("ld", sz, O_LOAD, nr);
		/* TODO node track */
		invalidate_ea();
		return 1;
	case T_LDEREF:
		/* TODO: review for volatile byteable */
		/* val2 offset of variable, val offset of ptr */
		/* We cannot alas do a straight ptr of ptr load, but must
		   go via EA. No problem here as we will trash EA anyway */
		printf("\tld ea,%u,p1\n", v + sp);
		invalidate_ea();
		load_ptr_ea(2);
		make_ref_p2(n->val2);
		op16("ld", sz, O_LOAD, nr);
		/* TODO node track */
		invalidate_ea();
		return 1;
	case T_LEQ:
		printf(";LEQ\n");
		/* TODO: review for volatile byteable */
		/* Must go via EA which is messier because of course
		   we have a value in EA right now */
		load_t_ea();
		printf("\tld ea,%u,p1\n", v + sp);
		invalidate_ea();
		load_ptr_ea(2);
		load_ea_t();
		flush_writeback();
		make_ref_p2(n->val2);
		op16("st", sz, O_STORE, nr);
		/* TODO node track */
		invalidate_ea();
		return 1;
	case T_FUNCCALL:
		flush_writeback();
		invalidate_all();
		/* EA holds the function ptr */
		puts("\tjsr __callea\n");
		return 1;
	case T_CONSTANT:
		if (is_byte)
			sz = 1;
	case T_LABEL:
	case T_NAME:
		if (nr)
			return 1;
		make_ref(n, 0);
		op16("ld", sz, O_LOAD, nr);
		set_ea_node(n);
		return 1;
	case T_ARGUMENT:
		v += frame_len + ARGBASE;
	case T_LOCAL:
		if (nr)
			return 1;
		v += sp;
		load_ea_ptr(1);
		if (v)
			printf("\tadd ea,=%d\n", v);
		set_ea_node(n);
		return 1;
	case T_CAST:
		return gen_cast(n);
	case T_TILDE:
		make_ref_constant(0xFFFFFFFFUL);
		oplogic(sz, 2);	/* Do the XOR */
		invalidate_ea();	/* FIXME: logic result tracking */
		return 1;
	case T_NEGATE:
		if (sz > 2)
			return 0;
		make_ref_constant(0xFFFFFFFFUL);
		oplogic(sz, 2);	/* Do the XOR */
		invalidate_ea();	/* FIXME: logic result tracking */
		make_ref_constant(1);
		op16("add", sz, O_MODIFY, nr);
		return 1;
	case T_PLUS:
		if (sz > 2)
			return 0;
		make_ref_sp();
		op16("add", sz, O_MODIFY, nr);
		discard_words(sz);
		invalidate_ea();
		return 1;
	case T_MINUS:
		/* TOS - EA */
		if (sz > 2)
			return 0;
		make_ref_tmp();
		op16("st", sz, O_STORE, 0);
		puts("\tpop ea");
		invalidate_ea();
		op16("sub", sz,O_MODIFY, nr);
		invalidate_ea();
		return 1;
	case T_STAR:
		if (sz > 2)
			return 0;
		puts("\tld t,ea");
		puts("\tpop ea");
		puts("\tjsr __mpyfix");
		invalidate_ea();
		return 1;
	case T_SLASH:
/*		if (helper_stack(n, "divtmp", n->type)) seems easier without
			return 1; */
		return 0;
	case T_AND:
		make_ref_sp();
		oplogic(sz, 0);
		discard_words(sz);
		return 1;
	case T_OR:
		make_ref_sp();
		oplogic(sz, 1);
		discard_words(sz);
		return 1;
	case T_HAT:
		make_ref_sp();
		oplogic(sz, 2);
		discard_words(sz);
		return 1;
	case T_BANG:
		/* Common case of !(boolstuff) */
		if (n->right->flags & ISBOOL) {
			n->flags |= ISBOOL;
			puts("\txor a,=1");
			/* Can't be a pointer as is bool */
			a_value ^= 1;
			return 1;
		}
		n->flags |= ISBOOL;
		if (n->right->flags & ISBOOL)
			return 1;
		sz = get_size(r->type);
		if (sz > 2)
			return 0;
		if (sz == 2)
			puts("\tor a,e");
		printf("\tbz X%u\n", ++label);
		load_ea(2, 1);
		printf("X%u:\n", label);
		puts("\txor a,=1");
		return 1;
	case T_BOOL:
		n->flags |= ISBOOL;
		if (n->right->flags & ISBOOL)
			return 1;
		sz = get_size(r->type);
		if (sz > 2)
			return 0;
		if (sz == 2)
			puts("\tor a,e");
		printf("\tbz X%u\n", ++label);
		load_ea(2, 1);
		printf("X%u:\n", label);
		return 1;
	/* Shift TOS by EA */
	case T_LTLT:
	/* Need a variant that stacks the other way around ?
		if (helper_stack(n, "shltmp", n->type))
			return 1; */
		return 0;
	case T_GTGT:
	/* Need a variant that stacks the other way around ?
		if (helper_stack(n, "shrtmp", n->type))
			return 1; */
		return 0;

	/* eq operations, turn what we can into tmpops for speed */
	case T_PLUSEQ:
		if (sz > 2)
			return 0;
		pop_p2();
		/* EA holds the value, p2 the ptr */
		make_ref_p2(0);
		op16("add", sz, O_MODIFY, 1);
		op16("st", sz, O_STORE, nr);
		invalidate_ea();
		return 1;
	case T_MINUSEQ:
		if (sz > 2)
			return 0;
		pop_p2();
		/* This one is backwards so trickier */
		make_ref_tmp();
		op16("st", sz, O_STORE, 0);
		/* EA holds the value, p2 the ptr */
		make_ref_p2(0);
		op16("ld", sz, O_LOAD, 1);
		make_ref_tmp();
		op16("sub", sz, O_MODIFY, 1);
		make_ref_p2(0);
		op16("st", sz, O_STORE, nr);
		invalidate_ea();
		return 1;
	case T_STAREQ:
		/* We can do byte sized muleq fast. Constant
		   stuff is picked off in gen_direct first */
		if (sz == 1) {
			pop_p2();	 /* Pointer */
			make_ref_p2(0);
			load_t_ea();
			load_ea(2, 0);	/* Ensure upper byte is clear */
			op16("ld", 1, O_LOAD, 1);
			puts("\tmpy ea,t");
			/* Result is in T */
			invalidate_ea();
			invalidate_t();
			load_ea_t();
			op16("st", 1, O_STORE, nr);
			return 1;
		}
		if (helper_stack(n, "muleqtmp", n->type))
			return 1;
		return 0;
	case T_SLASHEQ:
/*		if (helper_stack(n, "diveqtmp", n->type))
			return 1; */
		return 0;
	case T_PERCENTEQ:
/*		if (helper_stack(n, "modeqtmp", n->type))
			return 1;*/
		return 0;
	/* TOS is the pointer, EA is the value */
	/* TODO: logic fallbacks ? */
	case T_ANDEQ:
		return logic_ptr_op(sz, 0, nr);
	case T_OREQ:
		return logic_ptr_op(sz, 1, nr);
	case T_HATEQ:
		return logic_ptr_op(sz, 2, nr);
	/* Conditions */
	case T_EQEQ:
		if (cc_helper_stack(n, "cceqtmp"))
			return 1;
		return 0;
	case T_GTEQ:
		if (cc_helper_stack(n, "ccgteqtmp"))
			return 1;
		return 0;
	case T_GT:
		if (cc_helper_stack(n, "ccgttmp"))
			return 1;
		return 0;
	case T_LTEQ:
		if (cc_helper_stack(n, "cclteqtmp"))
			return 1;
		return 0;
	case T_LT:
		if (cc_helper_stack(n, "cclttmp"))
			return 1;
		return 0;
	case T_BANGEQ:
		if (cc_helper_stack(n, "ccnetmp"))
			return 1;
		return 0;
	}
	return 0;
}
