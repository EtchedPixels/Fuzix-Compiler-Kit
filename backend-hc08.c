/*
 *	HC08 backend for the Fuzix C Compiler
 *
 *	Register usage
 *	A: lower half of working value
 *	X: upper half of working value or low half of pointer
 *	H: upper half of pointer
 *	@tmp: scratch value used extensively
 *	@tmp2: temporary word following @tmp
 *	@hireg: upper 16bits of 32bit working values
 *
 *	In many respects the 6502 and 68HC08 are very similar. We however
 *	have a proper stack and index register which makes life somewhat
 *	easier.
 *
 *	We are a bit naiive in that we use ,SP forms always whereas it would
 *	often be shorter to stuff SP into HX and remember the fact so we can
 *	use the byte shorter ,X modes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "compiler.h"
#include "backend.h"
#include "backend-byte.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

#define USE_HX		0x0080

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned unreachable;	/* Code following an unconditional jump */
static unsigned xlabel;		/* Internal backend generated branches */
static unsigned argbase = 2;	/* Track shift between arguments and stack */

static uint8_t hx_live;		/* Set when H:X holds a pointer rather than X:A
				   data */
/*
 *	Node types we create in rewriting rules
 */
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
#define T_REQ		(T_USER+10)		/* *regptr */


/*
 *	68HC08 specifics. We need to track some register values to produce
 *	bearable code
 */

static void output(const char *p, ...)
{
	va_list v;
	va_start(v, p);
	putchar('\t');
	vprintf(p, v);
	putchar('\n');
	va_end(v);
}

static void label(const char *p, ...)
{
	va_list v;
	va_start(v, p);
	vprintf(p, v);
	putchar(':');
	putchar('\n');
	va_end(v);
}


#define R_A	0
#define R_X	1
#define R_H	2

#define INVALID	0

struct regtrack {
	unsigned state;
	uint8_t value;
	unsigned snum;
	unsigned offset;
};

static struct regtrack reg[3];

static void invalidate_regs(void)
{
	reg[R_A].state = INVALID;
	reg[R_X].state = INVALID;
	reg[R_H].state = INVALID;
}


static void invalidate_a(void)
{
	reg[R_A].state = INVALID;
}

static void invalidate_x(void)
{
	reg[R_X].state = INVALID;
}

static void invalidate_hx(void)
{
	reg[R_H].state = INVALID;
	reg[R_X].state = INVALID;
}

static void const_a_set(unsigned val)
{
	if (reg[R_A].state == T_CONSTANT)
		reg[R_A].value = val;
	else
		reg[R_A].state = INVALID;
}

static void const_x_set(unsigned val)
{
	if (reg[R_X].state == T_CONSTANT)
		reg[R_X].value = val;
	else
		reg[R_X].state = INVALID;
}

/* Get a value into A, adjust and track */
static void load_a(uint8_t n)
{
	if (reg[R_A].state == T_CONSTANT) {
		if (reg[R_A].value == n)
			return;
	}
	/* No inca deca */
	else if (n == 0)
		output("clra");
	else if (reg[R_X].state == T_CONSTANT && reg[R_X].value == n)
		output("txa");
	else
		output("lda #%u", n);
	reg[R_A].state = T_CONSTANT;
	reg[R_A].value = n;
}

/* Get a value into X, adjust and track */
static void load_x(uint8_t n)
{
	if (reg[R_X].state == T_CONSTANT) {
		if (reg[R_X].value == n)
			return;
		if (reg[R_X].value == n - 1) {
			output("inx");
			reg[R_X].value++;
			return;
		}
		if (reg[R_X].value == n + 1) {
			output("dex");
			reg[R_X].value--;
			return;
		}
	}
	else if (n == 0)
		output("clrx");
	else if (reg[R_A].state == T_CONSTANT && reg[R_A].value == n)
		output("tax");
	else
		output("ldx #%u", n);
	reg[R_X].state = T_CONSTANT;
	reg[R_X].value = n;
}

/* Get a value into H:X, adjust and track */
static void load_hx(uint16_t n)
{
	if (reg[R_X].state == T_CONSTANT && reg[R_H].state == T_CONSTANT) {
		if (reg[R_X].value == (n & 0xFF) && reg[R_H].value == (n >> 8))
			return;
	}
	if (n == 0) {
		output("clrh");
		output("clrx");
	} else
		output("ldhx #%u", n);
	reg[R_X].state = T_CONSTANT;
	reg[R_X].value = n & 0xFF;
	reg[R_H].state = T_CONSTANT;
	reg[R_H].value = n >> 8;
}

/*
 *	H:X is used for pointer work but is not the same as XA for integers
 *	This is a pain but not something we can do much about as it's built
 *	into the CPU design
 */
static void set_hx_node(struct node *n)
{
	unsigned op = n->op;
	unsigned value = n->value;

	/* Turn store forms into ref forms */
	switch(op) {
	case T_NSTORE:
		op = T_NREF;
		break;
	case T_LBSTORE:
		op = T_LBREF;
		break;
	case T_LSTORE:
		op = T_LREF;
		break;
	case T_NAME:
	case T_CONSTANT:
	case T_NREF:
	case T_LBREF:
	case T_LREF:
	case T_LOCAL:
	case T_ARGUMENT:
		break;
	default:
		invalidate_a();
		invalidate_x();
		return;
	}
	reg[R_H].state = op;
	reg[R_X].state = op;
	reg[R_H].value = value >> 8;
	reg[R_X].value = value & 0xFF;
	reg[R_H].snum = n->snum;
	reg[R_X].snum = n->snum;
	return;
}

static unsigned hx_contains(struct node *n)
{
	if (n->op == T_NREF && (n->flags & SIDEEFFECT))		/* Volatiles */
		return 0;
	if (reg[R_H].state != n->op || reg[R_X].state != n->op)
		return 0;
	if (reg[R_H].value != (n->value >> 8) || reg[R_X].value != (n->value & 0xFF))
		return 0;
	if (reg[R_H].snum != n->snum || reg[R_X].snum != n->snum)
		return 0;
	/* Looks good */
	return 1;
}

static void set_xa_node(struct node *n)
{
	unsigned op = n->op;
	unsigned value = n->value;

	/* Turn store forms into ref forms */
	switch(op) {
	case T_NSTORE:
		op = T_NREF;
		break;
	case T_LBSTORE:
		op = T_LBREF;
		break;
	case T_LSTORE:
		op = T_LREF;
		break;
	case T_NAME:
	case T_CONSTANT:
	case T_NREF:
	case T_LBREF:
	case T_LREF:
	case T_LOCAL:
	case T_ARGUMENT:
		break;
	default:
		invalidate_a();
		invalidate_x();
		return;
	}
	reg[R_X].state = op;
	reg[R_A].state = op;
	reg[R_X].value = value >> 8;
	reg[R_A].value = value & 0xFF;
	reg[R_X].snum = n->snum;
	reg[R_A].snum = n->snum;
	return;
}

static unsigned xa_contains(struct node *n)
{
	if (n->op == T_NREF && (n->flags & SIDEEFFECT))		/* Volatiles */
		return 0;
	if (reg[R_X].state != n->op || reg[R_A].state != n->op)
		return 0;
	if (reg[R_X].value != (n->value >> 8) || reg[R_A].value != (n->value & 0xFF))
		return 0;
	if (reg[R_X].snum != n->snum || reg[R_A].snum != n->snum)
		return 0;
	/* Looks good */
	return 1;
}

static void set_a_node(struct node *n)
{
	unsigned op = n->op;
	unsigned value = n->value;

	switch(op) {
	case T_NSTORE:
		op = T_NREF;
		break;
	case T_LBSTORE:
		op = T_LBREF;
		break;
	case T_LSTORE:
		op = T_LREF;
		break;
	case T_NAME:
	case T_CONSTANT:
	case T_NREF:
	case T_LBREF:
	case T_LREF:
	case T_LOCAL:
	case T_ARGUMENT:
		break;
	default:
		invalidate_a();
		return;
	}
	reg[R_A].state = op;
	reg[R_A].value = value;
	reg[R_A].snum = n->snum;
}

static unsigned a_contains(struct node *n)
{
	if (reg[R_A].state != n->op)
		return 0;
	if (reg[R_A].value != n->value)
		return 0;
	if (reg[R_A].snum != n->snum)
		return 0;
	/* Looks good */
	return 1;
}



/* Memory writes occured, invalidate according to what we know. Passing
   NULL indicates unknown memory changes */

#if 0
static void invalidate_node(struct node *n)
{
	/* For now don't deal with the complex cases of whether we might
	   invalidate another object */
	if (reg[R_A].state != T_CONSTANT)
		reg[R_A].state = INVALID;
	if (reg[R_X].state != T_CONSTANT)
		reg[R_X].state = INVALID;
}
#endif

static void invalidate_mem(void)
{
	if (reg[R_A].state != T_CONSTANT)
		reg[R_A].state = INVALID;
	if (reg[R_X].state != T_CONSTANT)
		reg[R_X].state = INVALID;
	if (reg[R_H].state != T_CONSTANT)
		reg[R_H].state = INVALID;
}

static void set_reg(unsigned r, unsigned v)
{
	reg[r].state = T_CONSTANT;
	reg[r].value = (uint8_t)v;
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

/*
 *	For HC08 we keep byte objects byte size
 */
static unsigned get_stack_size(unsigned t)
{
	return get_size(t);
}


/* Generate a call to an internal helper */
static void gen_internal(const char *p)
{
	invalidate_regs();
	output("jsr __%s", p);
	hx_live = 0;
}

static void repeated_op(unsigned n, const char *o)
{
	while(n--)
		output(o);
}

static void use_xa(unsigned n)
{
	if (hx_live) {
		printf(";use_xa %u\n", n);
		output("sthx @tmp");
		output("txa");
		output("ldx @tmp");
		hx_live = 0;
	}
}

static void use_hx(unsigned n)
{
	if (hx_live == 0) {
		printf(";use_hx %u\n", n);
		output("stx @tmp");
		output("sta @tmp+1");
		output("ldhx @tmp");
		hx_live = 1;
	}
}

static void adjust_hx(int v)
{
	invalidate_hx();
	while(v >= 127) {
		output("aix #127");
		v -= 127;
	}
	while(v < -128) {
		output("aix #-128");
		v += 128;
	}
	if (v)
		output("aix #%d", v);
}

static void adjust_sp(int v)
{
	while(v >= 127) {
		output("ais #127");
		v -= 127;
	}
	while(v < -128) {
		output("ais #-128");
		v += 128;
	}
	if (v)
		output("ais #%d", v);
}


/*
 *	For endian reasons these are a bit differennt to the 6502. We expect
 *	the caller to deal with any endian biasing of offsets
 */
/* Construct a direct operation if possible for the primary op */
static int do_pri8(struct node *n, const char *op, void (*pre)(struct node *__n), unsigned bias)
{
	struct node *r = n->right;
	const char *name;
	unsigned v = n->value;

	/* We can fold in some simple casting */
	if (n->type == T_CAST) {
		if ((!PTR(n->type) && n->type != CINT && n->type != UINT) || r->type != UCHAR)
			return 0;
		/* Just do the right hand side */
		n = n->right;
		v = n->value;
		r = n->right;
	}

	switch(n->op) {
	case T_LABEL:
		pre(n);
		output("%s #<T%d+%d", op,  n->val2, v);
		return 1;
	case T_NAME:
		pre(n);
		name = namestr(n->snum);
		output("%s #<_%s+%d", op,  name, v);
		return 1;
	case T_CONSTANT:
		/* These had the right squashed into them */
	case T_LREF:
	case T_NREF:
	case T_LBREF:
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
		/* These had the right squashed into them */
		r = n;
		break;
	}

	v = r->value;

	switch(r->op) {
	case T_CONSTANT:
		pre(n);
		if (strcmp(op, "lda") == 0)
			load_a(v);
		else if (strcmp(op, "ldx") == 0)
			load_x(v);
		else
			output("%s #%d", op, r->value & 0xFF);
		return 1;
	case T_LREF:
	case T_LSTORE:
		pre(n);
		output("%s %u,sp", op, v + bias + sp);
		return 1;
	case T_NREF:
	case T_NSTORE:
		pre(n);
		name = namestr(r->snum);
		output("%s _%s+%d", op,  name, (unsigned)r->value + bias);
		return 1;
	case T_LBSTORE:
	case T_LBREF:
		pre(n);
		output("%s T%d+%d", op,  r->val2, (unsigned)r->value + bias);
		return 1;
	/* If we add registers
	case T_RREF:
		output("%s __reg%d", op, r->val2);
		return 1;*/
	}
	return 0;
}

/* Construct a direct operation if possible for the primary op */
static int do_pri8hi(struct node *n, const char *op, void (*pre)(struct node *__n))
{
	struct node *r = n->right;
	const char *name;
	unsigned v = n->value;

	use_xa(0);

	/* We can fold in some simple casting */
	if (n->type == T_CAST) {
		if ((!PTR(n->type) && n->type != CINT && n->type != UINT) || r->type != UCHAR)
			return 0;
		/* We need to do it on 0 */
		load_a(0);
		n = n->right;
		v = n->value;
		r = n->right;
	}
	switch(n->op) {
	case T_LABEL:
		pre(n);
		output("%s #>T%d+%d", op,  n->val2, v);
		return 1;
	case T_NAME:
		pre(n);
		name = namestr(n->snum);
		output("%s #_%s+%d", op,  name, v);
		return 1;
	case T_CONSTANT:
		/* These had the right squashed into them */
	case T_LREF:
	case T_NREF:
	case T_LBREF:
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
		/* These had the right squashed into them */
		r = n;
		break;
	}

	v = r->value;

	switch(r->op) {
	case T_CONSTANT:
		pre(n);
		v >>= 8;
		if (strcmp(op, "lda") == 0)
			load_a(v);
		else if (strcmp(op, "ldx") == 0)
			load_x(v);
		else
			output("%s #%d", op, v);
		return 1;
	case T_LREF:
	case T_LSTORE:
		pre(n);
		printf(";8hi lref %u sp %u\n", v, sp);
		output("%s %u,sp", op, v + sp);
		return 1;
	case T_NREF:
	case T_NSTORE:
		pre(n);
		name = namestr(r->snum);
		output("%s _%s+%d", op,  name, v);
		return 1;
	case T_LBSTORE:
	case T_LBREF:
		pre(n);
		output("%s T%d+%d", op,  r->val2, v);
		return 1;
	/* If we add registers
	case T_RREF:
		output("%s __reg%d+1", op, r->val2);
		return 1;*/
	}
	return 0;
}

/* 16bit: We are rather limited here because we only have a few ops with x */
static int do_pri16(struct node *n, const char *op, void (*pre)(struct node *__n))
{
	struct node *r = n->right;
	const char *name;
	unsigned v = n->value;

	/* We can fold in some simple casting */
	if (n->type == T_CAST) {
		use_xa(10);
		if ((!PTR(n->type) && n->type != CINT && n->type != UINT) || r->type != UCHAR)
			return 0;
		/* Just do the right hand side */
		n = n->right;
		v = n->value;
		r = n->right;
		load_x(0);
	}

	switch(n->op) {
	case T_LABEL:
		use_xa(11);
		pre(n);
		output("%sa #<T%d+%d", op,  n->val2, v);
		output("%sx #>T%d+%d", op,  n->val2, v >> 8);
		return 1;
	case T_NAME:
		use_xa(12);
		pre(n);
		name = namestr(n->snum);
		output("%sa #<_%s+%d", op,  name, v);
		output("%sx #>_%s+%d", op,  name, v >> 8);
		return 1;
	case T_LOCAL:
	case T_LREF:
	case T_NREF:
	case T_LBREF:
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
	case T_CONSTANT:
		/* These had the right squashed into them */
		r = n;
	}

	v = r->value;

	if (get_size(r->type) != 2)
		error("pri16bt");
	switch(r->op) {
	case T_CONSTANT:
		use_xa(13);
		pre(n);
		if (strcmp(op, "ld") == 0) {
			load_a(v);
			load_x(v >> 8);
		} else {
			output("%sa #%u", op, v & 0xFF);
			output("%sx #%u", op, v >> 8);
		}
		return 1;
	case T_LREF:
		if (strcmp(op, "ld") == 0) {
			use_xa(1);
			pre(n);
			v += sp;
			output("ldx %u,sp\n", v);
			output("lda %u,sp\n", v + 1);
			return 1;
		}
	case T_LSTORE:
		use_xa(14);
		v += sp;
		pre(n);
		output("%sa %u, sp", op, v);
		output("tax");
		output("%sa %u, sp", op, v + 1);
		return 1;
	case T_NSTORE:
	case T_NREF:
		use_xa(15);
		name = namestr(r->snum);
		pre(n);
		output("%sa _%s+%d", op,  name, (unsigned)r->value);
		output("%sx _%s+%d", op,  name, ((unsigned)r->value) + 1);
		return 1;
	case T_LBSTORE:
	case T_LBREF:
		use_xa(16);
		pre(n);
		output("%sa T%d+%d", op,  r->val2, (unsigned)r->value);
		output("%sx T%d+%d", op,  r->val2, ((unsigned)r->value) + 1);
		return 1;
	/* If we add registers
	case T_RREF:
		pre(n);
		output("%sa __reg%dd", op, r->val2);
		output("%sx __reg%d + 1", op,  r->val2);
		return 1;*/
	}
	return 0;
}

/* 16bit via H:X. Only immediate and direct forms for load/store */
static int do_hxpri16(struct node *n, const char *op, void (*pre)(struct node *__n))
{
	struct node *r = n->right;
	const char *name;
	unsigned v = n->value;

	switch(n->op) {
	case T_LABEL:
		pre(n);
		output("%shx #T%d+%d", op,  n->val2, v);
		hx_live = 1;
		return 1;
	case T_NAME:
		pre(n);
		name = namestr(n->snum);
		output("%shx #_%s+%d", op,  name, v);
		hx_live = 1;
		return 1;
	case T_LOCAL:
	case T_LREF:
	case T_NREF:
	case T_LBREF:
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
	case T_CONSTANT:
		/* These had the right squashed into them */
		r = n;
	}

	v = r->value;

	if (get_size(r->type) != 2)
		error("hxpri16bt");
	switch(r->op) {
	case T_CONSTANT:
		pre(n);
		load_hx(v);
		hx_live = 1;
		return 1;
	case T_ARGUMENT:
		v += frame_len + argbase;
	case T_LOCAL:
		v += sp;
		pre(n);
		output("tsx");
		adjust_hx(v);
		hx_live = 1;
		return 1;
	}
	return 0;
}

static void pre_none(struct node *n)
{
}

static void pre_txa(struct node *n)
{
	output("txa");
	hx_live = 0;
}

static void pre_store8(struct node *n)
{
	if (hx_live)
		output("stx @tmp");
	else
		output("sta @tmp");
	hx_live = 0;
}

static void pre_store16(struct node *n)
{
	if (hx_live)
		output("sthx @tmp");
	else {
		output("sta @tmp");
		output("stx @tmp+1");
	}
	hx_live = 0;
}

static void pre_store16clx(struct node *n)
{
	if (hx_live) { 
		output("sthx @tmp");
		output("txa");
		memcpy(&reg[R_A], &reg[R_X], sizeof(struct regtrack));
		hx_live = 0;
	} else {
		output("sta @tmp");
		output("stx @tmp+1");
	}
	load_x(0);
	
}

static void pre_psha(struct node *n)
{
	output("psha");
	sp++;
}

static void pre_hx(struct node *n)
{
	use_hx(0);
}

static int pri8(struct node *n, const char *op)
{
	return do_pri8(n, op, pre_none, 0);
}

static int pri16(struct node *n, const char *op)
{
	return do_pri16(n, op, pre_none);
}

static unsigned fast_castable(struct node *n)
{
	struct node *r = n->right;
	/* Is this a case we can just flow into the code. Usually that's
	   a uchar to int */
	if (r->op == T_CAST && get_size(r->type) == 2 && r->right->type == UCHAR)
		return 1;
	return 0;
}

static int pri8_help(struct node *n, char *helper)
{
	struct node *r = n->right;
	/* Special case for cast first */
	if (fast_castable(n)) {
		if (do_pri8(r->right, "lda", pre_store8, 0)) {
			helper_s(n, helper);
			return 1;
		}
	}
	if (do_pri8(r, "lda", pre_store8, 0)) {
		/* Helper invalidates A itself */
		helper_s(n, helper);
		return 1;
	}
	return 0;
}

static void pre_fastcast(struct node *n)
{
	if (hx_live) {
		output("sthx @tmp");
		hx_live = 0;
	} else {
		output("stx @tmp");
		output("sta @tmp+1");
	}
}

static void pre_fastcastx0(struct node *n)
{
	if (hx_live) {
		output("stx @tmp+1");
		hx_live = 0;
	} else
		output("sta @tmp+1");
	output("clr @tmp");
}

static int pri16_help(struct node *n, char *helper)
{
	struct node *r = n->right;
	unsigned v = r->value;
	unsigned s = get_size(r->type);

	printf(";pri16_help %x %s\n", n->op, helper);
	/* Special case for cast first */
	if (fast_castable(n)) {
		if (get_size(r->right->type) == 2) {
			if (do_pri16(r->right, "ld", pre_fastcast)) {
				helper_s(n, helper);
				return 1;
			}
		} else {
			if (do_pri8(r->right, "ld", pre_fastcastx0, 1)) {
				helper_s(n, helper);
				return 1;
			}
		}
	}
	if (s == 1) {
		if (do_pri8(n, "ld", pre_store16clx, 0)) {
			helper_s(n, helper);
			return 1;
		}
	} else if (s == 2) {
		if (do_pri16(n, "ld", pre_store16)) {
			/* Helper invalidates XA itself */
			helper_s(n, helper);
			return 1;
		}
	}
	/* As we are saving via @tmp we can do these as well */
	switch(r->op) {
	case T_ARGUMENT:
		v += frame_len + argbase;
	case T_LOCAL:
		v += sp;
		/* TODO: SP biasing */
		if (hx_contains(n))
			return 1;
		if (v < 1024) {
			output("tsx");
			adjust_hx(v);
			set_hx_node(r);
			hx_live = 1;
			/*hx_*/helper_s(n, helper);
			return 1;
		}
		break;
	}
	return 0;
}

static int hxpri16_help(struct node *n, const char *op)
{	if (do_hxpri16(n, op, pre_hx)) {
		helper_s(n, op);
		/* These helpers return the working value in XA */
		hx_live = 0;
		return 1;
	}
	return 0;
}


static int pri_help(struct node *n, char *helper)
{
	unsigned s = get_size(n->type);

	if (s == 1 && pri8_help(n, helper))
		return 1;
	else if (s == 2 && pri16_help(n, helper))
		return 1;
	return 0;
}

/* Do a helper op that wants the value in HX */
static int hx_help(struct node *n, char *helper)
{
	unsigned s = get_size(n->type);

	/* Byte wide ops happen on A with H:X as pointer */
	if (s == 1 && pri8_help(n, helper))
		return 1;
	/* Word sized helpers via HX */
	else if (s == 2 && hxpri16_help(n, helper))
		return 1;
	return 0;
}

static int pri_cchelp(register struct node *n, unsigned s, char *helper)
{
	register struct node *r = n->right;
	unsigned v = r->value;

	n->flags |= ISBOOL;

	/* In the case where we know the upper half of the value. Need to sort
	   the signed version out eventually */
	if (r->op == T_CONSTANT && s == 2 && (n->type & UNSIGNED)) {
		if (reg[R_X].state == T_CONSTANT && reg[R_X].value == (v >> 8)) {
			n->type &= UNSIGNED;
			n->type |= CCHAR;
			return pri8_help(n, helper);
		}
	}
	if (n->flags & BYTEABLE) {
		n->type &= UNSIGNED;
		n->type |= CCHAR;
		return pri8_help(n, helper);
	}
	return pri_help(n, helper);
}

static void pre_stash(struct node *n)
{
	if (hx_live == 1) { 
		output("sthx @tmp");
		hx_live = 0;
	} else {
		output("sta @tmp");
		output("stx @tmp+1");
	}
}

/*
 *	inc and dec are complicated but worth some effort as they
 *	are so commonly used for small constants. We could o with
 *	spotting and folding some stuff like *x++ perhaps to get a
 *	bit better codegen.
 */

/* Try to write inline inc and dec for simple forms */
static int leftop_memc(struct node *n, const char *op)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned v;
	unsigned sz = get_size(n->type);
	char *name;
	unsigned count;
	unsigned nr = n->flags & NORETURN;

	if (sz > 2)
		return 0;
	if (r->op != T_CONSTANT || r->value > 2)
		return 0;
	else
		count = r->value;

	/* Being super clever doesn't help if we need the value anyway */
	if (!nr && (n->op == T_PLUSPLUS || n->op == T_MINUSMINUS))
		return 0;

	v = l->value;

	switch(l->op) {
	case T_NAME:
		name = namestr(l->snum);
		while(count--) {
			output("%s _%s+%d", op, name, v + 1);
			if (sz == 2) {
				output("beq X%d", ++xlabel);
				output("%s _%s+%d", op, name, v);
				label("X%d", xlabel);
			}
		}
		if (!nr) {
			output("lda _%s+%d", name, v + 1);
			if (sz == 2)
				output("ldx _%s+%d", name, v);
			hx_live = 0;
		}
		return 1;
	case T_LABEL:
		while(count--) {
			output("%s T%d+%d", op, (unsigned)l->val2, v + 1);
			if (sz == 2) {
				output("beq X%d", ++xlabel);
				output("%s T%d+%d", op, (unsigned)l->val2, v);
				label("X%d", xlabel);
			}
		}
		if (nr == 1) {
			output("lda T%d+%d", (unsigned)l->val2, v + 1);
			if (sz == 2)
				output("ldx T%d+%d", (unsigned)l->val2, v);
			hx_live = 0;
		}
		return 1;
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		v += sp;
		while(count--) {
			output("%s %u,sp", op, v + 1);
			if (sz == 2) {
				output("beq X%d", ++xlabel);
				output("%s %u,sp", op, v);
				label("X%d", xlabel);
			}
		}
		if (nr == 1) {
			output("lda %u,sp", v + 1);
			if (sz == 2)
				output("%u,sp", v);
			hx_live = 0;
		}
		return 1;
	}
	return 0;
}


/* Do a 16bit operation upper half by switching X into A */
static unsigned try_via_x(struct node *n, const char *op, void (*pre)(struct node *))
{
	/* Name and lbref are progably not worth it as have to go via tmp */
	printf(";try_via_x\n");
	if (do_pri8(n, op, pre, 1) == 0)
		return 0;
	output("psha");
	sp++;
	output("txa");
	memcpy(&reg[R_A], &reg[R_X], sizeof(struct regtrack));
	do_pri8hi(n, op, pre_none);
	output("tax");
	output("pula");
	sp--;
	invalidate_a();
	invalidate_x();
	return 1;
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

static void swap_op(struct node *n, unsigned op)
{
	struct node *l = n->left;
	n->left = n->right;
	n->right = l;
	n->op = op;
}

static unsigned is_simple(struct node *n)
{
	unsigned op = n->op;

	/* Multi-word objects are never simple */
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;

	/* We can use these directly with primary operators on A */
	if (op == T_CONSTANT || op == T_LABEL || op == T_NAME || (op == T_LREF && n->value < 255))
		return 10;
	/* Can go via @tmp */
	if (op == T_NREF || op == T_LBREF)
		return 1;
	/* Hard */
	return 0;
}

/* Chance to rewrite the tree from the top rather than none by node
   upwards. We will use this for 8bit ops at some point and for cconly
   propagation */
struct node *gen_rewrite(struct node *n)
{
	byte_label_tree(n, BTF_RELABEL);
	/* TODO label H:X subtrees */
	return n;
}

/* Terminal nodes we can do via HX */
static unsigned term_via_hx(struct node *n)
{
	switch(n->op) {
	/* T_LOCAL always ends up via H:X */
	case T_LOCAL:
	/* These can be done either way */
	case T_NAME:
	case T_CONSTANT:
	case T_LABEL:
	/* These would be nice to do but we'll always generate them via
	   X:A at the moment */
	case T_LREF:
	case T_NREF:
	case T_LBREF:
		return 1;
	}
	return 0;
}

/* Try and do this subtree via HX */
/* TODO: spot constant pushes and do via HX so we can LDHX #nnnn and save
   a byte */
static void work_via_hx(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	if (term_via_hx(n)) {
		n->flags |= USE_HX;
		return;
	}
	switch(n->op) {
	case T_CAST:	/* TODO */
		return;
	/* We can do these two via AIX */
	case T_PLUS:
	case T_MINUS:
		if (r->op == T_CONSTANT && term_via_hx(l)) {
			l->flags |= USE_HX;
			r->flags |= USE_HX;
			n->flags |= USE_HX;
		}
	}
	return;
}
/*
 *	Our chance to do tree rewriting. We don't do much for the HC08
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned op = n->op;
	unsigned nt = n->type;

	/* TODO
		- rewrite some reg ops
	*/

	/* *regptr */
	if (op == T_DEREF && r->op == T_RREF) {
		n->op = T_RDEREF;
		n->right = NULL;
		return n;
	}
	/* *regptr = */
	if (op == T_EQ && l->op == T_RREF) {
		n->op = T_REQ;
		n->left = NULL;
		return n;
	}
	/* Rewrite references into a load operation */
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
		n->val2 = r->val2;
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
	/* Reverse the order of comparisons to make them easier. C sequence points says this
	   is fine. Arguably we should implement T_LT and T_GTEQ and only do this if at that
	   point on code gen XA is holding the value we want for the left TODO */
	if (op == T_GT)
		swap_op(n, T_LT);
	if (op == T_LTEQ)
		swap_op(n, T_GTEQ);

	/* Turn things to HX if it makes sense */
	if (op == T_EQ)
		work_via_hx(l);
	if (op == T_DEREF)
		work_via_hx(r);
	if (op == T_PLUSEQ || op == T_MINUSEQ || op == T_STAREQ ||
		op == T_SLASHEQ || op == T_PERCENTEQ || op == T_SHLEQ ||
		op == T_SHREQ || op == T_ANDEQ || op == T_OREQ || op == T_HATEQ)
		work_via_hx(l);		
	return n;
}

/* Export the C symbol */
void gen_export(const char *name)
{
	output(".export _%s\n", name);
}

void gen_segment(unsigned s)
{
	switch(s) {
	case A_CODE:
		output(".code");
		break;
	case A_DATA:
		output(".data");
		break;
	case A_LITERAL:
		output(".literal");
		break;
	case A_BSS:
		output(".bss");
		break;
	default:
		error("gseg");
	}
}

void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
	unreachable = 0;
	invalidate_regs();
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size;
	if (size == 0)
		return;

	sp = 0;
	adjust_sp(-size);
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp)
		error("sp");

	if (unreachable)
		return;

	adjust_sp(size);
	output("rts");
	unreachable = 1;
}

void gen_label(const char *tail, unsigned n)
{
	unreachable = 0;
	label("L%d%s", n, tail);
	invalidate_regs();
}

unsigned gen_exit(const char *tail, unsigned n)
{
	if (frame_len == 0) {
		output("rts");
		unreachable = 1;
		return 1;
	} else {
		output("jmp L%d%s", n, tail);
		unreachable = 1;
		return 0;
	}
}

void gen_jump(const char *tail, unsigned n)
{
	/* Want to use BRA if we have the option */
	output("jmp L%d%s", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	output("jeq L%d%s", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	output("jne L%d%s", n, tail);
}

void gen_switch(unsigned n, unsigned type)
{
	/* TODO: fix to work with output */
	gen_helpcall(NULL);
	printf("switch");
	helper_type(type, 0);
	printf("\n");
	output(".word Sw%d\n", n);
}

void gen_switchdata(unsigned n, unsigned size)
{
	label("Sw%d", n);
	output("\t.word %d", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
	label("Sw%d_%d", tag, entry);
	invalidate_regs();
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

/* Helpers all return in XA */
void gen_helpcall(struct node *n)
{
	invalidate_regs();
	printf(";helpcall clearing hx_live (was %u)\n", hx_live);
	printf("\tjsr __");
	hx_live = 0;
}

void gen_helptail(struct node *n)
{
}

void gen_helpclean(struct node *n)
{
	/* Bool return is 0 or 1 therefore X is 0 */
	if (n->flags & ISBOOL) {
		reg[R_X].state = T_CONSTANT;
		reg[R_X].value = 0;
	}
}

void gen_data_label(const char *name, unsigned align)
{
	label("_%s", name);
}

void gen_space(unsigned value)
{
	output(".ds %d", value);
}

void gen_text_data(struct node *n)
{
	output(".word T%d", n->val2);
}

void gen_literal(unsigned n)
{
	if (n)
		label("T%d", n);
}

void gen_name(struct node *n)
{
	output(".word _%s+%d", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	if (PTR(type)) {
		output(".word %u", (unsigned) value);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		output(".byte %u", (unsigned) value & 0xFF);
		break;
	case CSHORT:
	case USHORT:
		output(".word %d", (unsigned) value & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are big endian */
		output(".word %d", (unsigned) ((value >> 16) & 0xFFFF));
		output(".word %d", (unsigned) (value & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	output(".code");
}

void gen_end(void)
{
}

void gen_tree(struct node *n)
{
	hx_live = 0;
	codegen_lr(n);
	/* Make sure result is in the right place */
	if (!(n->flags & NORETURN))
		use_xa(0);
	label(";");
}

unsigned gen_push(struct node *n)
{
	unsigned s = get_stack_size(n->type);
	sp += s;

	if (hx_live) {
		switch(s) {
		case 1:
			output("pshx");
			return 1;
		case 2:
			output("pshx");
			output("pshh");
			return 1;
		default:
			error("psh4hx");
		}
	} else switch(s) {
		case 1:
			output("psha");
			return 1;
		case 2:
			output("psha");
			output("pshx");
			return 1;
		case 4:
			output("psha");
			output("pshx");
			output("lda @hireg+1");
			output("psha");
			output("lda @hireg+1");
			output("psha");
			invalidate_a();
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
/*	unsigned nr = n->flags & NORETURN; */
	unsigned v;

	if (r)
		v = r->value;

	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		adjust_sp(v);
		sp -= v;
		return 1;
	case T_EQ:	/* address in XA, can we build right ? */
		/* Need to rework for HX forms */
		/* This might be BYTEROOT but it doesn't matter if so as the
		   type handling was done for us */
		if (s > 2)
			return 0;
		if (s == 1 && do_pri8(n, "lda", pre_stash, 0)) {
			invalidate_a();
			output("sta (@tmp)");
			return 1;
		} else if (s == 2 && do_pri16(n, "ld", pre_stash)) {
			invalidate_x();
			invalidate_a();
			output("sta (@tmp),1");
			output("stx (@tmp),0");
			return 1;
		}
		/* Complex on both sides. Do these the hard way. Not as bad
		   as it seems as these are not common */
		return 0;
	case T_AND:
		/* There are some cases we can deal with */
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (s == 2) {
				if ((v & 0xFF00) == 0x0000)
					load_x(0);
				else if ((v & 0xFF00) != 0xFF00)
					return try_via_x(n, "and", pre_none);
			}
			if ((v & 0xFF) == 0x00)
				load_a(0);
			else if ((v & 0xFF) != 0xFF) {
				output("and #%d", v & 0xFF);
				const_a_set(reg[R_A].value & v);
			}
			return 1;
		}
		if (s == 1 && pri8(n, "and"))
			return 1;
		if (s == 2 && try_via_x(n, "and", pre_none))
			return 1;
		return pri_help(n, "andtmp");
	case T_OR:
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (s == 2) {
				if ((v & 0xFF00) == 0xFF00)
					load_x(0xFF);
				else if ((v & 0xFF00) != 0x0000)
					return try_via_x(n, "ora", pre_none);
			}
			if ((v & 0xFF) == 0xFF)
				load_a(0xFF);
			else if ((v & 0xFF) != 0x00) {
				output("ora #%d", v & 0xFF);
				const_a_set(reg[R_A].value | v);
			}
			return 1;
		}
		if (s == 1 && pri8(n, "ora"))
			return 1;
		if (s == 2 && try_via_x(n, "ora", pre_none))
			return 1;
		return pri_help(n, "oratmp");
	case T_HAT:
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (s == 2) {
				if ((v & 0xFF00) != 0x0000)
					return try_via_x(n, "eor", pre_none);
			}
			if ((v & 0xFF) != 0x00) {
				output("eor #%d", ((unsigned)r->value) & 0xFF);
				const_a_set(reg[R_A].value ^ r->value);
			}
			return 1;
		}
		if (s == 1 && pri8(n, "eor"))
			return 1;
		if (s == 2 && try_via_x(n, "eor", pre_none))
			return 1;
		return pri_help(n, "xortmp");
	case T_PLUS:
		if (s > 2)
			return 0;
		/* If we've been labelled to use HX then do so. Optimises
		   a lot of mess with pointers */
		if (s == 2 && r->op == T_CONSTANT && hx_live) {
			printf(";hxplus\n");
			adjust_hx(v);
			return 1;
		}
		if (s == 1 && r->op == T_CONSTANT && v == 1) {
			output("inca");
			const_a_set(reg[R_A].value + 1);
			return 1;
		}
		if (s == 1 && r->op == T_CONSTANT && v == 0xFFFF) {
			output("deca");
			const_a_set(reg[R_A].value - 1);
			return 1;
		}
		if (s == 1 && do_pri8(n, "add", pre_none, 0)) {
			if (r->op == T_CONSTANT)
				const_a_set(reg[R_A].value + r->value);
			else
				invalidate_a();
			return 1;
		}
		if (s == 2 && r->op == T_CONSTANT) {
			if (r->value <= 0xFF) {
				output("add #%d",v & 0xFF);
				output("bcs X%d", ++xlabel);
				output("incx");
				label("X%d", xlabel);
				const_a_set(reg[R_A].value + (v & 0xFF));
				/* TODO: set up X properly if known */
				invalidate_x();
				return 1;
			}
			if (r->value == 256) {
				output("incx");
				const_x_set(reg[R_X].value + 1);
				return 1;
			}
			if (r->value == 512) {
				output("incx");
				output("incx");
				const_x_set(reg[R_X].value + 2);
				return 1;
			}
		}
		if (s == 2 && try_via_x(n, "add", pre_none))
			return 1;
		return pri_help(n, "addtmp");
	case T_MINUS:
		if (s > 2)
			return 0;
		/* If we've been labelled to use HX then do so. Optimises
		   a lot of mess with pointers */
		if (s == 2 && r->op == T_CONSTANT && hx_live) {
			printf(";hxminus\n");
			adjust_hx(-v);
			return 1;
		}
		if (s == 1 && r->op == T_CONSTANT && v == 1) {
			output("deca");
			const_a_set(reg[R_A].value - 1);
			return 1;
		}
		if (s == 1 && r->op == T_CONSTANT && v == 0xFFFF) {
			output("inca");
			const_a_set(reg[R_A].value + 1);
			return 1;
		}
		if (s == 1 && do_pri8(n, "sub", pre_none, 0)) {
			if (r->op == T_CONSTANT)
				const_a_set(reg[R_A].value - r->value);
			else
				invalidate_a();
			return 1;
		}
		if (s == 2 && r->op == T_CONSTANT) {
			if (r->value <= 0xFF) {
				output("sub #%d", v & 0xFF);
				output("bcc X%d", ++xlabel);
				output("decx");
				label("X%d", xlabel);
				const_a_set(reg[R_A].value - (v & 0xFF));
				/* TODO: we should probably set this up */
				invalidate_x();
				return 1;
			}
			if (r->value == 256) {
				output("decx");
				const_x_set(reg[R_X].value - 1);
				return 1;
			}
			if (r->value == 512) {
				output("decx");
				output("decx");
				const_x_set(reg[R_X].value - 2);
				return 1;
			}
		}
		if (s == 2 && try_via_x(n, "sub", pre_none))
			return 1;
		return pri_help(n, "subtmp");
	case T_STAR:
		if (s > 2)
			return 0;
		/* ? do we need to catch x 1 and x 0 - should always have been cleaned up but
		   maybe not if byteop */
		/* TODO word forms possible on HC08 */
		if (r->op == T_CONSTANT) {
			if (v == 256) {
				if (s == 2) {
					/* Should be helpers for tax/txa */
					output("tax");
					memcpy(&reg[R_X], &reg[R_A], sizeof(struct regtrack));
				}
				load_a(0);
				return 1;
			}
			
			/* We can do a few very simple cases directly */
			if (s == 1) {
				if (v == 0) {
					load_a(0);
					return 1;
				}
				/* TODO: tidy up into a proper mul helper */
				if (v == 1)
					return 1;
				if (v == 2) {
					output("lsla");
					const_a_set(reg[R_A].value << 1);
					return 1;
				}
				if (v == 4) {
					output("lsla");
					output("lsla");
					const_a_set(reg[R_A].value << 2);
					return 1;
				}
				if (v == 8) {
					output("lsla");
					output("lsla");
					output("lsla");
					const_a_set(reg[R_A].value << 3);
					return 1;
				}
			}
			if (v < 16) {
				if (s == 1)
					load_x(0);
				output("jsr __mulc%u", v);
				invalidate_x();
				invalidate_a();
				return 1;
			}
		}
		return pri_help(n, "multmp");
	case T_SLASH:
		if (r->op == T_CONSTANT && v == 256 && (n->type & UNSIGNED)) {
			output("txa");
			memcpy(&reg[R_A], &reg[R_X], sizeof(struct regtrack));
			load_x(0);
			return 1;
		}
		/* TODO - power of 2 const into >> */
		return pri_help(n, "divtmp");
	case T_PERCENT:
		if (r->op == T_CONSTANT && v == 256 && (n->type & UNSIGNED)) {
			load_x(0);
			return 1;
		}
		return pri_help(n, "remtmp");
	/*
	 *	There are various < 0, 0, !0, > 0 optimizations to do here
	 *	TODO - optimizations especially for bool/byteable cases
	 *	Need CCONLY to make this work really
	 */
	case T_EQEQ:
		if (r->op == T_CONSTANT && v == 0) {
			/* TODO: not via helper */
			helper(n, "not");
			n->flags |= ISBOOL;
			return 1;
		}
		return pri_cchelp(n, s, "eqeqtmp");
	case T_GTEQ:
		return pri_cchelp(n, s, "gteqtmp");
	case T_GT:
		return pri_cchelp(n, s, "gttmp");
	case T_LTEQ:
		return pri_cchelp(n, s, "lteqtmp");
	case T_LT:
		return pri_cchelp(n, s, "lttmp");
	case T_BANGEQ:
		if (r->op == T_CONSTANT && v == 0) {
			/* TODO: not via helper */
			helper(n, "bool");
			n->flags |= ISBOOL;
			return 1;
		}
		return pri_cchelp(n, s, "netmp");
	/* TODO: qq optimisations for >= fieldwidth ? */
	case T_LTLT:
		if (s == 2 && r->op == T_CONSTANT && v == 8) {
			output("tax");
			load_a(0);
			return 1;
		}
		/* Shifts: we can get 1 byte left shifts from the byteop convertor */
		if (s == 1 && r->op == T_CONSTANT) {
			if (v >= 8)
				load_a(0);
			else {
				repeated_op(v, "lsla");
				const_a_set(reg[R_A].value >> v);
			}
			return 1;
		}
		return pri_help(n, "lstmp");
	case T_GTGT:
		if (s == 2 && r->op == T_CONSTANT && v == 8) {
			if (n->type & UNSIGNED) {
				output("txa");
				load_x(0);
				return 1;
			}
		}
		return pri_help(n, "rstmp");
	/* TODO: special case by 1,2,4, maybe inline byte cases ? */
	/* We want to spot trees where the object on the left is directly
	   addressible and fold them so we can generate inc _reg, bcc, inc _reg+1 etc */
	/* TODO: look at push/pop for nr in leftop_tmp as option when need result - esp on C02 */
	case T_PLUSPLUS:
		if (s == 2 && r->op == T_CONSTANT) {
			if (v == 1) {
				gen_internal("plusplus1");
				return 1;
			}
			if (v == 2) {
				gen_internal("plusplus2");
				return 2;
			}
			if (v == 4) {
				gen_internal("plusplus4");
				return 4;
			}
		}
		return hx_help(n, "plusplustmp");
	case T_MINUSMINUS:
		if (s == 2 && r->op == T_CONSTANT) {
			if (v == 1) {
				gen_internal("minusminus1");
				return 1;
			}
			if (v == 2) {
				gen_internal("minusminus2");
				return 1;
			}
			if (v == 4) {
				gen_internal("minusminus4");
				return 1;
			}
		}
		return hx_help(n, "minusminustmp");
	case T_PLUSEQ:
		if (s == 2 && r->op == T_CONSTANT) {
			if (v == 1) {
				gen_internal("pluseq1");
				return 1;
			}
			if (v == 2) {
				gen_internal("pluseq2");
				return 1;
			}
			if (v == 4) {
				gen_internal("pluseq4");
				return 1;
			}
		}
		return hx_help(n, "pluseqtmp");
	case T_MINUSEQ:
		if (s == 2 && r->op == T_CONSTANT) {
			if (v == 1) {
				gen_internal("minuseq1");
				return 1;
			}
			if (v == 2) {
				gen_internal("minuseq2");
				return 1;
			}
			if (v == 4) {
				gen_internal("minuseq4");
				return 1;
			}
		}
		return hx_help(n, "minuseqtmp");
	case T_ANDEQ:
		return hx_help(n, "andeqtmp");
	case T_OREQ:
		return hx_help(n, "oreqtmp");
	case T_HATEQ:
		return hx_help(n, "oreqtmp");
#if 0
	/* still to do - more complex see 65C816 */
	case T_ARGCOMMA:
		/* We generate these directly when we can to optimize the
		   call return overhead a bit but it has to be done by
		   the peepholer */
		if (s == 1)
			output("jsr __pushc");
		else if (s == 2)
			output("jsr __push");
		else
			output("jsr __pushl");
		set_reg(R_H, 0);
		sp += s;
		return 1;
#endif
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

	/* Unreachable code we can shortcut into nothing whee.bye.. */
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
	case T_PLUSPLUS:
		/* The left nay be a complex expression but also may be soemthing
		   we can directly reference. The right is the amount */
		if (leftop_memc(n, "inc"))
			return 1;
		break;
	case T_MINUSMINUS:
		if (leftop_memc(n, "dec"))
			return 1;
		break;
	case T_PLUSEQ:
		if (leftop_memc(n, "inc"))
			return 1;
		break;
	case T_MINUSEQ:
		if (leftop_memc(n, "dec"))
			return 1;
		break;
	case T_LSTORE:
		if (r->type == T_CONSTANT) {
			printf(";lstore c %u %u\n", (unsigned)n->value, sp);
			v = r->value;
			load_a(v & 0xFF);
			output("sta %u,sp", (unsigned)n->value + sp);
			if (nr) {
				load_a(v >> 8);
				output("sta %u,sp", (unsigned)n->value + sp + 1);
			} else {
				load_x(v >> 8);
				output("stx %u,sp", (unsigned)n->value + sp + 1);
			}
			return 1;
		}
	}
	return 0;
}

static void char_to_int(void)
{
	load_x(0);
	output("ora #0");
	output("bmi X%d", ++xlabel);
	output("dex");
	label("X%d", xlabel);
	invalidate_x();
}

/* TODO cast from pointer - H:X to XA */
static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned ls;
	unsigned rs;

	use_xa(2);	/* TODO: tidy up and keep in HX when we can */

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
	if (!(rt & UNSIGNED)) {
		/* Signed */
		if (ls == 2) {
			char_to_int();
			return 1;
		}
		return 0;
	}
	if (ls == 4) {
		rs = get_size(rt);
		if (rs == 1) {
			load_x(0);
		}
		output("clr @hireg");
		output("clr @hireg+1");
		return 1;
	}
	if (ls == 2) {
		load_x(0);
		return 1;
	}
	return 0;
}

unsigned do_gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned v;
	unsigned nr = n->flags & NORETURN;
	unsigned se = n->flags & SIDEEFFECT;
	unsigned is_byte = (n->flags & (BYTETAIL | BYTEOP)) == (BYTETAIL | BYTEOP);
	unsigned hx = n->flags & USE_HX;

	v = n->value;

	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_FUNCCALL && n->op != T_CALLNAME)
		sp -= get_stack_size(n->left->type);
	switch(n->op) {
	/* FIXME: need to do 4 byte forms */
	case T_LREF:
		if (nr && !se)
			return 1;
		if (is_byte && !se) {
			if (a_contains(n))
				return 1;
			/* We can avoid the half load but big endian means
			   we need the high byte */
			output("lda %u,sp", v + sp + 1);
			invalidate_a();
			hx_live = 0;
			return 1;
		}
		if (size == 1) {
			if (a_contains(n))
				return 1;
			output("lda %u,sp", v + sp);
			invalidate_a();
			hx_live = 0;
			return 1;
		}
		if (size == 2) {
			printf(";T_LREF %u %u\n", v, sp);
			if (a_contains(n))
				return 1;
			output("lda %u,sp", v + sp);
			invalidate_a();
			output("tax");
			invalidate_x();
			output("lda %u,sp", v + sp + 1);
			invalidate_a();
			hx_live = 0;
			return 1;
		}
		if (optsize) {
			if (size == 4 && v < 253) {
				if (n == 0)
					output("jsr __gloa0l");
				else {
					load_a(v + 3);
					output("jsr __gloal");
				}
				invalidate_a();
				invalidate_x();
				/* State of H ? TODO */
				return 1;
			}
		}
		/* Fall through */
	case T_NREF:
	case T_LBREF:
		if (nr && !se)
			return 1;
		if (is_byte && !se) {
			if (a_contains(n))
				return 1;
			if (do_pri8(n, "lda", pre_none, 1)) {
				set_a_node(n);
				return 1;
			} 
		}
		if (size == 1) {
			if (a_contains(n))
				return 1;
			if (pri8(n, "lda")) {
				set_a_node(n);
				return 1;
			}
		} else if (size == 2) {
			if (xa_contains(n))
				return 1;
			if (pri16(n, "ld")) {
				set_xa_node(n);
				return 1;
			}
		}
		/* FIXME: need to do 4 byte forms ?? */
		return 0;
	case T_NSTORE:
	case T_LBSTORE:
	case T_LSTORE:
		/* If value is in HX then for byte just flip X into A */
		if (size == 1 && hx_live == 1) {
			if (nr && pri8(n, "stx"))
				return 1;
			if (do_pri8(n, "sta", pre_txa, 0)) {
				set_a_node(n);
				return 1;
			}
		}
		/* TODO for byte size in HX just txa sta */
		if (size == 1 && pri8(n, "sta")) {
			set_a_node(n);
			return 1;
		} else if (size == 2) {
			if (nr && pri16(n, "st")) {
				set_xa_node(n);
				return 1;
			}
			/* Stack and restore A if we need XA intact (rare) */
			if (do_pri16(n, "st", pre_psha)) {
				output("pla");
				set_xa_node(n);
				return 1;
			}
		}
		/* FIXME: need to do 4byte forms **/
		return 0;
	case T_CALLNAME:
		invalidate_regs();
		output("jsr _%s+%d", namestr(n->snum), n->value);
		return 1;
	case T_EQ:
		/* store XA in top of stack addr  .. ugly. We want to end
		   up with stuff in @tmp and H:X form */
		if (size > 2)
			return 0;
		if (size == 1) {
			output("pulh");
			output("pulx");
			output("sta ,x");
			invalidate_hx();
			invalidate_mem();
			return 1;
		}
		output("stx @tmp");
		output("pulh");
		output("pulx");
		invalidate_hx();
		output("sta 1,x");
		output("lda @tmp");
		output("sta ,x");
		if (!nr) {
			output("tax");
			output("lda 1,x");
		}
		invalidate_mem();
		return 1;
	case T_FUNCCALL:
		/* For now just helper it via HX */
		return 2;
	case T_DEREF:
		use_hx(40);
		if (nr && !se)
			return 1;
		/* If BYTEOP is set then non volatiles can be done
		   byte sized */
		if (!se && is_byte)
			size = 1;
		/* We could optimize the tracing a bit here. A deref
		   of memory where we know XA is a name, local etc is
		   one where we can update the contents info TODO */
		if (size > 2)
			return 0;
		if (size == 1) {
			output("lda ,x");
			invalidate_a();
		} else {
			output("lda ,x");
			output("ldx 1,x");
			invalidate_a();
			invalidate_x();
		}
		hx_live = 0;
		return 1;
	case T_CONSTANT:
		if (hx && size == 2) {
			load_hx(n->value);
			hx_live = 1;
			return 1;
		}
		/* Only load the bits needed if we are constant */
		if (is_byte)
			size = 1;
		if (size > 2) {
			load_a(n->value >> 24);
			output("sta @hireg+1");
			load_a(n->value >> 16);
			output("sta @hireg");
		}
		/* We have to special case this to get the value setting right */
		if (size == 2)
			load_x(v >> 8);
		load_a(v & 0xFF);
		hx_live = 0;
		return 1;
	case T_NAME:
		if (hx) {
			output("ldhx #_%s+%u\n", namestr(n->snum), v);
			hx_live = 1;
			return 1;
		}
	case T_LABEL:
		if (hx) {
			output("ldhx #T%u+%u\n", n->val2, v);
			hx_live = 1;
			return 1;
		}
		if (is_byte)
			size = 1;
		if (size == 1 && pri8(n, "lda")) {
			invalidate_a();
			return 1;
		}
		if (size == 2 && pri16(n, "ld")) {
			invalidate_x();
			invalidate_a();
			return 1;
		}
		return 0;
	/* These two end up in HX regardless */
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		v += sp;
		if (hx_contains(n))
			return 1;
		if (v < 1024) {
			output("tsx");
			adjust_hx(v);
		} else {
			load_x(v >> 8);
			load_a(v);
			output("jsr __xasp");
		}
		/* Result in H:X - need to start doing proper HX tracking */
		set_hx_node(n);
		hx_live = 1;
		return 1;
	/* Local and argument are more complex so helper them */
	case T_CAST:
		return gen_cast(n);
	/* TODO: CCONLY */
	case T_BANG:
		use_xa(42);
		if (n->right->flags & (ISBOOL|BYTEABLE)) {
			output("eora #1");
			invalidate_a();
		} else
			helper(n, "not");
		n->flags |= ISBOOL;
		return 1;
	case T_BOOL:
		if (n->right->flags & ISBOOL)
			return 1;
		use_xa(43);
		if (n->flags & BYTEABLE) {
			output("tax");	/* Set the Z flag */
			output("beq X%u", ++xlabel);
			output("lda #1");
			label("X%u:", xlabel);
		} else {
			helper(n, "bool");
		}
		n->flags |= ISBOOL;
		return 1;
	/* Helpers that want the value in HX */
	case T_PLUSEQ:
	case T_MINUSEQ:
	case T_MINUSMINUS:
	case T_PLUSPLUS:
	case T_ANDEQ:
	case T_OREQ:
	case T_HATEQ:
	case T_STAREQ:
	case T_SLASHEQ:
	case T_PERCENTEQ:
	case T_SHLEQ:
	case T_SHREQ:
		return 2;
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned r;
	r = do_gen_node(n);
	if (r == 1)
		return 1;
	/* Force correct helper */
	if (r == 0) {
		printf(";force helper %x\n", n->op);
		use_xa(3);
	} else if (r == 2) {
		printf(";force helper hx %x\n", n->op);
		use_hx(3);
	}
	return 0;
}

