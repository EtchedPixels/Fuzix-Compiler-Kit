#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"
#include "backend-6800.h"

unsigned label;		/* Used to hand out local labels of the form X%u */

/*
 *	Fix up weirdness in the asm formats.
 */

/* 16bit constant load */
void load_d_const(uint16_t n)
{
	register unsigned hi,lo;

	lo = n & 0xFF;
	hi = n >> 8;

/*	printf(";want %04X have %02X:%02X val %d %d\n",
		n, a_val, b_val, a_valid, b_valid); */

	if (cpu_has_d) {
		if (n == 0) {
			if (!a_valid || a_val)
				puts("\tclra");
			if (!b_valid || b_val)
				puts("\tclrb");
		} else if (!a_valid || !b_valid || a_val != hi || b_val != lo) {
			printf("\tldd #%u\n", n);
		}
	} else {
		/* TODO: track AB here and see if we can use existing values */
		lo = n & 0xFF;
		hi = n >> 8;
		if (a_valid == 0 || hi != a_val) {
			if (hi == 0)
				puts("\tclra");
			else if (b_valid && hi == b_val)
				puts("\ttba");
			else
				printf("\tldaa #%d\n", hi);
		}
		if (b_valid == 0 || lo != b_val) {
			if (lo == 0)
				puts("\tclrb");
			else if (lo == hi)
				puts("\ttab");
			else
				printf("\tldab #%d\n", lo);
		}
	}
	a_valid = 1;	/* We know the byte values */
	b_valid = 1;
	d_valid = 0;	/* No longer an object reference */
	a_val = hi;
	b_val = lo;

}

void load_a_const(register uint8_t n)
{
	if (a_valid && n == a_val)
		return;
	if (n == 0)
		puts("\tclra");
	else if (b_valid && n == b_val)
		puts("\ttba");
	else
		printf("\tldaa #%u\n", n & 0xFF);
	a_valid = 1;
	a_val = n;
	d_valid = 0;
}

void load_b_const(register uint8_t n)
{
	if (b_valid && n == b_val)
		return;
	if (n == 0)
		puts("\tclrb");
	else if (a_valid && n == a_val)
		puts("\ttab");
	else
		printf("\tldab #%u\n", n & 0xFF);
	b_valid = 1;
	b_val = n;
	d_valid = 0;
}

void add_d_const(register uint16_t n)
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

void add_b_const(register uint8_t n)
{
	if (n == 0)
		return;
	printf("\taddb #%u\n", n & 0xFF);
	b_val += n;
	d_valid = 0;

}

void load_a_b(void)
{
	puts("\ttba");
	a_val = b_val;
	a_valid = b_valid;
	d_valid = 0;
}

void load_b_a(void)
{
	puts("\ttab");
	b_val = a_val;
	b_valid = a_valid;
	d_valid = 0;
}

void move_s_d(void)
{
	puts("\tsts @tmp");
	if (cpu_has_d)
		puts("\tldd @tmp");
	else
		puts("\tldaa @tmp\n\tldab @tmp+1");
	invalidate_work();
}

void move_d_s(void)
{
	if (cpu_has_d)
		puts("\tstd @tmp");
	else
		puts("\tstaa @tmp\n\tstab @tmp+1");
	puts("\tlds @tmp");
}

void swap_d_y(void)
{
	puts("\txgdy");
}

void swap_d_x(void)
{
	/* Should really track on the exchange later */
	puts("\txgdx");
	invalidate_work();
	invalidate_x();
}

/* Get D into X (may trash D) */
void make_x_d(void)
{
	if (cpu_has_xgdx)
		swap_d_x();
	else {
		if (cpu_has_d)
			puts("\tstd @tmp\n\tldx @tmp");
		else
			puts("\tstaa @tmp\n\tstab @tmp+1\n\tldx @tmp");
	}
	/* TODO: d -> x see if we know d type */
	invalidate_x();
}

/* Get X into D (may trash X) */
void make_d_x(void)
{
	if (cpu_has_xgdx)
		swap_d_x();
	else {
		if (cpu_has_d)
			puts("\tstx @tmp\n\tldd @tmp");
		else
			puts("\tstx @tmp\n\tldaa @tmp\n\tldab @tmp+1");
	}
	/* TODO: x->d  see if we know x type */
	invalidate_work();
}

void pop_x(void)
{
	/* Must remember this trashes X, or could make it smart
	   when we track and use offsets of current X then ins ins */
	/* Easier said than done on a 6800 */
	if (cpu_has_pshx)
		puts("\tpulx");
	else
		puts("\ttsx\n\tldx ,x\n\tins\n\tins");
	invalidate_x();
}

/*
 *	There are multiple strategies depnding on chip features
 *	available.
 */
void adjust_s(register int n, unsigned save_d)
{
	register unsigned abxcost = 3 + 2 * save_d +  n / 255;
	register unsigned hardcost;
	unsigned cost;

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
			x_fpoff = sp;
			return;
		}
		invalidate_x();
		if (n > 0) {
			/* Otherwise we know pulx is cheapest */
			repeated_op(n / 2, "pulx");
			if (n & 1)
				puts("\tins");
		}
		if (n < 0) {
			/* pshx likewise is an option */
			repeated_op(-n / 2, "pshx");
			if (n & 1)
				puts("\tdes");
		}
		return;
	}

	if (cpu_has_abx && abxcost < hardcost)
		cost = abxcost;
	/* PULX might be fastest */
	if (n > 0 && cpu_has_pshx && (n / 2) + (n & 1) <= cost) {
		invalidate_x();
		repeated_op(n / 2, "pulx");
		if (n & 1)
			puts("\tins");
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
		puts("\ttsx");
		if (save_d)
			puts("\tpshb");
		if(n > 255) {
			load_b_const(255);
			while(n >= 255) {
				puts("\tabx");
				n -= 255;
			}
		}
		if (n) {
			load_b_const(n);
			puts("\tabx");
		}
		if (save_d)
			puts("\tpulb");
		puts("\ttxs");
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
			puts("\tdes");
		return;
	}
	if (n < 0 && n >= -4) {
		repeated_op(-n, "des");
		return;
	}
	if (optsize) {
		if (n > 0 && n <= 255) {
			printf("\tjsr __addsp8\n\t.byte %u\n", n & 0xFF);
			return;
		}
		if (n <0 && n >= -255) {
			printf("\tjsr __subsp8\n\t.byte %u\n", -n & 0xFF);
			return;
		}
		printf("\tjsr __modsp16\n\t.word %u\n", WORD(n));
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
		puts("\tstaa @tmp2\n\tstab @tmp2+1");

	move_s_d();
	add_d_const(n);
	move_d_s();
	if (save_d)
		puts("\tldaa @tmp2\n\tldab @tmp2+1");
}

void op8_on_ptr(const char *op, unsigned off)
{
	printf("\t%sb %u,x\n", op, off);
}

/* Do the low byte first in case it's add adc etc */
void op16_on_ptr(const char *op, const char *op2, unsigned off)
{
	/* Big endian */
	printf("\t%sb %u,x\n\t%sa %u,x\n", op, off + 1, op2, off);
}

/* Operations where D can be used on later processors */
void op16d_on_ptr(const char *op, const char *op2, unsigned off)
{
	/* Big endian */
	if (cpu_has_d)
		printf("\t%sd %u,x\n", op, off);
	else
		printf("\t%sb %u,x\n\t%sa %u,x\n", op, off + 1, op2, off);
}

static void op32_on_ptr(const char *op, const char *op2, unsigned off)
{
	printf("\t%sb %u,x\n\t%sa %u,x\n", op, off + 3, op2, off + 2);
	if (cpu_has_y) {
		swap_d_y();
		printf("\t%sb %u,x\n\t%sa %u,x\n", op2, off + 1, op2, off);
		swap_d_y();
	} else {
		puts("\tpshb\n\tpsha\n\tldaa @hireg\n\tldab @hireg+1");
		printf("\t%sb %u,x\n\t%sa %u,x\n", op2, off + 1, op2, off);
		puts("\tstaa @hireg\n\tstab @hireg+1\n\tpula\n\tpulb");
	}
}

void op32d_on_ptr(const char *op, const char *op2, unsigned off)
{
	if (!cpu_has_d) {
		op32_on_ptr(op, op2, off);
		return;
	}
	printf("\t%sd %u,x\n", op, off + 2);
	if (cpu_has_y) {
		swap_d_y();
		printf("\t%sb %u,x\n\t%sa %u,x\n", op2, off + 1, op2, off);
		swap_d_y();
	} else {
		puts("\tpshb\n\tpsha\n\tldd @hireg\n");
		printf("\t%sb %u,x\n\t%sa %u,x\n", op2, off + 1, op2, off);
		puts("\tstd @hireg\n\tpula\n\tpulb");
	}
}

void load32(register unsigned off)
{
	if (cpu_has_y) {
		printf("\tldd %u,x\n", off);
		swap_d_y();
		printf("\tldd %u,x\n", off + 2);
	} else if (cpu_has_d)
		printf("\tldd %u,x\n\tstd @hireg\n\tldd %u,x\n", off, off + 2);
	else {
		printf("\tldaa %u,x\n\tldab %u,x\n\tldx %u,x\n\tstx @hireg\n",
			off + 2, off + 3, off);
		invalidate_x();
	}
}

void store32(register unsigned off, unsigned nr)
{
	if (cpu_has_y)
		printf("\tsty %u,x\n\tstd %u,x\n", off, off + 2);
	else if (cpu_has_d) {
		printf("\tstd %u,x\n\tldd @hireg\n\tstd %u,x\n", off + 2, off);
		if (nr)
			printf("\tldd %u,x\n", off + 2);
	} else {
		printf("\tstab %u,x\n\tstaa %u,x\n\tpsha\n", off + 3, off + 2);
		printf("\tldaa @hireg+1\n\tstaa %u,x\n\tldaa @hireg\n\tstaa %u,x\n\tpula\n", off + 1, off);
	}
}

void uniop_on_ptr(register const char *op, register unsigned off,
						register unsigned size)
{
	off += size;
	while(size--)
		printf("\t%s %u,x\n", op, --off);
}

/*
 *	Generate a reference via X to a local
 */
unsigned make_local_ptr(unsigned off, unsigned rlim)
{
	/* Both relative to frame base */
	int noff = off - x_fpoff;

	printf(";make local ptr off %u, rlim %u noff %u\n", off, rlim, noff);

	/* TODO: if we can d a small < 7 or so shift by decrement then
	   it may beat going via tsx */
	if (x_fprel == 0 ||  noff < 0) {
		printf("\ttsx\n");
		x_fprel = 1;
		x_fpoff = sp;
	}
	off += x_fpoff;
	if (off <= rlim)
		return off;

	/* It is cheaper to inx than mess around with calls for smaller
	   values - 7 or 5 if no save needed */
	if (off - rlim < 7) {
		repeated_op(off - rlim, "inx");
		x_fpoff -= off - rlim;
		return rlim;
	}
	/* These cases push and pop the old D but we don't have a tracking
	   mechanism to optimise it. FIXME one day. The first half of things
	   is fine as we push and load so can optimize, but the resulting
	   case we can currently only invalidate for */
	if (off - rlim < 256) {
		puts("\tpshb");
		load_b_const(off - rlim);
		if (cpu_has_d)	/* And thus ABX */
			puts("\tabx");
		else
			puts("\tjsr __abx");
		puts("\tpulb");
		x_fpoff -= off - rlim;
		invalidate_work();
		return rlim;
	} else {
		/* This case is (thankfully) fairly rare */
		puts("\tpshb\n\tpsha");
		load_d_const(off);
		puts("\tjsr __adx\n\tpula\n\tpulb");
		x_fpoff -= off;
		invalidate_work();
		return 0;
	}
}

/* Get pointer to the top of stack. We can optimize this in some cases
   when we track but it will be limited. The 6800 is quite weak on ops
   between register so we sometimes need to build ops against top of stack.
   
   This needs some care because the stack pointer has already been adjusted
   in codegen-6800 to allow for the removal of the data, thus we actually
   need to offset by the passed size */
static unsigned make_tos_ptr(unsigned s)
{
	printf(";make_tos_ptr %u %u, %u\n", x_fpoff, sp + s, x_fprel);
	/* TODO: we might have X pointing below the stack but in range and that would be just fine */
	if (x_fpoff == sp + s && x_fprel == 1)
		return 0;
	puts("\ttsx");
	x_fpoff = sp + s;
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
	else if (s == 3)
		mod = ">";

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
	default:
		error("aform");
	}
	return NULL;
}

/* Those with address forms we can directly load */
unsigned can_load_d_nox(struct node *n, unsigned off)
{
	register unsigned op = n->op;
	if (op == T_CONSTANT || op == T_NAME || op == T_LABEL || op == T_NREF || op == T_LBREF)
		return 1;
	return 0;
}

/* These functions must not touch X on the 6809, they can on others */
unsigned op8_on_node(struct node *r, const char *op, unsigned off)
{
	unsigned v = r->value;

	invalidate_work();

	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		off = make_local_ptr(v + off, 255);
		op8_on_ptr(op, off);
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
unsigned op16_on_node(register struct node *r, const char *op, const char *op2, unsigned off)
{
	register unsigned v = r->value;

	invalidate_work();

	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		off = make_local_ptr(v + off, 254);
		op16_on_ptr(op, op2, off);
		break;
	case T_CONSTANT:
		printf("\t%sb #<%u\n", op, (v + off) & 0xFFFF);
		printf("\t%sa #>%u\n", op2, (v + off) & 0xFFFF);
		break;
	case T_LBSTORE:
	case T_LBREF:
	case T_NSTORE:
	case T_NREF:
		printf("\t%sb %s\n", op, addr_form(r, off + 1, 1));
		printf("\t%sa %s\n", op2, addr_form(r, off, 1));
		break;
	case T_NAME:
	case T_LABEL:
		printf("\t%sb %s\n", op, addr_form(r, off, 1));
		printf("\t%sa %s\n", op2, addr_form(r, off, 3));
		break;
	default:
		return 0;
	}
	return 1;
}

unsigned op16d_on_node(register struct node *r, const char *op, const char *op2, unsigned off)
{
	unsigned v = r->value;

	invalidate_work();
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		off = make_local_ptr(v + off, 254);
		op16d_on_ptr(op, op2, off);
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

unsigned op16y_on_node(register struct node *r, const char *op, unsigned off)
{
	unsigned v = r->value;
	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		off = make_local_ptr(v + off, 254);
		printf("\t%sy %u,x\n", op, off);
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

unsigned write_op(register struct node *r, const char *op, const char *op2, unsigned off)
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

	switch(r->op) {
	case T_LSTORE:
	case T_LREF:
		off = make_local_ptr(v + off, 254);
		uniop_on_ptr(op, off, s);
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

static void op8_on_tos(const char *op)
{
	unsigned off = make_tos_ptr(1);
	printf("\t%sb %u,x\n\tins\n", op, off);
}

static void op16_on_tos(const char *op)
{
	register unsigned off;
	invalidate_work();
	off = make_tos_ptr(2);
	printf("\t%sb %u,x\n\t%sa %u,x\n\tins\n\tins\n", op, off + 1, op, off);
}

static void op16d_on_tos(const char *op)
{
	unsigned off;
	invalidate_work();
	off = make_tos_ptr(2);
	printf("\t%sd %u,x\n\tins\n\tins\n", op, off);
}

/* Only used for operations where there is no ordering requirement */
unsigned write_tos_op(struct node *n, register const char *op)
{
	register unsigned s = get_size(n->type);
	if (s > 2 && !cpu_has_y)
		return 0;
	if (s == 4) {
		swap_d_y();
		/* So that the second 16bit op is correctly offset */
		sp += 2;
		op16_on_tos(op);
		sp -= 2;
		swap_d_y();
		op16_on_tos(op);
	} else if (s == 2)
		op16_on_tos(op);
	else
		op8_on_tos(op);
	invalidate_work();
	return 1;
}

unsigned write_tos_opd(struct node *n, const char *op, const char *unused)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	else if (s == 2)
		op16d_on_tos(op);
	else
		op8_on_tos(op);
	invalidate_work();
	return 1;
}

static void uniop8_on_tos(const char *op)
{
	unsigned off = make_tos_ptr(1);
	invalidate_work();
	printf("\t%s %u,x\n\tins\n", op, off);
}

static void uniop16_on_tos(register const char *op)
{
	register unsigned off = make_tos_ptr(2);
	invalidate_work();
	printf("\t%s %u,x\n\t%s %u,x\n\tins\n\tins\n", op, off + 1, op, off);
}

unsigned write_tos_uniop(struct node *n, register const char *op)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	if (s == 2)
		uniop16_on_tos(op);
	else
		uniop8_on_tos(op);
	return 1;
}

/* TODO: decide how much we inline for -Os */

unsigned left_shift(register struct node *n)
{
	register unsigned s = get_size(n->type);
	register unsigned v;

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
			puts("\tlslb\n\trola");
		invalidate_work();
		return 1;
	}
	return 0;
}

unsigned right_shift(register struct node *n)
{
	register unsigned s = get_size(n->type);
	register unsigned v;
	register const char *op = "asr";

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
	unsigned s = get_size(r->type);
	if (s == 4)
		return 0;
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
	unsigned s = get_size(r->type);
	if (s == 4)
		return 0;
	switch(r->op) {
	case T_ARGUMENT:
	case T_LOCAL:
	case T_LREF:
	case T_CONSTANT:
	case T_LBREF:
	case T_NREF:
	case T_NAME:
	case T_LABEL:
		return 1;
	}
	return 0;
}

/* For 6800 at least it is usually cheaper to reload even if the value
   we want is in D */
static unsigned load_r_with(char reg, register struct node *r, unsigned off)
{
	register unsigned v = r->value;
	switch(r->op) {
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		/* TODO: will need a Y specific rule for HC11 */
		/* Worst case for size is 252 */
		return make_local_ptr(v + off, 252);
	case T_LREF:
		off = make_local_ptr(v + off, 252);
		printf("\tld%c %u,x\n", reg, off);
		if(reg == 'x')
			invalidate_x();
		break;
	case T_CONSTANT:
	case T_LBREF:
	case T_LABEL:
	case T_NREF:
	case T_NAME:
		printf("\tld%c %s\n", reg, addr_form(r, off, 2));
		if(reg == 'x')
			invalidate_x();
		break;
	default:
		error("lxw");
	}
	return 0;
}

unsigned load_x_with(struct node *r, unsigned off)
{
	return load_r_with('x', r, off);
}

/* 6809 specific register loading */
unsigned load_u_with(struct node *r, unsigned off)
{
	return load_r_with('u', r, off);
}

unsigned cmp_direct(struct node *n, const char *uop, const char *op)
{
	register struct node *r = n->right;
	register unsigned v = r->value;
	register unsigned s = get_size(r->type);

	if (r->op != T_CONSTANT)
		return 0;
	if (r->type & UNSIGNED)
		op = uop;

	/* If we knoww the upper half matches - eg a byte var that has been expanded then
	   shortcut it. Need to think about this for signed math. I think we can do signed
	   math if we use uop in the size 2 upper match case : TODO*/
	if (s == 1 || (op == uop && a_valid && (v >> 8) == a_val)) {
		printf("\tcmpb #%u\n\t%s %s\n", v & 0xFF, jsr_op, op);
		n->flags |= ISBOOL;
		invalidate_b();
		return 1;
	}
	if (s == 2) {
		/* If we know the value of A (eg from a cast) */
		if (cpu_has_d) {
			printf("\tsubd #%u\n\t%s %s\n", v & 0xFFFF, jsr_op, op);
			n->flags |= ISBOOL;
			invalidate_work();
			return 1;
		}
	}
	return 0;
}

/*
 *	Do fast multiplies were we can
 *
 *	TODO: do the other powers of two at least for -O2 and higher
 */

unsigned can_fast_mul(unsigned s, unsigned n)
{
	/* For now */
	/* TODO: at least do powers of 2.. might be worth doing
	   jsr to a helper for 3-15 too */
	if (n <= 2 || n == 4)
		return 1;
	return 0;
}

void gen_fast_mul(unsigned s, unsigned n)
{
	if (n == 0)
		load_d_const(0);
	else if (n == 2)
		puts("\tlslb\n\trola");
	else if (n == 4)
		puts("\tlslb\n\trola\n\tlslb\n\trola");
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
			puts("\tlsra\n\trorb");
			n >>= 1;
		}
	} else {
		/* Round towards zero */
		unsigned m = (n - 1) & 0xFFFF;
		printf("\ttsta\n\tbpl X%u\n", ++label);
		if (cpu_has_d)
			printf("\taddd #%u\n", m);
		else {
			printf("\taddb #%u\n\tadca #%u\n",
				m  & 0xFF, m >> 8);
		}
		printf("X%u:\n", label);
		while(n > 1) {
			puts("\tasra\n\trorb");
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
unsigned cmp_op(register struct node *n, const char *uop, const char *op)
{
	unsigned s = get_size(n->right->type);
	if (n->right->type & UNSIGNED)
		op = uop;
	if (s == 1) {
		op8_on_tos("cmp");
		printf("\t%s %s\n", jsr_op, op);
		n->flags |= ISBOOL;
		invalidate_work();
		return 1;
	}
	if (s == 2 && cpu_has_d) {
		op16d_on_tos("sub");
		printf("\t%s %s\n", jsr_op, op);
		n->flags |= ISBOOL;
		invalidate_work();
		return 1;
	}
	return 0;
}

unsigned gen_push(struct node *n)
{
	unsigned size = get_size(n->type);
	/* Our push will put the object on the stack, so account for it */
	sp += get_stack_size(n->type);
	switch(size) {
	case 1:
		puts("\tpshb");
		return 1;
	case 2:
		puts("\tpshb\n\tpsha");
		return 1;
	case 4:
		/* TODO: if we have no valid X we should ldx/pshx on 6803 */
		puts("\tpshb\n\tpsha\n");
		if (cpu_has_y)
			puts("\tpshy");
		else if (cpu_has_d) {
			puts("\tldd @hireg\n\tpshb\n\tpsha");
			invalidate_work();
		} else {
			puts("\tldaa @hireg+1\n\tpsha\n\tldaa @hireg\n\tpsha");
			invalidate_work();
		}
		return 1;
	}
	return 0;
}

