#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"
#include "backend-6800.h"

/* 16bit constant load */
void load_d_const(uint16_t n)
{
	unsigned hi,lo;

	lo = n & 0xFF;
	hi = n >> 8;

/*	printf(";want %04X have %02X:%02X val %d %d\n",
		n, a_val, b_val, a_valid, b_valid); */

	if (cpu_has_d) {
		if (n == 0) {
			if (!a_valid || a_val)
				printf("\tclra\n");
			if (!b_valid || b_val)
				printf("\tclrb\n");
		} else if (!a_valid || !b_valid || a_val != hi || b_val != lo) {
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
				if (cpu_is_09)
					printf("\ttfr b,a\n");
				else
					printf("\ttba\n");
				printf("\t%sa #%d\n", ld8_op, hi);
			}
		}
		if (b_valid == 0 || lo != b_val) {
			if (lo == 0)
				printf("\tclrb\n");
			else if (lo == hi) {
				if (cpu_is_09)
					printf("\ttfr a,b\n");
				else
					printf("\ttab\n");
			}
			return;
		} else
			printf("\t%sb #%d\n", ld8_op, lo);
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
	else if (a_valid && n == a_val) {
		if (cpu_is_09)
			printf("\ttfr a,b\n");
		else
			printf("\ttab\n");
	} else
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

	if (cpu_has_d)
		printf("\taddd #%u\n", n);
	else {
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
	if (cpu_is_09)
		printf("\ttfr b,a\n");
	else
		printf("\ttba\n");
	a_val = b_val;
	a_valid = b_valid;
	d_valid = 0;
}

void load_b_a(void)
{
	if (cpu_is_09)
		printf("\ttfr a,b\n");
	else
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

void swap_d_x(void)
{
	if (cpu_is_09)
		printf("\texg d,x\n");
	else
		printf("\txgdx\n");
	invalidate_work();
	invalidate_x();
}

/* Get D into X (may trash D) */
void make_x_d(void)
{
	if (cpu_is_09)
		printf("\ttfr d,x\n");
	else if (cpu_has_xgdx) {
		/* Should really track on the exchange later */
		invalidate_work();
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
		printf("\tpuls x\n");
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
	if (cpu_has_lea) {
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
		if (n > 14 || n < -14) {
			printf("\ttsx\n\txgdx\n\taddd #%u\n\txgdx\n\ttxs\n", WORD(n));
			x_fprel = 1;
			x_fpoff = 0;
			return;
		}
		invalidate_x();
		if (n > 0) {
			/* Otherwise we know pulx is cheapest */
			repeated_op(n / 2, "pulx");
			if (n & 1)
				printf("\tins\n");
		}
		if (n < 0) {
			/* pshx likewise is an option */
			repeated_op(-n / 2, "pshx");
			if (n & 1)
				printf("\tdes\n");
		}
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
		/* FIXME: need top put S into X and back.. */
		if (save_d)
			printf("\tpshb\n");
		printf("\ttsx\n");
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
		printf("\ttsx\n");
		if (save_d)
			printf("\tpulb\n");
		x_fprel = 1;
		x_fpoff = 0;
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
	/* TODO: if we save_d we need to keep and valid */
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
	printf("\t%sb %u,x\n", remap_op(op), off);
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

/* Do the low byte first in case it's add adc etc */
void op16_on_spi(const char *op, const char *op2)
{
	printf("\t%sa ,s+\n", op);
	printf("\t%sb ,s+\n", op2);
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
		/* FIXME: use D */
		printf("\tpshb\n\tpsha");
		printf("\tldd @hireg\n");
		printf("\t%sb %u,x\n", op2, off + 1);
		printf("\t%sa %u,x\n", op2, off);
		printf("\tstd @hireg\n");
		printf("\tpula\n\tpulb\n");
	}
}

void uniop_on_ptr(register const char *op, register unsigned off,
						register unsigned size)
{
	op = remap_op(op);
	off += size;
	while(size--)
		printf("\t%s %u,x\n", op, --off);
}

/* TODO: propogate down if we need to save B */
unsigned make_local_ptr(unsigned off, unsigned rlim)
{
	/* Both relative to frame base */
	int noff = off - x_fpoff;

	/* Although we can access arguments via S we sometimes still need
	   this path to make pointers to locals. We do need to go through
	   the cases we can just use ,s to make sure we avoid two steps */
	if (cpu_has_lea) {
		printf("\tleax %u,s\n", off + sp);
		return 0;
	}

	printf(";make local ptr off %u, rlim %u noff %u\n", off, rlim, noff);

	/* TODO: if we can d a small < 7 or so shift by decrement then
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
		sprintf(addr, "%s_%s+%u%s", mod, namestr(r->snum), v + off, pic_op);
		return addr;
	case T_LABEL:
		sprintf(addr, "#%sT%u+%u", mod, r->val2, v + off);
		return addr;
	case T_LBSTORE:
		sprintf(addr, "%sT%u+%u%s", mod, r->val2, v + off, pic_op);
		return addr;
	case T_LBREF:
		sprintf(addr, "%sT%u+%u%s", mod, r->val2, v + off, pic_op);
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
		if (cpu_is_09)
			op16d_on_s(op, op2, v + off + sp);
		else {
			off = make_local_ptr(v + off, 254);
			op16d_on_ptr(op, op2, off);
		}
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
		if (cpu_is_09)
			printf("\t%sy %u,s\n", op, v + off + sp);
		else {
			off = make_local_ptr(v + off, 254);
			printf("\t%sy %u,x\n", op, off);
		}
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
	if (s == 2) {
		if (!cpu_has_d)
			return op16_on_node(r, op, op2, off);
		return op16d_on_node(r, op, op2, off);
	}
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

	op = remap_op(op);
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		if (cpu_is_09) {
			if (s == 2)
				printf("\t%s %u,s\n", op, v + off + 1);
			printf("\t%s %u,s\n", op, v + off);
		} else {
			off = make_local_ptr(v + off, 254);
			uniop_on_ptr(op, off, 2);
		}
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
	invalidate_work();
	if (cpu_is_09)
		op16_on_spi(op, op2);
	else {
		off = make_tos_ptr();
		printf("\t%sb %u,x\n", remap_op(op), off + 1);
		printf("\t%sa %u,x\n", remap_op(op2), off);
		printf("\tins\n");
		printf("\tins\n");
	}
}

void op16d_on_tos(const char *op, const char *op2)
{
	unsigned off;
	invalidate_work();
	if (cpu_is_09)
		op16d_on_spi(op);
	else {
		off = make_tos_ptr();
		if (cpu_has_d)
			printf("\t%sd %u,x\n", op, off);
		else {
			printf("\t%sb %u,x\n", remap_op(op), off + 1);
			printf("\t%sa %u,x\n", remap_op(op2), off);
		}
		printf("\tins\n");
		printf("\tins\n");
	}
}

/* TODO: this seems to be buggy for 32bit */
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
unsigned can_load_x_simple(struct node *r, unsigned off)
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
unsigned can_load_x_with(struct node *r, unsigned off)
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
		if (!cpu_is_09)
			return 0;
		if (!can_load_x_with(r->left, off))
			return 0;
		if (r->right->op != T_CONSTANT)
			return 0;
		return 1;
	}
	return 0;
}

/* For 6800 at least it is usually cheaper to reload even if the value
   we want is in D */
unsigned load_x_with(struct node *r, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		/* Worst case for size is 252 */
		return make_local_ptr(v + off, 252);
	case T_LREF:
		if (cpu_is_09)
			printf("\tldx %u,s\n", v + sp);
		else {
			off = make_local_ptr(v + off, 252);
			printf("\tldx %u,x\n", off);
		}
		invalidate_x();
		break;
	case T_CONSTANT:
		printf("\tldx #%u\n", v + off);
		invalidate_x();
		break;
	case T_LBREF:
		printf("\tldx T%u+%u%s\n", r->val2, v + off, pic_op);
		invalidate_x();
		break;
	case T_LABEL:
		if (cpu_pic)
			printf("\tleax T%u+%u,pcr\n", r->val2, v + off);
		else
			printf("\tldx #T%u+%u\n", r->val2, v + off);
		invalidate_x();
		break;
	case T_NREF:
		printf("\tldx _%s+%u%s\n", namestr(r->snum), v + off, pic_op);
		invalidate_x();
		break;
	case T_NAME:
		if (cpu_pic)
			printf("\tleax _%s+%u,pcr\n", namestr(r->snum), v + off);
		else
			printf("\tldx #_%s+%u\n", namestr(r->snum), v + off);
		invalidate_x();
		break;
	case T_RREF:
		printf("\ttfr u,x\n");
		invalidate_x();
		break;
	case T_RDEREF:
		printf("\tldx %u,u\n", r->val2);
		invalidate_x();
		break;
	case T_PLUS:
		/* Special case array/struct */
		if (cpu_is_09 && can_load_x_simple(r->left, off) &&
			r->right->op == T_CONSTANT) {
			load_x_with(r->left, off);
			return r->right->value;
		}
		break;
	case T_MINUS:
		if (cpu_is_09 && can_load_x_simple(r->left, off) &&
			r->right->op == T_CONSTANT) {
			load_x_with(r->right, off);
			return -r->right->value;
		}
		break;
	/* case T_RREF:
		printf("\tldx @__reg%u\n", v);
		break; */
	default:
		error("lxw");
	}
	return 0;
}

/* 6809 specific register loading */
unsigned load_u_with(struct node *r, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		printf("leau %u,s\n", v + off);
		return 1;
	case T_LREF:
		printf("\tldu %u,s\n", v + sp);
		return 1;
	case T_CONSTANT:
		printf("\tldu #%u\n", v + off);
		return 1;
	case T_LBREF:
		printf("\tldu T%u+%u%s\n", r->val2, v + off, pic_op);
		return 1;
	case T_LABEL:
		if (cpu_pic)
			printf("\tleau T%u+%u,pcr\n", r->val2, v + off);
		else
			printf("\tldu #T%u+%u\n", r->val2, v + off);
		return 1;
	case T_NREF:
		printf("\tldu _%s+%u%s\n", namestr(r->snum), v + off, pic_op);
		return 1;
	case T_NAME:
		if (cpu_pic)
			printf("\tleau _%s+%u,pcr\n", namestr(r->snum), v + off);
		else
			printf("\tldu #_%s+%u\n", namestr(r->snum), v + off);
		return 1;
	case T_RREF:
		/* Only one reg so .. */
		return 1;
	case T_RDEREF:
		printf("\tldu %u,u\n", r->val2);
		return 1;
	}
	return 0;
}
