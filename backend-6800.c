/*
 *	6800: original in the line of processors ending with 68HC11
 *	- Only has 8bit operations
 *	- Cannot push x (hack to pop it)
 *	- Cannot  move between accumulators and X except via memory
 *	- Can copy X to and from S but can only move to/from accumulator
 *	  via memory
 *	- Some useful flag behaviour on late devices is not present.
 *
 *	6803:
 *	- Adds 16bit operations
 *	- Adds ABX
 *	- Can push or pull X
 *
 *	6303: like 6803
 *	- Adds some interesting bit ops
 *	- Adds XGDX
 *
 *	68HC11: like 6803
 *	- Adds a bunch of bitops, branch on bit and other stuff
 *	- Adds a Y register
 *	- Has XGDX and XGDY
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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
#define T_DEREFPLUS	(T_USER+7)
#define T_EQPLUS	(T_USER+8)
#define T_LDEREF	(T_USER+9)	/* *local + offset */
#define T_LEQ		(T_USER+10)	/* *local + offset = n*/
/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned argbase;	/* Argument offset in current function */
static unsigned unreachable;	/* Code following an unconditional jump */
static unsigned cpu_has_d;	/* 16bit ops and 'D' are present */
static unsigned cpu_has_xgdx;	/* XGDX is present */
static unsigned cpu_has_abx;	/* ABX is present */
static unsigned cpu_has_pshx;	/* Has PSHX PULX */
static unsigned cpu_has_y;	/* Has Y register */
static unsigned cpu_is_09;	/* Bulding for 6x09 so a bit different */

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

/*
 *	Fix up weirdness in the asm formats.
 */
static const char *remap_op(const char *op)
{
	if (cpu_is_09)
		return op;
	/* Some 68xx ops are a bit irregular
	   - ldd v ldab etc */
	if (strcmp(op, "ld") == 0)
		return "lda";
	if (strcmp(op, "or") == 0)
		return "ora";
	if (strcmp(op, "st") == 0)
		return "sta";
	return op;
}

/*
 *	Start with some simple X versus S tracking
 *	to clean up the local accesses
 */
static uint16_t x_fpoff;
static unsigned x_fprel;
static uint8_t a_val;
static uint8_t b_val;
static unsigned a_valid;
static unsigned b_valid;
static struct node d_node;
static unsigned d_valid;

void invalidate_all(void)
{
	x_fprel = 0;
	a_valid = 0;
	b_valid = 0;
	d_valid = 0;
}

void invalidate_x(void)
{
	x_fprel = 0;
}

void invalidate_a(void)
{
	a_valid = 0;
}

void invalidate_b(void)
{
	b_valid = 0;
}

void invalidate_d(void)
{
	d_valid = 0;
}

void invalidate_work(void)
{
	a_valid = 0;
	b_valid = 0;
	d_valid = 0;
}

void invalidate_mem(void)
{
	/* If memory changes it might be an alias to the value cached in AB */
	switch(d_node.op) {
	case T_LREF:
	case T_LBREF:
	case T_NREF:
		d_valid = 0;
	}
}

void set_d_node(struct node *n)
{
	memcpy(&d_node, n, sizeof(struct node));
	switch(d_node.op) {
	case T_LSTORE:
		d_node.op = T_LREF;
		break;
	case T_LBSTORE:
		d_node.op = T_LBREF;
		break;
	case T_NSTORE:
		d_node.op = T_NREF;
		break;
	case T_NREF:
	case T_LBREF:
	case T_LREF:
	case T_NAME:
	case T_LABEL:
	case T_LOCAL:
	case T_ARGUMENT:
		break;
	default:
		d_valid = 0;
	}
	d_valid = 1;
}

/* Do we need to check fields by type or will the default filling
   be sufficient ? */
unsigned d_holds_node(struct node *n)
{
	if (d_valid == 0)
		return 0;
	if (d_node.op != n->op)
		return 0;
	if (d_node.val2 != n->val2)
		return 0;
	if (d_node.snum != n->snum)
		return 0;
	if (d_node.value != n->value)
		return 0;
	return 1;
}

void modify_a(uint8_t val)
{
	a_val = val;
	d_valid = 0;
}

void modify_b(uint8_t val)
{
	b_val = val;
	d_valid = 0;
}

/* 16bit constant load */
void load_d_const(uint16_t n)
{
	unsigned hi,lo;

	if (cpu_has_d) {
		if (n == 0) {
			if (!a_valid || a_val)
				printf("\tclra\n");
			if (!b_valid || b_val)
				printf("\tclrb\n");
		} else {
			/* TODO: There are some fringe cases where we can
			   do better - eg if A is already valid and right */
			printf("\tldd #%u\n", n);
		}
	} else {
		/* TODO: track AB here and see if we can use existing values */
		lo = n & 0xFF;
		hi = n >> 8;
		if (a_valid == 0 || hi != a_val) {
			if (hi == 0)
				printf("\tclra\n");
			else if (hi == b_val) {
				printf("\ttba\n");
				printf("\t%sa #%d\n", remap_op("ld"), hi);
			}
		}
		if (b_valid == 0 || lo != b_val) {
			if (lo == 0)
				printf("\tclrb\n");
			else if (lo == hi)
				printf("\ttab\n");
			return;
		} else
			printf("\t%sb #%d\n", remap_op("ld"), lo);
	}
	a_valid = 1;	/* We know the byte values */
	b_valid = 1;
	d_valid = 0;	/* No longer an object reference */
	a_val = hi;
	b_val = lo;

}

void load_a_const(uint8_t n)
{
	if (a_valid && n == a_val)
		return;
	if (n == 0)
		printf("\tclra\n");
	else if (b_valid && n == b_val) {
		if (cpu_is_09)
			printf("\ttfr b,a\n");
		else
			printf("\ttba\n");
	} else
		printf("\t%sa #%u\n", remap_op("ld"), n & 0xFF);
	a_valid = 1;
	a_val = n;
	d_valid = 0;
}

void load_b_const(uint8_t n)
{
	if (b_valid && n == b_val)
		return;
	if (n == 0)
		printf("\tclrb\n");
	else if (a_valid && n == a_val) {
		if (cpu_is_09)
			printf("\ttfr a,b\n");
		else
			printf("\ttab\n");
	} else
		printf("\t%sb #%u\n", remap_op("ld"), n & 0xFF);
	b_valid = 1;
	b_val = n;
	d_valid = 0;
}

void add_d_const(uint16_t n)
{
	if (n == 0)
		return;
	if (cpu_has_d)
		printf("\taddd #%u\n", n);
	else {
		/* TODO: can do better in terms of obj/offset but not clear it is
		   that useful */
		d_valid = 0;
		if (n & 0xFF) {
			printf("\taddb #%u\n", n & 0xFF);
			printf("\tadca #%u\n", n >> 8);
		} else
			printf("\tadda #%u\n", n >> 8);
	}
	if (b_val + (n & 0xFF) < b_val)
		a_val += (n >> 8) + 1;
	else
		a_val += (n >> 8);
	b_val += (n & 0xFF);
}

void add_b_const(uint8_t n)
{
	if (n == 0)
		return;
	printf("\taddb #%u\n", n & 0xFF);
	b_val += n;
	d_valid = 0;

}

void load_a_b(void)
{
	printf("\ttba\n");
	a_val = b_val;
	a_valid = b_valid;
	d_valid = 0;
}

void load_b_a(void)
{
	printf("\ttab\n");
	b_val = a_val;
	b_valid = a_valid;
	d_valid = 0;
}

void move_s_d(void)
{
	if (cpu_is_09)
		printf("\ttfr s,d\n");
	else {
		printf("\tsts @tmp\n");
		if (cpu_has_d)
			printf("\tldd @tmp\n");
		else {
			printf("\tldaa @tmp\n");
			printf("\tldab @tmp+1\n");
		}
	}
	invalidate_work();
}

void move_d_s(void)
{
	if (cpu_is_09)
		printf("\ttfr d,s\n");
	else {
		if (cpu_has_d)
			printf("\tstd @tmp\n");
		else {
			printf("\tstaa @tmp\n");
			printf("\tstab @tmp+1\n");
		}
		printf("\tlds @tmp\n");
	}
}

void swap_d_y(void)
{
	if (cpu_is_09)
		printf("\texg d,y\n");
	else
		printf("\txgdy\n");
}

/* Get D into X (may trash D) */
void make_x_d(void)
{
	if (cpu_is_09)
		printf("\ttfr d,x\n");
	else if (cpu_has_xgdx) {
		/* Should really track on the exchange later */
		invalidate_d();
		printf("\txgdx\n");
	} else {
		if (cpu_has_d)
			printf("\tstd @tmp\n");
		else
			printf("\tstaa @tmp\n\tstab @tmp+1\n");
		printf("\tldx @tmp\n");
	}
	/* TODO: d -> x see if we know d type */
	invalidate_x();
}

void pop_x(void)
{
	/* Must remember this trashes X, or could make it smart
	   when we track and use offsets of current X then ins ins */
	if (cpu_is_09)
		printf("\tldx ,--s\n");
	else {
		/* Easier said than done on a 6800 */
		if (cpu_has_pshx)
			printf("\tpulx\n");
		else
			printf("\ttsx\n\tldx ,x\n\tins\n\tins\n");
	}
	invalidate_x();
}

/*
 *	There are multiple strategies depnding on chip features
 *	available.
 */
void adjust_s(int n, unsigned save_d)
{
	unsigned abxcost = 3 + 2 * save_d +  n / 255;
	unsigned hardcost;
	unsigned cost;

	/* 6809 is nice and simple */
	if (cpu_is_09) {
		if (n)
			printf("\tleas %d,s\n", n);
		return;
	}

	if (cpu_has_d)
		hardcost = 15 + 4 * save_d;
	else
		hardcost = 18 + 2 * save_d;

	cost = hardcost;

	/* Processors with XGDX always have PULX so we use whichever is
	   the shorter of the two approaches */
	if (cpu_has_xgdx) {
		if (n > 14) {
			printf("\ttsx\n\txgdx\n\taddd #%u\n\txgdx\n\ttxs\n", WORD(n));
			invalidate_x();
			return;
		}
		/* Otherwise we know pulx is cheapest */
		repeated_op(n / 2, "pulx");
		if (n & 1)
			printf("\tins\n");
		return;
	}

	if (cpu_has_abx && abxcost < hardcost)
		cost = abxcost;
	/* PULX might be fastest */
	if (n > 0 && cpu_has_pshx && (n / 2) + (n & 1) <= cost) {
		repeated_op(n / 2, "pulx");
		if (n & 1)
			printf("\tins\n");
		return;
	}
	/* If not check if ins is */
	if (n >= 0 && n <= cost) { 
		repeated_op(n, "ins");
		return;
	}
	/* ABX is the cheapest option if we have it */
	if (n > 0 && cpu_has_abx && cost == abxcost) {
		/* TODO track b properly when save save_d */
		if (save_d)
			printf("\tpshb\n");
		if(n > 255) {
			load_b_const(255);
			while(n >= 255) {
				printf("\tabx\n");
				n -= 255;
			}
		}
		if (n) {
			load_b_const(n);
			printf("\tabx\n");
		}
		if (save_d)
			printf("\tpulb\n");
		return;
			
	}
	/* Forms where ins/des are always best */
	if (n >=0 && n <= 4) {
		repeated_op(n, "ins");
		return;
	}
	if (cpu_has_pshx && n < 0 && -n/2 + (n & 1) <= hardcost) {
		repeated_op(-n/2, "pshx");
		if (n & 1)
			printf("\tdes\n");
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
		printf("\t.word %u\n", WORD(n));
		return;
	}
	if (n >=0 && n <= hardcost) {
		repeated_op(n, "ins");
		return;
	}
	if (n < 0 && -n <= hardcost) {
		repeated_op(-n, "des");
		return;
	}
	/* TODO: if we save_d we need to keep abd valid */
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
	printf("\t%sb %u,x\n", remap_op(op), off + 1);
	printf("\t%sa %u,x\n", remap_op(op2), off);
}

/* Operations where D can be used on later processors */
void op16d_on_ptr(const char *op, const char *op2, unsigned off)
{
	/* Big endian */
	if (cpu_has_d) {
		printf("\t%sd %u,x\n", op, off);
	} else {
		/* ldd not ldab, std not stad ! */
		printf("\t%sb %u,x\n", remap_op(op), off + 1);
		printf("\t%sa %u,x\n", remap_op(op2), off);
	}
}

void op8_on_s(const char *op, unsigned off)
{
	printf("\t%sb %u,s\n", remap_op(op), off);
}

void op8_on_spi(const char *op)
{
	printf("\t%sb ,s+\n", op);
}

/* Do the low byte first in case it's add adc etc */
void op16_on_s(const char *op, const char *op2, unsigned off)
{
	/* Big endian */
	printf("\t%sb %u,s\n", remap_op(op), off + 1);
	printf("\t%sa %u,s\n", remap_op(op2), off);
}

/* Always with D on a 6809 only op */
void op16d_on_spi(const char *op)
{
	printf("\t%sd ,s++\n", op);
}

void op16_on_spi(const char *op)
{
	printf("\t%sa ,s+\n", op);
	printf("\t%sb ,s+\n", op);
}

void op16d_on_s(const char *op, const char *op2, unsigned off)
{
	/* Big endian */
	if (cpu_has_d) {
		printf("\t%sd %u,s\n", op, off);
	} else {
		/* ldd not ldab, std not stad ! */
		printf("\t%sb %u,s\n", remap_op(op), off + 1);
		printf("\t%sa %u,s\n", remap_op(op2), off);
	}
}

void op32_on_ptr(const char *op, const char *op2, unsigned off)
{
	op = remap_op(op);
	op2 = remap_op(op2);
	printf("\t%sb %u,x\n", op, off + 3);
	printf("\t%sa %u,x\n", op2, off + 2);
	if (cpu_has_y) {
		swap_d_y();
		printf("\t%sb %u,x\n", op2, off + 1);
		printf("\t%sa %u,x\n", op2, off);
		swap_d_y();
	} else {
		printf("\tpshb\n\tpsha");
		printf("\tldaa @hireg\n\tldab @hireg+1\n");
		printf("\t%sb %u,x\n", op2, off + 1);
		printf("\t%sa %u,x\n", op2, off);
		printf("\tstaa @hireg\n\tstab @hireg+1\n");
		printf("\tpula\n\tpulb\n");
	}
}

void op32d_on_ptr(const char *op, const char *op2, unsigned off)
{
	op = remap_op(op);
	op2 = remap_op(op2);
	if (!cpu_has_d) {
		op32_on_ptr(op, op2, off);
		return;
	}
	op = remap_op(op);
	printf("\t%sd %u,x\n", op, off + 2);
	if (cpu_has_y) {
		swap_d_y();
		printf("\t%sb %u,x\n", op2, off + 1);
		printf("\t%sa %u,x\n", op2, off);
		swap_d_y();
	} else {
		printf("\tpshb\n\tpsha");
		printf("\tldd @hireg\n");
		printf("\t%sb %u,x\n", op2, off + 1);
		printf("\t%sa %u,x\n", op2, off);
		printf("\tstd @hireg\n");
		printf("\tpula\n\tpulb\n");
	}
}

void uniop8_on_ptr(const char *op, unsigned off)
{
	op = remap_op(op);
	printf("\t%s %u,x\n", op, off);
}

void uniop16_on_ptr(const char *op, unsigned off)
{
	op = remap_op(op);
	printf("\t%s %u,x\n", op, off + 1);
	printf("\t%s %u,x\n", op, off);
}

void uniop32_on_ptr(const char *op, unsigned off)
{
	op = remap_op(op);
	printf("\t%s %u,x\n", op, off + 3);
	printf("\t%s %u,x\n", op, off + 2);
	printf("\t%s %u,x\n", op, off + 1);
	printf("\t%s %u,x\n", op, off);
}

void uniop8_on_s(const char *op, unsigned off)
{
	op = remap_op(op);
	printf("\t%s %u,s\n", op, off);
}

void uniop16_on_s(const char *op, unsigned off)
{
	op = remap_op(op);
	printf("\t%s %u,s\n", op, off + 1);
	printf("\t%s %u,s\n", op, off);
}

void uniop32_on_s(const char *op, unsigned off)
{
	op = remap_op(op);
	printf("\t%s %u,s\n", op, off + 3);
	printf("\t%s %u,s\n", op, off + 2);
	printf("\t%s %u,s\n", op, off + 1);
	printf("\t%s %u,s\n", op, off);
}

/* TODO: propogate down if we need to save B */
unsigned make_local_ptr(unsigned off, unsigned rlim)
{
	/* Both relative to frame base */
	int noff = off - x_fpoff;

	/* Although we can access arguments via S we sometimes still need
	   this path to make pointers to locals. We do need to go through
	   the cases we can just use ,s to make sure we avoid two steps */
	if (cpu_is_09) {
		printf("\tleax %u,s\n", off + sp);
		return 0;
	}

	printf(";make local ptr off %u, rlim %u noff %u\n", off, rlim, noff);

	/* TODO: if we can d a small < 7 or so shift by decreement then
	   it may beat going via tsx */
	if (x_fprel == 0 ||  noff < 0) {
		printf("\ttsx\n");
		x_fprel = 1;
		x_fpoff = 0;
	} else
		off = noff;

	off += sp;
	if (off <= rlim)
		return off;
	/* It is cheaper to inx than mess around with calls for smaller
	   values - 7 or 5 if no save needed */
	if (off - rlim < 7) {
		repeated_op(off - rlim, "inx");
		/* TODO: track */
		x_fpoff += off - rlim;
		return rlim;
	}
	if (off - rlim < 256) {
		printf("\tpshb\n");
		load_b_const(off - rlim);
		printf("\tjsr __abx\n");
		x_fpoff += off - rlim;
		printf("\tpulb\n");
		return rlim;
	} else {
		/* This case is (thankfully) fairly rare */
		printf("\tpshb\n\tpsha\n");
		load_d_const(off);
		printf("\tjsr __adx\n");
		x_fpoff += off;
		printf("\tpula\n\tpulb\n");
		return 0;
	}
}

/* Get pointer to the top of stack. We can optimize this in some cases
   when we track but it will be limited. The 6800 is quite weak on ops
   between register so we sometimes need to build ops against top of stack */
unsigned make_tos_ptr(void)
{
	if (cpu_is_09)
		printf("\ttfr s,x\n");
	else
		printf("\ttsx\n");
	x_fpoff = sp;
	x_fprel = 1;
	return 0;
}

unsigned op8_on_node(struct node *r, const char *op, unsigned off)
{
	unsigned v = r->value;

	op = remap_op(op);

	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		if (cpu_is_09)
			op8_on_s(op, v + off + sp);
		else {
			off = make_local_ptr(v + off, 255);
			op8_on_ptr(op, off);
		}
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

	op = remap_op(op);
	op2 = remap_op(op);

	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		if (cpu_is_09)
			op16_on_s(op, op2, v + off + sp);
		else { 
			off = make_local_ptr(v + off, 254);
			op16_on_ptr(op, op2, off);
		}
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
		set_d_node(r);
		break;
	case T_NSTORE:
	case T_NREF:
		printf("\t%sb _%s+%u\n", op, namestr(r->snum), v + off + 1);
		printf("\t%sa _%s+%u\n", op2, namestr(r->snum), v + off);
		break;
	case T_NAME:
		printf("\t%sb #<_%s+%u\n", op, namestr(r->snum), v + off);
		printf("\t%sa #>_%s+%u\n", op2, namestr(r->snum), v + off);
		set_d_node(r);
		break;
	/* case T_RREF:
		printf("\t%sb @__reg%u\n", v);
		break; */
	default:
		return 0;
	}
	return 1;
}

unsigned op16d_on_node(struct node *r, const char *op, const char *op2, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		if (cpu_is_09)
			op16d_on_s(op, op2, v + off + sp);
		else {
			off = make_local_ptr(v + off, 254);
			op16d_on_ptr(op, op2, off);
		}
		break;
	case T_CONSTANT:
		printf("\t%sd #%u\n", op, v + off);
		break;
	case T_LBSTORE:
	case T_LBREF:
		printf("\t%sd T%u+%u\n", op, r->val2, v + off);
		break;
	case T_LABEL:
		printf("\t%sd #T%u+%u\n", op, r->val2, v + off);
		set_d_node(r);
		break;
	case T_NSTORE:
	case T_NREF:
		printf("\t%sd _%s+%u\n", op, namestr(r->snum), v + off);
		break;
	case T_NAME:
		printf("\t%sd #_%s+%u\n", op, namestr(r->snum), v + off);
		set_d_node(r);
		break;
	/* case T_RREF:
		printf("\t%sd @__reg%u\n", v);
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

unsigned write_opd(struct node *r, const char *op, const char *op2, unsigned off)
{
	unsigned s = get_size(r->type);
	if (s == 2) {
		if (!cpu_has_d) 
			return op16_on_node(r, op, op2, off);
		return op16d_on_node(r, op, op2, off);
	}
	if (s == 1)
		return op8_on_node(r, op, off);
	return 0;
}

unsigned uniop8_on_node(struct node *r, const char *op, unsigned off)
{
	unsigned v = r->value;
	op = remap_op(op);
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		if (cpu_is_09)
			uniop8_on_s(op, off + sp);
		else {
			off = make_local_ptr(v + off, 255);
			uniop8_on_ptr(op, off);
		}
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
	op = remap_op(op);
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		if (cpu_is_09)
			uniop16_on_s(op, v + off + sp);
		else {
			off = make_local_ptr(v + off, 254);
			uniop16_on_ptr(op, off);
		}
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
	if (cpu_is_09)
		op8_on_spi(op);
	else {
		printf("\t%sb %u,x\n", remap_op(op), off);
		printf("\tins\n");
	}
}

void op16_on_tos(const char *op, const char *op2)
{
	unsigned off;
	if (cpu_is_09)
		op16_on_spi(op);
	else {
		off = make_tos_ptr();
		printf("\t%sb %u,x\n", remap_op(op), off + 1);
		printf("\t%sa %u,x\n", remap_op(op2), off);
		printf("\tins\n");
		printf("\tins\n");
	}
}

unsigned write_tos_op(struct node *n, const char *op, const char *op2)
{
	unsigned s = get_size(n->type);
	if (s > 2 && !cpu_has_y)
		return 0;
	if (s == 4) {
		swap_d_y();
		op16_on_tos(op2, op2);
		swap_d_y();
		op16_on_tos(op, op2);
	} else if (s == 2)
		op16_on_tos(op, op2);
	else
		op8_on_tos(op);
	invalidate_work();
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

/* TODO: addd  subd etc cases of this */
unsigned write_tos_uniop(struct node *n, const char *op)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	if (cpu_is_09) {
		if (s == 2)
			printf("\t%s ,-s\n", op);
		printf("\t%s ,-s\n", op);
		return 1;
	}
	op = remap_op(op);
	if (s == 2)
		uniop16_on_tos(op);
	else
		uniop8_on_tos(op);
	return 1;
}

/* TODO: decide how much we inline for -Os */

unsigned left_shift(struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned v;

	if (s > 2 || n->right->op != T_CONSTANT)
		return 0;
	v = n->right->value;
	if (s == 1) {
		if (v >= 8) {
			load_b_const(0);
			return 1;
		}
		invalidate_work();
		repeated_op(v, "lslb");
		return 1;
	}
	if (s == 2) {
		if (v >= 16) {
			load_d_const(0);
			return 1;
		}
		if (v >= 8) {
			load_a_b();
			load_b_const(0);
			v -= 8;
			if (v) {
				invalidate_work();
				repeated_op(v, "lsl");
			}
			return 1;
		}
		while(v--)
			printf("\tlslb\n\trola\n");
		invalidate_work();
		return 1;
	}
	return 0;
}

unsigned right_shift(struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned v;
	const char *op = "asr";

	if (n->type & UNSIGNED)
		op = "lsr";

	if (s > 2 || n->right->op != T_CONSTANT)
		return 0;
	v = n->right->value;
	if (s == 1) {
		if (v >= 8) {
			load_b_const(0);
			return 1;
		}
		invalidate_work();
		repeated_op(v, op);
		return 1;
	}
	if (s == 2) {
		if (v >= 16) {
			load_d_const(0);
			return 1;
		}
		if (v >= 8 && (n->type & UNSIGNED)) {
			load_b_a();
			load_a_const(0);
			v -= 8;
			if (v) {
				while(v--)
					printf("\t%sb\n", op);
				invalidate_work();
			}
			return 1;
		}
		while(v--)
			printf("\t%sa\n\trorb\n", op);
		invalidate_work();
		return 1;
	}
	return 0;
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
	case T_NREF:
	case T_NAME:
	case T_LABEL:
	case T_LOCAL:
	case T_ARGUMENT:
		return 1;
	}
	return 0;
}

/* For 6800 at least it is usually cheaper to reload even if the value
   we want is in D */
void load_x_with(struct node *r, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_ARGUMENT:
		v += argbase;
	case T_LOCAL:
		make_local_ptr(v + off, 0);
		break;
	case T_LREF:
		if (cpu_is_09)
			printf("\tldx %u,s\n", v + sp);
		else {
			off = make_local_ptr(v + off, 254);
			printf("\tldx %u,x\n", off);
		}
		invalidate_x();
		break;
	case T_CONSTANT:
		printf("\tldx #%u\n", v + off);
		invalidate_x();
		break;
	case T_LBREF:
		printf("\tldx T%u+%u\n", r->val2, v + off);
		invalidate_x();
		break;
	case T_LABEL:
		printf("\tldx #T%u+%u\n", r->val2, v + off);
		invalidate_x();
		break;
	case T_NREF:
		printf("\tldx _%s+%u\n", namestr(r->snum), v + off);
		invalidate_x();
		break;
	case T_NAME:
		printf("\tldx #_%s+%u\n", namestr(r->snum), v + off);
		invalidate_x();
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
	unsigned off;

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
		printf(";merge right %x %lu+%lu\n",
			op, l->value, r->value);
		l->value += r->value;
		free_node(r);
		free_node(n);
		return l;
	}
	/* Turn a deref of an offset to an object into a single op so we can
	   generate a single lda offset,x in the code generator. This happens
	   in some array dereferencing and especially struct pointer access */
	if (op == T_DEREF || op == T_DEREFPLUS) {
		if (op == T_DEREF)
			n->value = 0;	/* So we can treat deref/derefplus together */
		if (r->op == T_PLUS) {
			off = n->value + r->right->value;
			if (r->right->op == T_CONSTANT && off < 253) {
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
	if (op == T_EQ || op == T_EQPLUS) {
		if (op == T_EQ)
			n->value = 0;	/* So we can treat deref/derefplus together */
		if (l->op == T_PLUS) {
			off = n->value + l->right->value;
			if (l->right->op == T_CONSTANT && off < 253) {
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
	if (argsize == 0 || cpu_has_d || (func_flags & F_VARARG))
		printf("\trts\n");
	else if (argsize <= 8)
		printf("\tjmp __cleanup%u\n", argsize);
	else {
		/* Icky - can we do better remembering AB is live for
		   non void funcs */
		printf("\tjmp __cleanup\n");
		printf("\t.word %u\n", argsize);
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
	printf("\tjmp L%d%s\n", n, tail);
	unreachable = 1;
	return 0;
}

void gen_jump(const char *tail, unsigned n)
{
	/* 6809 - option for lbra ? */
	printf("\tjmp L%d%s\n", n, tail);
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
	printf("\tldx #Sw%u\n", n);
	printf("\tjmp __switch");
	helper_type(type, 0);
	printf("\n");
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
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

void gen_helpcall(struct node *n)
{
	invalidate_all();
	printf("\tjsr __");
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
		cpu_is_09 = 1;
		cpu_has_d = 1;
		cpu_has_pshx = 1;
		cpu_has_y = 1;
		cpu_has_abx = 1;
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
	if (cpu != 6809)
		printf("\t.setcpu %u\n", cpu);
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
	if (cpu_is_09) {
		/* 6809 has differing ops */
		switch(size) {
		case 1:
			printf("\tstb ,-s\n");
			return 1;
		case 2:
			printf("\tstd ,--s\n");
			return 1;
		case 4:	/* Have to split them to get the order right */
			printf("\tstd ,--s\n");
			printf("\tsty ,--s\n");
			return 1;
		}
		return 0;
	}
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
		invalidate_work();
		return 1;
	}
	return 0;
}

unsigned cmp_direct(struct node *n, const char *uop, const char *op)
{
	unsigned s = get_size(n->right->type);
	unsigned v = n->right->value;

	if (n->right->op != T_CONSTANT)
		return 0;
	if (n->right->type & UNSIGNED)
		op = uop;
	if (s == 1) {
		printf("\tcmpb #%u\n", v);
		printf("\tjsr %s\n", op);
		n->flags |= ISBOOL;
		return 1;
	}
	if (s == 2 && cpu_has_d) {
		printf("\tsubd #%u\n", v);
		printf("\tjsr %s\n", op);
		n->flags |= ISBOOL;
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
	unsigned v;

	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		sp -= r->value;
		if (cpu_has_d || n->val2) /* Varargs */
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
		return write_opd(r, "add", "adc", 0);
	case T_MINUS:
		if (r->op == T_CONSTANT) {
			if (s == 2) {
				add_d_const(-r->value);
				return 1;
			}
			if (s == 1) {
				add_b_const(-r->value);
				b_val -= r->value;
				return 1;
			}
		}
		return write_op(r, "sub", "sbc", 0);
	case T_AND:
		if (r->op == T_CONSTANT && s <= 2) {
			v = r->value;
			if ((v & 0xFF) != 0xFF) {
				if (v & 0xFF)
					printf("\tandb #%u\n", v & 0xFF);
				else
					printf("\tclrb\n");
				modify_b(b_val & v);
			}
			if (s == 2) {
				v >>= 8;
				if (v != 0xFF) {
					if (v)
						printf("\tanda #%u\n", v);
					else
						printf("\tclra");
				}
				modify_a(a_val & v);
			}
			return 1;
		}
		return write_op(r, "and", "and", 0);
	case T_OR:
		if (r->op == T_CONSTANT && s <= 2) {
			v = r->value;
			if (v & 0xFF) {
				printf("\t%sb #%u\n", remap_op("or"), v & 0xFF);
				modify_b(b_val | v);
			}
			if (s == 2) {
				v >>= 8;
				if (v)
					printf("\t%sa #%u\n", remap_op("or"), v);
			}
			modify_a(a_val | v);
			return 1;
		}
		return write_op(r, "or", "or", 0);
	case T_HAT:
		if (r->op == T_CONSTANT && s <= 2) {
			v = r->value;
			if (v & 0xFF) {
				if ((v & 0xFF) == 0xFF)
					printf("\tcomb\n");
				else
					printf("\teorb #%u\n", v & 0xFF);
				modify_b(b_val ^ v);
			}
			if (s == 2) {
				v >>= 8;
				if (v ) {
					if (v == 0xFF)
						printf("\tcoma\n");
					else
						printf("\teora #%u\n", v);
				}
				modify_a(a_val ^ v);
			}
			return 1;
		}
		return write_op(r, "eor", "eor", 0);
	case T_LTLT:
		return left_shift(n);
	case T_GTGT:
		return right_shift(n);
	case T_EQEQ:
		return cmp_direct(n, "booleq", "booleq");
	case T_BANGEQ:
		return cmp_direct(n, "boolne", "boolne");
	case T_LT:
		return cmp_direct(n, "boolult", "boollt");
	case T_GT:
		return cmp_direct(n, "boolugt", "boolgt");
	case T_LTEQ:
		return cmp_direct(n, "boolule", "boolle");
	case T_GTEQ:
		return cmp_direct(n, "booluge", "boolge");
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
	unsigned off;

	switch(n->op) {
	case T_LEQ:
		/* We have a specific optimization case that occurs a lot
		   *auto = 0, that we can optimize nicely */
		if (r->op == T_CONSTANT && r->value == 0) {
			if (cpu_is_09) {
				if (s == 1)
					uniop8_on_s("clr", n->val2 + sp);
				else if (s == 2)
					uniop16_on_s("clr", n->val2 + sp);
				else
					uniop32_on_s("clr", n->val2 + sp);
				return 1;
			}
			/* Offset of pointer in local */
			/* val2 is the local offset, value the data offset */
			off = make_local_ptr(n->value, 256 - s);
			/* off,X is now the pointer */
			printf("\tldx %u,x\n", off);
			invalidate_x();
			if (s == 1)
				uniop8_on_ptr("clr", n->val2);
			else if (s == 2)
				uniop16_on_ptr("clr", n->val2);
			else
				uniop32_on_ptr("clr", n->val2);
			return 1;
		}
		return 0;
	/* Writes of 0 to an object we can use clr for providing the
	   result is not then re-used */
	case T_LSTORE:
	case T_LBSTORE:
	case T_NSTORE:
		/* Optimizations for constants */
		if (nr && r->op == T_CONSTANT && r->value == 0) {
			if (write_uni_op(n, "clr", 0)) {
				set_d_node(n);
				return 1;
			}
		}
		return 0;
	}
	return 0;
}

/*
 *	Try and build an op where we load X with the pointer,
 *	AB with the data and call a helper. Some of these may also
 *	benefit from inline forms later. 32bit also works as the
 *	value ends up in @hireg|Y/AB which is all safe from the load of
 *	the X pointer.
 */
 
void op_on_ptr(struct node *n, const char *op)
{
	unsigned s = get_size(n->type);
	if (s == 1)
		op8_on_ptr(op, 0);
	else if (s == 2)
		op16_on_ptr(op, op, 0);
	else
		op32_on_ptr(op,op, 0);
}
		
void opd_on_ptr(struct node *n, const char *op, const char *op2)
{
	unsigned s = get_size(n->type);
	if (s == 1)
		op8_on_ptr(op, 0);
	else if (s == 2)
		op16d_on_ptr(op, op2, 0);
	else
		op32_on_ptr(op, op2, 0);
}

/* TODO: 6809 could index locals via S so if n->left is LOCAL or
   ARGUMENT we can special case it */
unsigned do_xeqop(struct node *n, const char *op)
{
	if (!can_load_x_with(n->left, 0))
		return 0;
	/* Get the value part into AB */
	codegen_lr(n->right);
	/* Load X (lval of the eq op) up (doesn't disturb AB) */
	load_x_with(n->left, 0);
	/* Things we can then inline */
	switch(n->op) {
	case T_ANDEQ:
		op_on_ptr(n, "and");
		break;
	case T_OREQ:
		op_on_ptr(n, remap_op("or"));
		break;
	case T_HATEQ:
		op_on_ptr(n, "eor");
		break;
	case T_PLUSEQ:
		opd_on_ptr(n, "add", "adc");
		break;
	/* TODO:
	   MINUSEQ
	   PLUSPLUS
	   MINUSMINUS
	   shift prob not */
	default:
		helper_s(n, op);
		return 1;
	}
	/* Stuff D back into ,X */
	opd_on_ptr(n, "st", "st");
	return 1;
}

/*
 *	Things we can try and do as a direct memory op for bytes
 */

unsigned memop_const(struct node *n, const char *op)
{
	char buf[32];
	unsigned v;
	struct node *l = n->left;
	struct node *r = n->right;
	v = l->value;
	if (r->op != T_CONSTANT)
		return 0;
	/* The helper has to load x and a value and make a cal
	   so is quite expensive */
	if (r->value > 5 && opt < 2)
		return 0;
	switch(l->op) {
	case T_LABEL:
		sprintf(buf, "%s T%u+%u", op, l->val2, v);
		repeated_op(r->value, buf);
		invalidate_mem();
		return 1;
	case T_NAME:
		sprintf(buf, "%s _%s+%u", op, namestr(l->snum), v);
		repeated_op(r->value, buf);
		invalidate_mem();
		return 1;
	/* No ,x forms so cannot do locals */
	}
	return 0;
}

unsigned memop_shift(struct node *n, const char *op, const char *opu)
{
	char buf[32];
	unsigned v;
	unsigned rv;
	struct node *l = n->left;
	struct node *r = n->right;
	v = l->value;
	rv = r->value;
	if (r->op != T_CONSTANT)
		return 0;

	/* Right shifts are sign specific */
	if (n->type & UNSIGNED)
		op = opu;
	if (rv > 7) {
		/* Undefined but do something nice */
		rv = 1;
		op = "clr";
	}
	if (rv > 2 && opt < 2)
		return 0;
	switch(l->op) {
	case T_LABEL:
		sprintf(buf, "%s T%u+%u", op, l->val2, v);
		repeated_op(r->value, buf);
		invalidate_mem();
		return 1;
	case T_NAME:
		sprintf(buf, "%s _%s+%u", op, namestr(l->snum), v);
		repeated_op(r->value, buf);
		invalidate_mem();
		return 1;
	case T_LOCAL:
		/* No ,x forms so cannot do locals */
		if (!cpu_is_09)
			return 0;
		sprintf(buf, "%s %u,s", op, v + sp);
		repeated_op(r->value, buf);
		invalidate_mem();
		return 1;
	}
	return 0;
}

/* Generate inline code for ++ and -- operators when it makes sense */
unsigned add_to_node(struct node *n, int sign, int retres)
{
	struct node *r = n->right;
	unsigned s = get_size(n->type);
	if (s > 2 || r->op != T_CONSTANT)
		return 0;
	/* Might be worth special casing 6809 TODO
	   Something for local like
	           ldd #n
	           addd [n,s]
	           std [n,s]
	           {subd #n} */
	/* It's marginal whether we should go via X or not */
	if (!can_load_x_with(n->left, 0))
		return 0;
	load_x_with(n->left, 0);
	if (s == 1) {
		op8_on_ptr("ld", 0);
		if (!retres) {
			if (cpu_is_09)
				printf("\tstb ,-s\n");
			else
				printf("\tpshb\n");
		}
		add_b_const(sign * r->value);
		op8_on_ptr("st", 0);
		if (!retres) {
			if (cpu_is_09)
				printf("\tldb ,s+\n");
			else
				printf("\tpulb\n");
		}
		invalidate_work();
		invalidate_mem();
		set_d_node(n->left);
		return 1;
	}
	op16d_on_ptr("ld", "ld", 0);
	if (!retres) {
		if (cpu_is_09)
			printf("\tstd ,--s\n");
		else
			printf("\tpshb\n\tpsha\n");
	}
	add_d_const(sign * r->value);
	op16d_on_ptr("st", "st", 0);
	if (!retres) {
		if (cpu_is_09)
			printf("\tldd ,s++\n");
		else
			printf("\tpula\n\tpulb\n");
	}
	invalidate_work();
	invalidate_mem();
	set_d_node(n->left);
	return 1;
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
	unsigned v;
	/* Don't generate unreachable code */
	if (unreachable)
		return 1;
	switch(n->op) {
	case T_DEREF:
	case T_DEREFPLUS:
		/* Our right hand side is the thing to deref. See if we can
		   get it into X instead */
		printf(";deref r %x %u\n", r->op, (unsigned)r->value);
		if (can_load_x_with(r, 0)) {
			v = n->value;
			load_x_with(r, 0);
			invalidate_work();
			switch(s) {
			case 1:
				printf("\t%sb %u,x\n", remap_op("ld"), v);
				return 1;
			case 2:
				if (cpu_has_d)
					printf("\tldd %u,x\n", v);
				else {
					printf("\tldaa %u,x\n", v);
					printf("\tldab %u,x\n", v + 1);
				}
				return 1;
			case 4:
				if (cpu_has_y) {
					printf("\tldd %u,x\n", v + 2);
					printf("\tldy %u,x\n", v);
					return 1;
				}
				if (cpu_has_d)
					printf("\tldd %u,x\n", v);
				else {
					printf("\tldaa %u,x\n", v);
					printf("\tldab %u,x\n", v + 1);
				}
				if (cpu_has_d)
					printf("\tldd @hireg\n");
				else {
					printf("\tstaa @hireg\n");
					printf("\tstab @hireg+1\n");
				}
				if (cpu_has_d)
					printf("\tldd %u,x\n", v + 2);
				else {
					printf("\tldaa %u,x\n", v + 2);
					printf("\tldab %u,x\n", v + 3);
				}
				return 1;
			default:
				error("sdf");
			}
		}
		return 0;
	case T_EQ:	/* Our left is the address */
	case T_EQPLUS:
		if (can_load_x_with(l, 0)) {
			if (r->op == T_CONSTANT && nr && r->value == 0 && s <= 2) {
				/* We can optimize thing = 0 for the case
				   we don't also need the value */
				load_x_with(l, 0);
				write_uni_op(n, "clr", 0);
				invalidate_mem();
				return 1;
			}
			v = n->value;
			codegen_lr(r);
			load_x_with(l, 0);
			invalidate_mem();
			switch(s) {
			case 1:
				printf("\t%sb %u,x\n", remap_op("st"), v);
				return 1;
			case 2:
				if (cpu_has_d)
					printf("\tstd %u,x\n", v);
				else {
					printf("\tstaa %u,x\n", v);
					printf("\tstab %u,x\n", v + 1);
				}
				return 1;
			case 4:
				if (cpu_has_y) {
					printf("\tsty %u,x\n", v);
					printf("\tstd %u,x\n", v + 2);
					return 1;
				}					
				if (cpu_has_d)
					printf("\tstd %u,x\n", v + 2);
				else {
					printf("\tstaa %u,x\n", v + 2);
					printf("\tstab %u,x\n", v + 3);
				}
				if (!nr)
					printf("\tpshb\n\tpsha\n");
				if (cpu_has_d) {
					printf("\tldd @hireg\n");
					printf("\tstd %u,x\n", v);
				} else {
					printf("\tldaa @hireg\n");
					printf("\tldab @hireg+1\n");
					printf("\tstaa %u,x\n", v);
					printf("\tstab %u,x\n", v + 1);
				}
				if (!nr)
					printf("\tpula\n\tpulb\n");
				return 1;
			default:
				error("seqf");
			}
		}
		return 0;
	case T_PLUSEQ:
		if (s == 1 && nr && memop_const(n, "inc"))
			return 1;
		if (s == 2 && add_to_node(n, 1, 1))
			return 1;
		return do_xeqop(n, "xpluseq");
	case T_MINUSEQ:
		if (s == 1 && nr && memop_const(n, "dec"))
			return 1;
		if (s == 2 && add_to_node(n, -1, 1))
			return 1;
		return do_xeqop(n, "xminuseq");
	case T_PLUSPLUS:
		if (s == 1 && nr && memop_const(n, "inc"))
			return 1;
		if (s == 2 && add_to_node(n, 1, nr))
			return 1;
		return do_xeqop(n, "xplusplus");
	case T_MINUSMINUS:
		if (s == 1 && nr && memop_const(n, "dec"))
			return 1;
		if (s == 2 && add_to_node(n, -1, nr))
			return 1;
		return do_xeqop(n, "xmminus");
	case T_STAREQ:
		return do_xeqop(n, "xmuleq");
	case T_SLASHEQ:
		return do_xeqop(n, "xdiveq");
	case T_PERCENTEQ:
		return do_xeqop(n, "xremeq");
	case T_SHLEQ:
		if (s == 1 && nr && memop_shift(n, "lsl", "lsl"))
			return 1;
		return do_xeqop(n, "xshleq");
	case T_SHREQ:
		if (s == 1 && nr && memop_shift(n, "asr", "lsr"))
			return 1;
		return do_xeqop(n, "xshreq");
	case T_ANDEQ:
		return do_xeqop(n, "xandeq");
	case T_OREQ:
		return do_xeqop(n, "xoreq");
	case T_HATEQ:
		return do_xeqop(n, "xhateq");
	case T_MINUS:
		/* Do it backwards if not a const and on a 6809 */
		if (r->op == T_CONSTANT || s > 2 || cpu_is_09 == 0)
			return 0;
		codegen_lr(r);
		/* Try and write it as 
			ldd foo  subd 1,s or similar */
		if (write_opd(l, "sub", "sbc", 0))
			return 1;
		/* Ok then we have to write it the long way */
		gen_push(r);
		codegen_lr(l);
		if (s == 1)
			op8_on_spi("sub");
		else
			op16d_on_spi("sub");
		/* our op takes it back off stack */
		sp -= get_stack_size(r->type);
		return 1;
	}
	return 0;
}

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

	/* Floats and stuff handled by helper */
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		return 0;

	ls = get_size(lt);
	rs = get_size(rt);

	/* Size shrink is free */
	if (ls <= rs)
		return 1;
	/* Don't do the harder ones */
	if (!(rt & UNSIGNED)) {
		if (cpu_is_09) {
			if (rs == 1) { 
				printf("\tsex\n");
				return 1;
			}
			/* TODO: 6309 has sexw */
		}
		return 0;
	}
	if (rs == 1)
		load_a_const(0);
	if (ls == 4) {
		if (cpu_has_y)
			printf("\tldy #0\n");
		else
			printf("\tclr @hireg\n\tclr @hireg+1\n");
	}
	return 1;
}

unsigned cmp_op(struct node *n, const char *uop, const char *op)
{
	unsigned s = get_size(n->right->type);
	if (n->right->type & UNSIGNED)
		op = uop;
	if (cpu_is_09) {
		if (s > 2)	/* For now anyway */
			return 0;
		/* We can do this versus s+ or s++ */
		if (s == 1)
			op8_on_spi("cmp");
		else if (s == 2)
			op16d_on_spi("sub");
		printf("\tjsr %s\n", op);
		n->flags |= ISBOOL;
		return 1;
	}
	if (s == 1) {
		op8_on_ptr("cmp", 0);
		printf("\tjsr %s\n", op);
		n->flags |= ISBOOL;
		return 1;
	}
	if (s == 2 && cpu_has_d) {
		op16d_on_ptr("sub", "sbc", 0);
		printf("\tins\n");
		printf("\tins\n");
		printf("\tjsr %s\n", op);
		n->flags |= ISBOOL;
		return 1;
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned nr = n->flags & NORETURN;
	unsigned v;
	unsigned off;

	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_stack_size(n->left->type);
	v = n->value;
	switch (n->op) {
	case T_CALLNAME:
		invalidate_all();
		printf("\tjsr _%s+%u\n", namestr(n->snum), v);
		return 1;
	case T_DEREF:
	case T_DEREFPLUS:
		make_x_d();
		if (s == 1) {
			op8_on_ptr("ld", v);
			invalidate_a();
			return 1;
		}
		if (s == 2) {
			op16d_on_ptr("ld", "ld", v);
			set_d_node(n);
			return 1;
		}
		if (s == 4 && cpu_has_y) {
			printf("\tldy ,x\n");
			printf("\tldd 2,x\n");
			set_d_node(n);	/* TODO: review 32 bit cases */
			return 1;
		}
		break;
	case T_EQ:	/* Assign - ToS is address, working value is value */
	case T_EQPLUS:
		invalidate_mem();
		if (s == 1) {
			pop_x();
			op8_on_ptr("st", v);
			return 1;
		}
		if (s == 2) {
			pop_x();
			op16d_on_ptr("st", "st", v);
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
		/* size 4 varies */
		if (cpu_has_y) {
			load_d_const(v);
			/* TODO: tracking on Y ? */
			printf("\tldy #%u\n", (unsigned)(n->value >> 16));
			return 1;
		}
		if (cpu_has_d) {
			load_d_const(n->value >> 16);
			printf("\tstd @hireg\n");
			load_d_const(n->value);
			return 1;
		}
		load_d_const(n->value >> 16);
		printf("\tstaa @hireg\n");
		printf("\tstab @hireg\n");
		load_d_const(n->value);
		return 1;
	case T_LABEL:
	case T_NAME:
	case T_LREF:
	case T_NREF:
	case T_LBREF:
		if (d_holds_node(n))
			return 1;
		if (write_opd(n, "ld", "ld", 0)) {
			set_d_node(n);
			return 1;
		}
		return 0;
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
		if (write_opd(n, "st", "st", 0)) {
			invalidate_mem();
			set_d_node(n);
			return 1;
		}
		break;
	case T_ARGUMENT:
		if (d_holds_node(n))
			return 1;
		v += argbase;
	case T_LOCAL:
		/* For v != 0 case it would be more efficient to load
		   const then add @tmp/tmp+1 TODO */
		if (d_holds_node(n))
			return 1;
		move_s_d();
		v += sp;
		add_d_const(v);
		set_d_node(n);
		return 1;
	case T_LDEREF:
		if (cpu_is_09) {
			if (n->val2 == 0 && s <= 2) {
				if (s == 1)
					printf("\tldb [%u,s]\n", v + sp);
				else
					printf("\tldd [%u,s]\n", v + sp);
				return 1;
			} else {
				printf("\tldx %u,s\n", v + sp);
			}
		} else {
			/* Offset of pointer in local */
			/* val2 is the local offset, value the data offset */
			off = make_local_ptr(v, 256 - s);
			/* off,X is now the pointer */
			printf("\tldx %u,x\n", off);
		}
		invalidate_x();
		v = n->val2;
		if (s == 1)
			op8_on_ptr("ld", v);
		else if (s == 2)
			op16d_on_ptr("ld", "ld", v);
		else
			op32d_on_ptr("ld", "ld", v);
		invalidate_work();
		return 1;
	case T_LEQ:
		/* We probably want some indirecting helpers later */
		if (cpu_is_09) {
			if (v == 0 && s <= 2) {
				if (s == 1)
					printf("\tstb [%u,s]\n", n->val2 + sp);
				else
					printf("\tstd [%u,s]\n", n->val2 + sp);
				return 1;
			}
			printf("\nldx %u,s\n", n->val2 + sp);
		} else {
			/* Offset of pointer in local */
			off = make_local_ptr(n->val2, 256 - s);
			/* off,X is now the pointer */
			printf("\tldx %u,x\n", off);
		}
		invalidate_x();
		if (s == 1)
			op8_on_ptr("st", v);
		else if (s == 2)
			op16d_on_ptr("st", "st", v);
		else
			op32d_on_ptr("st", "st", v);
		return 1;
	/* Type casting */
	case T_CAST:
		if (nr)
			return 1;
		return gen_cast(n);
	/* Single argument ops we can do directly */
	case T_TILDE:
		if (s == 4)
			return 0;
		if (s == 2) {
			printf("\tcoma\n");
			printf("\tcomb\n");
			modify_a(~a_val);
			modify_b(~b_val);
		} else {
			printf("\tcomb\n");
			modify_b(~b_val);
		}
		return 1;
	case T_NEGATE:
		if (s == 2) {
			printf("\tcoma\n");
			printf("\tcomb\n");
			modify_a(~a_val);
			modify_b(~b_val);
			add_d_const(1);
			return 1;
		}
		if (s == 1) {
			printf("\tnegb\n");
			modify_b(-b_val);
			return 1;
		}
		return 0;
	/* Double argument ops we can handle easily */
	case T_PLUS:
		/* TODO - with D reg this one needs special handling */
		return write_tos_op(n, "add", "adc");
	case T_AND:
		return write_tos_op(n, "and", "and");
	case T_OR:
		return write_tos_op(n, "or", "or");
	case T_HAT:
		return write_tos_op(n, "eor", "eor");
	/* These do the maths backwards in effect so use the other equvialent
	   compare for the ordered part */
	case T_EQEQ:
		return cmp_op(n, "booleq", "booleq");
	case T_BANGEQ:
		return cmp_op(n, "boolne", "boolne");
	case T_LT:
		return cmp_op(n, "boolugt", "boolgt");
	case T_GT:
		return cmp_op(n, "boolult", "boollt");
	case T_LTEQ:
		return cmp_op(n, "booluge", "boolge");
	case T_GTEQ:
		return cmp_op(n, "boolule", "boolle");
	}
	return 0;
}

/* TODO
	WIP Track X v S offset
	Conditions (EQEQ BANGEQ LT GT LTEQ GTEQ) - optimize < 0 and
	other easy ones ?
	BANG
	BOOL
	STAR SLASH PERCENT (const optimizations)
	WIP Track register values
	Track condition codes
	Think about how to improve long handling
	Arg helpers like Z8 etc so we can optimize post 6800 a bit
	and also optimize push const cases and long consts especially
	(and probably push arg)
	Optimize arg pushes if have pshx by using LDX PSHX for consts
	Add all the general operations we can do 16bit with a 6803
*/
