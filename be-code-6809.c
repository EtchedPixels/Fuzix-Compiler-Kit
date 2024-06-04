#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"
#include "backend-6800.h"

/*
 *	6809 implementation of the code generation section
 *	We have Y as the upper bits of the working value, and we have
 *	U as a register. There are no complications about index ranges,
 *	and we can index off stack. 6809 is easy mode.
 */

/* 16bit constant load */
void load_d_const(uint16_t n)
{
	unsigned hi,lo;

	lo = n & 0xFF;
	hi = n >> 8;

/*	printf(";want %04X have %02X:%02X val %d %d\n",
		n, a_val, b_val, a_valid, b_valid); */

	if (n == 0) {
		if (!a_valid || a_val)
			printf("\tclra\n");
		if (!b_valid || b_val)
			printf("\tclrb\n");
	} else if (!a_valid || !b_valid || a_val != hi || b_val != lo)
		printf("\tldd #%u\n", n);

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
	else if (b_valid && n == b_val)
		printf("\ttfr b,a\n");
	else
		printf("\t%sa #%u\n", ld8_op, n & 0xFF);
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
	else if (a_valid && n == a_val)
		printf("\ttfr a,b\n");
	else
		printf("\t%sb #%u\n", ld8_op, n & 0xFF);
	b_valid = 1;
	b_val = n;
	d_valid = 0;
}

void add_d_const(uint16_t n)
{
	if (n == 0)
		return;

	/* TODO: can do better in terms of obj/offset but not clear it is
	   that useful */

	d_valid = 0;

	printf("\taddd #%u\n", n);
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
	printf("\ttfr b,a\n");
	a_val = b_val;
	a_valid = b_valid;
	d_valid = 0;
}

void load_b_a(void)
{
	printf("\ttfr a,b\n");
	b_val = a_val;
	b_valid = a_valid;
	d_valid = 0;
}

void move_s_d(void)
{
	printf("\ttfr s,d\n");
	invalidate_work();
}

void move_d_s(void)
{
	printf("\ttfr d,s\n");
}

void swap_d_y(void)
{
	printf("\texg d,y\n");
}

void swap_d_x(void)
{
	printf("\texg d,x\n");
	invalidate_work();
	invalidate_x();
}

/* Get D into X (may trash D) */
void make_x_d(void)
{
	printf("\ttfr d,x\n");
	invalidate_x();
}

void pop_x(void)
{
	printf("\tpuls x\n");
	invalidate_x();
}

/*
 *	There are multiple strategies depnding on chip features
 *	available.
 */
void adjust_s(int n, unsigned save_d)
{
	if (n)
		printf("\tleas %d,s\n", n);
	return;
}

void op8_on_ptr(const char *op, unsigned off)
{
	printf("\t%sb %u,x\n", op, off);
}

/* Do the low byte first in case it's add adc etc */
void op16_on_ptr(const char *op, const char *op2, unsigned off)
{
	/* Big endian */
	printf("\t%sb %u,x\n", op, off + 1);
	printf("\t%sa %u,x\n", op2, off);
}

/* Operations where D can be used on later processors */
void op16d_on_ptr(const char *op, const char *op2, unsigned off)
{
	/* Big endian */
	printf("\t%sd %u,x\n", op, off);
}

static void op8_on_s(const char *op, unsigned off)
{
	printf("\t%sb %u,s\n", op, off);
}

static void op8_on_tos(const char *op)
{
	printf("\t%sb ,s+\n", op);
}

/* Do the low byte first in case it's add adc etc */
static void op16_on_s(const char *op, const char *op2, unsigned off)
{
	/* Big endian */
	printf("\t%sb %u,s\n", op, off + 1);
	printf("\t%sa %u,s\n", op2, off);
}


static void op16d_on_s(const char *op, const char *op2, unsigned off)
{
	/* Big endian */
	printf("\t%sd %u,s\n", op, off);
}

static void op32_on_ptr(const char *op, const char *op2, unsigned off)
{
	printf("\t%sb %u,x\n", op, off + 3);
	printf("\t%sa %u,x\n", op2, off + 2);
	swap_d_y();
	printf("\t%sb %u,x\n", op2, off + 1);
	printf("\t%sa %u,x\n", op2, off);
	swap_d_y();
}

void op32d_on_ptr(const char *op, const char *op2, unsigned off)
{
	printf("\t%sd %u,x\n", op, off + 2);
	swap_d_y();
	printf("\t%sb %u,x\n", op2, off + 1);
	printf("\t%sa %u,x\n", op2, off);
	swap_d_y();
}

void load32(unsigned off)
{
	printf("\tldy %u,x\n\tldd %u,x\n", off, off + 2);
}

void uniop_on_ptr(register const char *op, register unsigned off,
						register unsigned size)
{
	off += size;
	while(size--)
		printf("\t%s %u,x\n", op, --off);
}

unsigned make_local_ptr(unsigned off, unsigned rlim)
{
	printf("\tleax %u,s\n", off + sp);
	return 0;
}

/* Get pointer to the top of stack. We can optimize this in some cases
   when we track but it will be limited. The 6800 is quite weak on ops
   between register so we sometimes need to build ops against top of stack */
unsigned make_tos_ptr(void)
{
	printf("\ttfr s,x\n");
	x_fpoff = sp;
	x_fprel = 1;
	return 0;
}

static char *addr_form(register struct node *r, unsigned off, unsigned s)
{
	static char addr[32];
	const char *mod = "";
	unsigned v = r->value;

	if (s == 1)
		mod = "<";

	switch(r->op) {
	case T_CONSTANT:
		if (s == 1)
			v &= 0xFF;
		sprintf(addr, "#%s%u", mod, v + off);
		return addr;
	case T_NAME:
		sprintf(addr, "#%s_%s+%u", mod, namestr(r->snum), v + off);
		return addr;
	case T_NSTORE:
	case T_NREF:
		sprintf(addr, "_%s+%u%s", namestr(r->snum), v + off, pic_op);
		return addr;
	case T_LABEL:
		sprintf(addr, "#%sT%u+%u", mod, r->val2, v + off);
		return addr;
	case T_LBSTORE:
		sprintf(addr, "T%u+%u%s", r->val2, v + off, pic_op);
		return addr;
	case T_LBREF:
		sprintf(addr, "T%u+%u%s", r->val2, v + off, pic_op);
		return addr;
	/* Only occurs on 6809 */
	case T_RDEREF:
		sprintf(addr, "%u,u", r->val2 + off);
		return addr;
	default:
		error("aform");
	}
	return NULL;
}

/* These functions must not touch X on the 6809, they can on others */
unsigned op8_on_node(struct node *r, const char *op, unsigned off)
{
	unsigned v = r->value;

	invalidate_work();

	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		op8_on_s(op, v + off + sp);
		break;
	case T_CONSTANT:
	case T_LBSTORE:
	case T_LBREF:
	case T_LABEL:
	case T_NSTORE:
	case T_NREF:
	case T_NAME:
		printf("\t%sb %s\n", op, addr_form(r, off, 1));
		break;
	default:
		return 0;
	}
	return 1;
}

/* Do the low byte first in case it's add adc etc */
unsigned op16_on_node(struct node *r, const char *op, const char *op2, unsigned off)
{
	unsigned v = r->value;

	invalidate_work();

	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		op16_on_s(op, op2, v + off + sp);
		break;
	case T_CONSTANT:
		printf("\t%sa #>%u\n", op, (v + off) & 0xFFFF);
		printf("\t%sb #<%u\n", op2, (v + off) & 0xFFFF);
		break;
	case T_LBSTORE:
	case T_LBREF:
	case T_LABEL:
	case T_NSTORE:
	case T_NREF:
	case T_NAME:
		printf("\t%sa %s\n", op, addr_form(r, off, 1));
		printf("\t%sb %s\n", op2, addr_form(r, off + 1, 1));
		break;
	default:
		return 0;
	}
	return 1;
}

unsigned op16d_on_node(struct node *r, const char *op, const char *op2, unsigned off)
{
	unsigned v = r->value;

	invalidate_work();
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		op16d_on_s(op, op2, v + off + sp);
		break;
	case T_CONSTANT:
	case T_LBSTORE:
	case T_LBREF:
	case T_LABEL:
	case T_NSTORE:
	case T_NREF:
	case T_NAME:
		printf("\t%sd %s\n", op, addr_form(r, off, 2));
		break;
	default:
		return 0;
	}
	return 1;
}

unsigned op16y_on_node(struct node *r, const char *op, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		printf("\t%sy %u,s\n", op, v + off + sp);
		break;
	case T_CONSTANT:
	case T_LBSTORE:
	case T_LBREF:
	case T_LABEL:
	case T_NSTORE:
	case T_NREF:
	case T_NAME:
		printf("\t%sy %s\n", op, addr_form(r, off, 2));
	default:
		return 0;
	}
	return 1;
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
	if (s == 2)
		return op16d_on_node(r, op, op2, off);
	if (s == 1)
		return op8_on_node(r, op, off);
	return 0;
}

unsigned write_uni_op(register struct node *r, const char *op, unsigned off)
{
	unsigned v = r->value;
	unsigned s = get_size(r->type);

	if (s == 4)
		return 0;

	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		if (s == 2)
			printf("\t%s %u,s\n", op, v + off + 1);
		printf("\t%s %u,s\n", op, v + off);
		break;
	case T_LBSTORE:
	case T_LBREF:
	case T_NSTORE:
	case T_NREF:
		printf("\t%s %s\n", op, addr_form(r, off, 1));
		if (s == 2)
			printf("\t%s %s\n", op, addr_form(r, off + 1, 1));
		break;
	default:
		return 0;
	}
	return 1;
}

static void op16_on_tos(const char *op)
{
	invalidate_work();		/* ?? needed on 09 ? */
	printf("\t%sa ,s+\n", op);
	printf("\t%sb ,s+\n", op);
}

void op16d_on_tos(const char *op)
{
	invalidate_work();
	printf("\t%sd ,s++\n", op);
}

unsigned write_tos_op(struct node *n, const char *op)
{
	unsigned s = get_size(n->type);
	if (s == 4) {
		swap_d_y();
		op16_on_tos(op);
		swap_d_y();
		op16_on_tos(op);
	} else if (s == 2)
		op16_on_tos(op);
	else
		op8_on_tos(op);
	invalidate_work();
	return 1;
}

unsigned write_tos_opd(struct node *n, const char *op, const char *op2)
{
	unsigned s = get_size(n->type);
	if (s == 4) {
		printf("\t%s 2,s\n", op);
		printf("\t%s 1,s\n", op2);
		printf("\t%s ,s\n", op2);
		printf("\tleas 4,s\n");
		return 1;
	} else if (s == 2)
		op16d_on_tos(op);
	else
		op8_on_tos(op);
	invalidate_work();
	return 1;
}

void uniop8_on_tos(const char *op)
{
	unsigned off = make_tos_ptr();
	invalidate_work();
	printf("\t%s %u,x\n", op, off);
	printf("\tins\n");
}

void uniop16_on_tos(const char *op)
{
	unsigned off = make_tos_ptr();
	invalidate_work();
	printf("\t%s %u,x\n", op, off + 1);
	printf("\t%s %u,x\n", op, off);
	printf("\tins\n");
	printf("\tins\n");
}

unsigned write_tos_uniop(struct node *n, const char *op)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	if (s == 2)
		printf("\t%s ,-s\n", op);
	printf("\t%s ,-s\n", op);
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

/* See if we can easily get the value we want into X/Y/U rather than D. Must
   not harm D in the process. We can make this smarter over time if needed.
   Might be worth passing if we can trash D as it will help make_local_ptr
   later, and will be true for some load cases */
unsigned can_load_r_simple(struct node *r, unsigned off)
{
	switch(r->op) {
	case T_ARGUMENT:
	case T_LOCAL:
	case T_LREF:
	case T_CONSTANT:
	case T_LBREF:
	case T_NREF:
	case T_NAME:
	case T_LABEL:
	case T_RREF:
	case T_RDEREF:
		return 1;
	}
	return 0;
}

/* Also allow offset in the result and some level of complexity via
   lea for offsets on things like struct */
unsigned can_load_r_with(struct node *r, unsigned off)
{
	switch(r->op) {
	case T_ARGUMENT:
	case T_LOCAL:
	case T_LREF:
	case T_CONSTANT:
	case T_LBREF:
	case T_NREF:
	case T_NAME:
	case T_LABEL:
	case T_RREF:
	case T_RDEREF:
		return 1;
	case T_PLUS:
	case T_MINUS:
		if (!can_load_r_with(r->left, off))
			return 0;
		if (r->right->op != T_CONSTANT)
			return 0;
		return 1;
	}
	return 0;
}

/* For 6800 at least it is usually cheaper to reload even if the value
   we want is in D */
static unsigned load_r_with(char reg, struct node *r, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		/* TODO: will need a Y specific rule for HC11 */
		printf("\tlea%c %u,s\n", reg, v + sp);
		break;
	case T_LREF:
		printf("\tld%c %u,s\n", reg, v + sp);
		break;
	case T_CONSTANT:
	case T_LBREF:
	case T_LABEL:
	case T_NREF:
	case T_NAME:
	case T_RDEREF:
		printf("\tld%c %s\n", reg, addr_form(r, off, 2));
		break;
	case T_RREF:
		if (reg != 'u')
			printf("\ttfr u,%c\n", reg);
		break;
	case T_PLUS:
		/* Special case array/struct */
		if (cpu_is_09 && can_load_r_simple(r->left, off) &&
			r->right->op == T_CONSTANT) {
			load_r_with(reg, r->left, off);
			return r->right->value;
		}
		break;
	case T_MINUS:
		if (cpu_is_09 && can_load_r_simple(r->left, off) &&
			r->right->op == T_CONSTANT) {
			load_r_with(reg, r->right, off);
			return -r->right->value;
		}
		break;
	default:
		error("lxw");
	}
	return 0;
}

unsigned load_x_with(struct node *r, unsigned off)
{
	unsigned rv = load_r_with('x', r, off);
	invalidate_x();
	return rv;
}

/* 6809 specific register loading */
unsigned load_u_with(struct node *r, unsigned off)
{
	return load_r_with('u', r, off);
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
		printf("\tcmpb #%u\n", v & 0xFF);
		printf("\t%s %s\n", jsr_op, op);
		n->flags |= ISBOOL;
		invalidate_b();
		return 1;
	}
	if (s == 2) {
		printf("\tsubd #%u\n", v & 0xFFFF);
		printf("\t%s %s\n", jsr_op, op);
		n->flags |= ISBOOL;
		invalidate_work();
		return 1;
	}
	return 0;
}

/*
 *	Do fast multiplies were we can
 */
int count_mul_cost(unsigned n)
{
	int cost = 0;
	if ((n & 0xFF) == 0) {
		n >>= 8;
		cost += 3;		/* tfr a,b clrb */
	}
	while(n > 1) {
		if (n & 1)
			cost += 4;	/* std s++, addd ,--s */
		n >>= 1;
		cost += 2;		/* lslb rola */
	}
	return cost;
}

/* We can probably do better on 6809 TODO */
/* Write the multiply for any value > 0 */
void write_mul(unsigned n)
{
	unsigned pops = 0;
	if ((n & 0xFF) == 0) {
		load_a_b();
		load_b_const(0);
		n >>= 8;
	}
	while(n > 1) {
		if (n & 1) {
			pops++;
			printf("\tpshs d\n");
		}
		printf("\tlslb\n\trola\n");
		n >>= 1;
	}
	while(pops--) {
		printf("\taddd ,s++\n");
	}
}

unsigned can_fast_mul(unsigned s, unsigned n)
{
	/* Pulled out of my hat 8) */
	unsigned cost = 15 + 3 * opt;
	if (s > 2)
		return 0;

	/* For the moment */
	if (!cpu_is_09)
		return 0;

	/* The base cost of a helper is 8 */
	if (optsize)
		cost = 8;
	if (n == 0 || count_mul_cost(n) <= cost)
		return 1;
	return 0;
}

void gen_fast_mul(unsigned s, unsigned n)
{

	if (n == 0)
		load_d_const(0);
	else {
		write_mul(n);
		invalidate_work();
	}
}

unsigned gen_fast_div(unsigned n, unsigned s, unsigned u)
{
	u &= UNSIGNED;
	if (s != 2)
		return 0;
	if (n == 1)
		return 1;
	if (n == 256 && u) {
		load_b_a();
		load_a_const(0);
		return 1;
	}
	if (n & (n - 1))
		return 0;
	if (u) {
		while(n > 1) {
			printf("\tlsra\n\trorb\n");
			n >>= 1;
		}
	} else {
		while(n > 1) {
			printf("\tasra\n\trorb\n");
			n >>= 1;
		}
	}
	invalidate_work();
	return 1;
}

void op_on_ptr(struct node *n, const char *op, unsigned off)
{
	unsigned s = get_size(n->type);
	if (s == 1)
		op8_on_ptr(op, off);
	else if (s == 2)
		op16_on_ptr(op, op, off);
	else
		op32_on_ptr(op,op, off);
}

void opd_on_ptr(struct node *n, const char *op, const char *op2, unsigned off)
{
	unsigned s = get_size(n->type);
	if (s == 1)
		op8_on_ptr(op, off);
	else if (s == 2)
		op16d_on_ptr(op, op2, off);
	else
		op32_on_ptr(op, op2, off);
}

/* TODO; compare and flip the boolify test rather than go via stack
   when we can */
unsigned cmp_op(struct node *n, const char *uop, const char *op)
{
	unsigned s = get_size(n->right->type);
	if (n->right->type & UNSIGNED)
		op = uop;
	if (s > 2)	/* For now anyway */
		return 0;
	/* We can do this versus s+ or s++ */
	/* FIXME: 6809 has cmpd unlike 6803 */
	if (s == 1)
		op8_on_tos("cmp");
	else if (s == 2)
		op16d_on_tos("cmp");
	printf("\t%s %s\n", jsr_op, op);
	n->flags |= ISBOOL;
	invalidate_work();
	return 1;
}

unsigned gen_push(struct node *n)
{
	unsigned size = get_size(n->type);
	/* Our push will put the object on the stack, so account for it */
	sp += get_stack_size(n->type);
	switch(size) {
	case 1:
		printf("\tpshs b\n");
		return 1;
	case 2:
		printf("\tpshs d\n");
		return 1;
	case 4:	/* Have to split them to get the order right */
		/* Or we could go PDP11 style mixed endian long ? */
		printf("\tpshs d\n");
		printf("\tpshs y\n");
		return 1;
	}
	return 0;
}
