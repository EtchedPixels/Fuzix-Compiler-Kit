/*
 *	Beginnings of a Z8 code generator
 *
 *	TODO
 *	- register tracking
 *	- using tcm/tm for bitops
 *	- support library
 *	- efficient comparisons
 *	- flag switching on jtrue/false for comparisons
 *	- CCONLY Z80 style
 *	- probably give up on magic hacks and accept r12/r13 is also a temp reg
 *	- registers in r4/5 6/7 8/9 10/11
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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
#define T_RREF		(T_USER+7)
#define T_RSTORE	(T_USER+8)
#define T_RDEREF	(T_USER+9)		/* *regptr */
#define T_REQ		(T_USER+10)		/* *regptr */


/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned argbase;	/* Argument offset in current function */
static unsigned unreachable;	/* Code following an unconditional jump */
static unsigned func_cleanup;	/* Zero if we can just ret out */
static unsigned label_count;	/* Used for internal labels X%u: */

static unsigned r14_sp;		/* R14/15 address relative to SP */
static unsigned r14_valid;	/* R14/15 are a valid local ptr */
static struct node ac_node;	/* What is in AC 0 type = unknown */

/* Minimal tracking on the working register only */

static void invalidate_ac(void)
{
	if (ac_node.op)
		printf(";invalidate ac\n");
	ac_node.op = 0;
}

static void set_ac_node(struct node *n)
{
	unsigned op = n->op;
	printf(";ac node now O %u T %u V %lu\n",
		op, n->type, n->value);
	/* Whatever was in AC just got stored so we are now a valid lref */
	if (op == T_LSTORE)
		op = T_LREF;
	if (op != T_LREF) {
		ac_node.type = 0;
		return;
	}
	memcpy(&ac_node, n, sizeof(ac_node));
	ac_node.op = op;
}

static void flush_all(unsigned f)
{
}

static void invalidate_all(void)
{
	invalidate_ac();
	r14_valid = 0;
}


#define R_SPL	255
#define R_SPH	254

/* 0xEx is special encoding form. We borrow that internally to indicate
   using the currennt working register byte (usually encoded as 0-3) */
#define R_ACCHAR	0xE3
#define R_ACINT		0xE2		/* Start reg for ptr/int */
#define R_ACPTR		0xE2
#define R_ACLONG	0xE0
#define R_ISAC(x)		(((x) & 0xE0) == 0xE0)
#define R_AC		0xE0

#define REGBASE		4		/* Reg vars 4,6,8,10,12 */

#define R_WORK		12		/* 12/13 are work ptr */
#define R_INDEX		14		/* 14/15 are usually index */
/*
 *	These will eventually be smart and track values etc but for now
 *	just do dumb codegen
 */

static void rr_decw(unsigned rr)
{
	if (R_ISAC(rr))
		rr = R_ACPTR;
	printf("\tdecw rr%u\n", rr);
	if (rr == R_INDEX)
		r14_sp--;
}

static void RR_decw(unsigned rr)
{
	printf("\tdecw %u\n", rr);
}

static void rr_incw(unsigned rr)
{
	if (R_ISAC(rr))
		rr = R_ACPTR;
	printf("\tincw rr%u\n", rr);
	if (rr == R_INDEX)
		r14_sp++;
}

static void RR_incw(unsigned rr)
{
	printf("\tincw %u\n", rr);
}

static void load_r_name(unsigned r, struct node *n, unsigned off)
{
	const char *c = namestr(n->snum);
	if (R_ISAC(r)) {
		r = 2;
		set_ac_node(n); 
	}
	if (r == R_INDEX)
		r14_valid = 0;
	printf("\tld r%u,#>_%s+%u\n", r, c, off);
	printf("\tld r%u,#<_%s+%u\n", r + 1, c, off);
}

static void load_r_label(unsigned r, struct node *n, unsigned off)
{
	if (R_ISAC(r)) {
		r = 2;
		set_ac_node(n);
	}
	if (r == R_INDEX)
		r14_valid = 0;
	printf("\tld r%u,#>T%u+%u\n", r, n->val2, off);
	printf("\tld r%u,#<T%u+%u\n", r + 1, n->val2, off);
}

static void load_r_r(unsigned r1, unsigned r2)
{
	if (R_ISAC(r1))
		r1 = r1 & 0x0F;
	if (R_ISAC(r2))
		r2 = r2 & 0x0F;

	if (r1 < 4)
		invalidate_ac();
	if (r1 == R_INDEX || r1 == R_INDEX + 1)
		r14_valid = 0;
	printf("\tld r%u,r%u\n", r1, r2);
}

static void load_r_R(unsigned r1, unsigned r2)
{
	if (R_ISAC(r1))
		r1 = r1 & 0x0F;
	if (r1 == R_INDEX || r1 == R_INDEX + 1)
		r14_valid = 0;
	printf("\tld r%u,%u\n", r1, r2);
}

static void load_R_r(unsigned r1, unsigned r2)
{
	if (R_ISAC(r2))
		r2 = r2 & 0x0F;
	printf("\tld %u,r%u\n", r1, r2);
}

/* Low level ops so we can track them later */
static void op_r_r(unsigned r1, unsigned r2, const char *op)
{
	if (r1 < 4)
		invalidate_ac();
	if (r1 == R_INDEX || r1 == R_INDEX + 1)
		r14_valid = 0;
	printf("\t%s r%u,r%u", op, r1, r2);
}

static void load_r_constb(unsigned r, unsigned char v)
{
	/* Once we do reg tracking we'll be able to deal with dups
	   using ld r1,r2 */
	if (R_ISAC(r))
		r &= 0x0F;
	if (r <= 3)
		invalidate_ac();
	if (r == R_INDEX || r == R_INDEX + 1)
		r14_valid = 0;
	if (v == 0)
		printf("\tclr r%u\n", r);
	else
		printf("\tld r%u,#%u\n", r, v);
}

static void load_rr_const(unsigned r, unsigned v)
{
	if (r == R_ACPTR) {
		invalidate_ac();
		r = 2;
	}
	load_r_constb(r, v >> 8);
	load_r_constb(r + 1, v);
}

static void load_r_const(unsigned r, unsigned long v, unsigned size)
{
	if (R_ISAC(r))
		r = 4 - size;
	/* Lots to do here for optimizing - using clr, copying bytes
	   between regs */
	if (size == 4) {
		load_r_constb(r++, v >> 24);
		load_r_constb(r++, v >> 16);
	}
	if (size > 1)
		load_r_constb(r++, v >> 8);
	load_r_constb(r++, v);
}

static void add_r_const(unsigned r, unsigned long v, unsigned size)
{
	if (R_ISAC(r))
		r = 3;
	else
		r += size - 1;

	if (r == R_INDEX || r == R_INDEX + 1) {
		if (size != 2) 
			r14_valid = 0;
		else
			r14_sp += v;
	}

	if (r < 4)
		invalidate_ac();

	/* Eliminate any low bytes that are not changed */
	while(size && (v & 0xFF) == 0x00) {
		r--;
		size--;
		v >>= 8;
	}
	if (size == 0)
		return;

	/* ADD to r%d is 3 bytes so the only useful cases are word inc/dec of
	   1 which are two bytes as incw/decw and four as add/adc */
	/* FIXME: 2 is also useful */
	if (size == 2) {
		if (v == 1) {
			rr_incw(r - 1);
			return;
		}
		if ((signed)v == -1) {
			rr_decw(r - 1);
			return;
		}
	}
		
	printf("\tadd r%u,#%u\n", r--, (unsigned)v & 0xFF);
	while(--size) {
		v >>= 8;
		printf("\tadc r%u,#%u\n", r--, (unsigned)v & 0xFF);
	}
}

static void add_R_const(unsigned r, unsigned long v, unsigned size)
{
	r += size - 1;
	/* Eliminate any low bytes that are not changed */
	while(size && (v & 0xFF) == 0x00) {
		r--;
		size--;
		v >>= 8;
	}
	if (size == 0)
		return;
	/* TODO: spot cases to use inc/incw/dec/decw */
	printf("\tadd %u,#%u\n", r--, (unsigned)v & 0xFF);
	while(--size) {
		v >>= 8;
		printf("\tadc %u,#%u\n", r--, (unsigned)v & 0xFF);
	}
}

/* Both registers may be the same */
static void add_r_r(unsigned r1, unsigned r2, unsigned size)
{
	if (R_ISAC(r1))
		r1 = 3;
	else
		r1 += size - 1;
	if (R_ISAC(r2))
		r2 = 3;
	else
		r2 += size - 1;

	if (r1 < 4)
		invalidate_ac();

	if (r1 == R_INDEX || r2 == R_INDEX || r1 == R_INDEX + 1 || r2 == R_INDEX + 1)
		r14_valid = 0;

	printf("\tadd r%u,r%u\n", r1--, r2--);
	while(--size)
		printf("\tadc r%u,r%u\n", r1--, r2--);
}


static void sub_r_r(unsigned r1, unsigned r2, unsigned size)
{
	if (R_ISAC(r1))
		r1 = 3;
	else
		r1 += size - 1;
	if (R_ISAC(r2))
		r2 = 3;
	else
		r2 += size - 1;

	if (r1 < 4)
		invalidate_ac();

	if (r1 == R_INDEX || r2 == R_INDEX || r1 == R_INDEX + 1 || r2 == R_INDEX + 1)
		r14_valid = 0;

	printf("\tsub r%u,r%u\n", r1--, r2--);
	while(--size)
		printf("\tsbc r%u,r%u\n", r1--, r2--);
}

static void sub_r_const(unsigned r, unsigned long v, unsigned size)
{
	if (R_ISAC(r))
		r = 3;
	else
		r += size - 1;

	if (r == R_INDEX || r == R_INDEX + 1) {
		if (size != 2) 
			r14_valid = 0;
		else
			r14_sp -= v;
	}

	if (r < 4)
		invalidate_ac();

	/* Eliminate any low bytes that are not changed */
	while(size && (v & 0xFF) == 0x00) {
		r--;
		size--;
		v >>= 8;
	}
	if (size == 0)
		return;

	/* SUB to r%d is 2 bytes so the only useful cases are word inc/dec of
	   1 which are two bytes as incw/decw and four as add/adc */
	if (size == 2) {
		if (v == 1) {
			rr_decw(r);
			return;
		}
		if ((signed)v == -1) {
			rr_decw(r);
			return;
		}
	}
	printf("\tsub r%u,#%u\n", r--, (unsigned)v & 0xFF);
	while(--size) {
		v >>= 8;
		printf("\tsbc r%u,#%u\n", r--, (unsigned)v & 0xFF);
	}
}

/* Same FIXME as cmp_r_const. Will be far better once we have cc tests */
static void cmpne_r_0(unsigned r, unsigned size)
{
	if (R_ISAC(r))
		r = 4 - size;
	else
		load_rr_const(2, 0);
	while(--size)
		printf("\tor r3, r%u\n", r++);
	printf("\tjr z, X%u\n", ++label_count);
	printf("\tinc r3\n");
	printf("X%u:\n", label_count);
}

static void cmpeq_r_0(unsigned r, unsigned size)
{
	/* Will be able to do better for T_BOOL cases and T_BANG */
	cmpne_r_0(r, size);
	printf("\txor r3,#1\n");
}

static void mono_r(unsigned r, unsigned size, const char *op)
{
	if (R_ISAC(r))
		r = 4 - size;
	if (r < 4)
		invalidate_ac();
	if (r == R_INDEX || r == R_INDEX + 1)
		r14_valid = 0;
	while(size--)
		printf("\t%s r%u\n", op, r++);
}

/*
 *	Perform a right shift of a register set signed or unsigned.
 *	Doesn't currently know about 8bit at a time shifts for unsigned
 *	in particular
 */
static void rshift_r(unsigned r, unsigned size, unsigned uns)
{
	if (R_ISAC(r))
		r = 4 - size;
	if (r < 4)
		invalidate_ac();
	if (uns)
		printf("\trcf\nrrc r%u\n", r++);
	else
		printf("\tsra r%u\n", r++);
	while(--size)
		printf("\trrc r%u\n", r++);
}

#define OP_AND	0
#define OP_OR	1
#define OP_XOR	2

static void logic_r_const(unsigned r, unsigned long v, unsigned size, unsigned op)
{
	const char *opn = "and\0or\0\0xor" + op * 4;
	unsigned n;

	if (R_ISAC(r))
		r = 4;
	else
		r += size;

	if (r <= 4)
		invalidate_ac();

	if (r >= R_INDEX)
		r14_valid = 0;

	/* R is now 1 after low byte */
	while(size--) {
		r--;
		n = v & 0xFF;
		v >>= 8;
		if (n == 0xFF) {
			if (op == OP_OR)
				load_r_constb(r, 0xFF);
			else if (op == OP_XOR)
				printf("\tcom r%u\n", r);
			/* and nothing for and FF */
			continue;
		}
		else if (n == 0x00) {
			if (op == OP_AND)
				load_r_constb(r, 0x00);
			/* OR and XOR do nothing */
			continue;
		}
		else
			printf("\t%s r%u, #%u\n", opn, r, n);
	}
}

static void load_r_local(unsigned r, unsigned off)
{
	int diff;
	if (R_ISAC(r))
		r = 2;

	printf(";local %d (cache %d)\n", off - sp, r14_sp);
	if (r == R_INDEX && r14_valid) {
		/* TODO: For exact match case we should allow it even if not INDEX and
		   just copy the registers. Maybe also if one out */
		diff = r14_sp - (off - sp);
		if (diff == 0)
			return;
		/* incw/decw are 2 bytes a shot, add is 3 also */
		if (diff < -2 || diff > 2) {
			add_r_const(R_INDEX, -diff, 2);
			r14_valid = 1;
			r14_sp = off - sp;
			return;
		}
		/* Within range but not quite right */
		printf("; diff %d\n", diff);
		while(diff < 0) {
			diff++;
			rr_incw(R_INDEX);
		}
		while(diff > 0) {
			diff--;
			rr_decw(R_INDEX);
		}
	} else {
		load_r_R(r, R_SPH);
		load_r_R(r + 1, R_SPL);
		add_r_const(r, off, 2);
		if (r == R_INDEX) {
			r14_valid = 1;
			r14_sp = off - sp;
		}
	}
}

static void load_r_memr(unsigned val, unsigned rr, unsigned size)
{
	if (R_ISAC(val))
		val = 4 - size;
	/* Check this on its own as we sometimes play games with AC
	   registers */
	if (val < 4)
		invalidate_ac();
	printf("\tlde r%u, @rr%u\n", val, rr);
	while(--size) {
		val++;
		rr_incw(rr);
		printf("\tlde r%u, @rr%u\n", val, rr);
	}
}

static void store_r_memr(unsigned val, unsigned rr, unsigned size)
{
	if (R_ISAC(val))
		val = 4 - size;
	printf("\tlde @rr%u, r%u\n", rr, val);
	while(--size) {
		val++;
		rr_incw(rr);
		printf("\tlde @rr%u, r%u\n", rr, val);
	}
}

/* Store reverse direction. We use it for eq ops for the moment but we
   could also try and spot cases where the ptr is at the other end of
   the value (ditto we could revload). Would need some smarts in T_LREF,
   T_LSTORE and a helper to decide which way to go : TODO */
static void revstore_r_memr(unsigned val, unsigned rr, unsigned size)
{
	if (R_ISAC(val))
		val = 3;
	else
		val += size - 1;
	printf("\tlde @rr%u, r%u\n", rr, val);
	while(--size) {
		val--;
		rr_decw(rr);
		printf("\tlde @rr%u, r%u\n", rr, val);
	}
}

static unsigned logic_eq_const(struct node *r,unsigned v, unsigned size, unsigned op)
{
	if (r->op != T_CONSTANT || size > 2)
		return 0;
	/* Could spot 0000 and FFFF but prob no point */
	load_r_r(0, 2);
	load_r_r(1, 3);
	load_r_memr(R_AC, 0, size);
	logic_r_const(R_AC, v, size, op);
	revstore_r_memr(R_AC, 0, size);
	return 1;
}

static void push_r(unsigned r)
{
	if (R_ISAC(r))
		r = r & 0x0F;
	printf("\tpush r%u\n", r);
}

static void pop_r(unsigned r)
{
	if (R_ISAC(r)) {
		invalidate_ac();
		r = r & 0x0F;
	}
	if (r == R_INDEX || r == R_INDEX + 1)
		r14_valid = 0;
	printf("\tpop r%u\n", r);
}

static void push_rr(unsigned rr)
{
	if (R_ISAC(rr))
		rr = rr & 0x0F;
	printf("\tpush r%u\n", rr + 1);
	printf("\tpush r%u\n", rr);
}

static void pop_rr(unsigned rr)
{
	if (R_ISAC(rr))
		rr = rr & 0x0F;
	if (rr == R_INDEX)
		r14_valid = 0;
	printf("\tpop r%u\n", rr);
	printf("\tpop r%u\n", rr + 1);
}

static void pop_ac(unsigned size)
{
	unsigned r = 4 - size;
	while(size--)
		pop_r(r++);
}

static void push_ac(unsigned size)
{
	switch(size) {
	case 1:
		push_r(R_ACCHAR);
		break;
	case 2:
		push_rr(R_ACINT);
		break;
	case 4:
		push_rr(R_ACINT);
		push_rr(R_ACLONG);
		break;
	default:
		error("psz");
	}
}

static void pop_op(unsigned r, const char *op, unsigned size)
{
	if (R_ISAC(r)) {
		invalidate_ac();
		r = 4 - size;
	}
	while(size--) {
		pop_r(R_WORK);
		printf("\t%s r%u, r%u\n", op, r++, R_WORK);
	}
}

/* Address is on the stack, value to logic it with is in AC */
static void logic_popeq(unsigned size, const char *op)
{
	unsigned n = size;
	pop_rr(R_INDEX);
	while(n) {
		load_r_memr(R_WORK, R_INDEX, 1);
		rr_incw(R_INDEX);
		printf("\t%s r%u, r%u\n", op, 4 - n, R_WORK);
		n--;
	}
	revstore_r_memr(R_AC, R_INDEX, size);
}

static void ret_op(void)
{
	printf("\tret\n");
	unreachable = 1;
}

static unsigned label(void)
{
	invalidate_all();
	unreachable = 0;
	printf("X%u:\n", ++label_count);
	return label_count;
}

static void djnz_r(unsigned r, unsigned l)
{
	if (r == R_INDEX || r == R_INDEX + 1)
		r14_valid = 0;
	if (R_ISAC(r))
		r = 3;
	printf("\tdjnz r%u, X%u\n", r, l);
}

/*
 *	Object sizes
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
	fprintf(stderr, "type %x\n", t);
	error("gs");
	return 0;
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
		return 1;
	return 0;
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

	/* TODO
		- rewrite some reg ops
	*/

	/* *regptr */
	if (op == T_DEREF && r->op == T_RREF) {
		n->op = T_RDEREF;
		n->right = NULL;
		n->val2 = 0;
		n->value = r->value;
		free_node(r);
		return n;
	}
	/* *regptr = */
	if (op == T_EQ && l->op == T_RREF) {
		n->op = T_REQ;
		n->val2 = 0;
		n->value = l->value;
		n->left = NULL;
		free_node(l);
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
	unreachable = 0;
	printf("_%s:\n", name);
}

/* Generate the stack frame */
/* TODO: defer this to statements so we can ld/push initializers */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size;
	sp = 0;

	if (size || func_flags & F_REG(1))
		func_cleanup = 1;
	else
		func_cleanup = 0;

	argbase = ARGBASE;
#if 0	
	if (func_flags & F_REG(1)) {
		opcode(OP_PUSH, R_BC|R_SP, R_SP, "push b");
		argbase += 2;
	}
#endif
	if (size > 4) {
		/* Special handling because of the stack and interrupts */
		load_r_R(R_INDEX, R_SPL);
		/* TODO: will need special tracking of course */
		printf("\tsub r%u,#%u\n", R_INDEX, size & 0xFF);
		printf("\tsbc 255,#%u\n", size >> 8);
		load_R_r(R_SPL, R_INDEX);
		r14_valid = 0;
		r14_sp = 0;
		return;
	}
	while(size--)
		RR_decw(R_SPH);
	r14_valid = 0;
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp != 0)
		error("sp");
	if (unreachable)
		return;
	add_R_const(R_SPH, size, 2);
	ret_op();
}

void gen_label(const char *tail, unsigned n)
{
	unreachable = 0;
	/* A branch label means the state is unknown so force any
	   existing state and don't assume anything */
	invalidate_all();
	printf("L%u%s:\n", n, tail);
}

/* A return statement. We can sometimes shortcut this if we have
   no cleanup to do */
unsigned gen_exit(const char *tail, unsigned n)
{
	if (func_cleanup) {
		gen_jump(tail, n);
		return 0;
	} else {
		ret_op();
		return 1;
	}
}

/* FIXME: teach assembler to adjust jr and make these use jr */
void gen_jump(const char *tail, unsigned n)
{
	/* Force anything deferred to complete before the jump */
	flush_all(0);
	printf("\tjp L%u%s\n", n, tail);
	unreachable = 1;
}

/* TODO: Will need work when we implement flag flipping and other compare tricks */
void gen_jfalse(const char *tail, unsigned n)
{
	flush_all(1);	/* Must preserve flags */
	printf("\tjp z,L%u%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	flush_all(1);	/* Must preserve flags */
	printf("\tjp nz,L%u%s\n", n, tail);
}

static void gen_cleanup(unsigned v)
{
	printf(";cleanup %d\n", v);
	sp -= v;
	if (v >= 4) {
		add_R_const(R_SPH, v, 2);
		return;
	}
	while(v--)
		RR_incw(R_SPH);
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
	invalidate_ac();
	printf("\tcall __");
	r14_valid = 0;
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
			printf("\tor r20,r3\n");
	}
}

void gen_switch(unsigned n, unsigned type)
{
	/* TODO: this probably belongs as a routine in the cpu bits */
	printf("\tld r%u, #>Sw%u\n", R_INDEX, n);
	printf("\tld r%u, #<Sw%u\n", R_INDEX + 1, n);
	/* TODO: tracking fixes for R14/15 when tracking added ??*/
	invalidate_all();
	/* Nothing is preserved over a switch but */
	printf("\tjp __switch");
	helper_type(type, 0);
	putchar('\n');
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%u:\n\t.word %u\n", n, size);
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

/* The label for a literal (currently only strings) */
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
/*	printf("\t.setcpu %u\n", cpu); */
}

void gen_end(void)
{
	flush_all(0);
}

void gen_tree(struct node *n)
{
	codegen_lr(n);
	printf(";\n");
/*	printf(";SP=%d\n", sp); */
}

/*
 *	Return 1 if the node can be turned into direct access. The VOID check
 *	is a special case we need to handle stack clean up of void functions.
 */
static unsigned access_direct(struct node *n)
{
	unsigned op = n->op;

	/* FIXME */
	
	/* The 8080 we can reliably access stuff within 253 bytes of the
	   current stack pointer. 8085 we don't have the short helpers as they
	   are not worth it for this case, but we can do it via the 4 byte one */
	if (op == T_LREF && n->value + sp < 253)
		return 1;
	/* We can direct access integer or smaller types that are constants
	   global/static or string labels */
	/* TODO group the user ones together for a range check ? */
	if (op != T_CONSTANT && op != T_NAME && op != T_LABEL &&
		 op != T_NREF && op != T_LBREF && op != T_RREF)
		 return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;
	return 1;
}

/* Generate compares */
static unsigned gen_compc(const char *op, struct node *n, struct node *r, unsigned sign)
{
#if 0
	if (r->op == T_CONSTANT && r->value == 0 && r->type != FLOAT) {
		char buf[10];
		strcpy(buf, op);
		strcat(buf, "0");
		if (sign)
			helper_s(n, buf);
		else
			helper(n, buf);
		n->flags |= ISBOOL;
		return 1;
	}
	if (gen_deop(op, n, r, sign)) {
		n->flags |= ISBOOL;
		return 1;
	}
#endif	
	return 0;
}

static int count_mul_cost(unsigned n)
{
	int cost = 0;
	if ((n & 0xFF) == 0) {
		n >>= 8;
		cost += 3;		/* mov mvi */
	}
	while(n > 1) {
		if (n & 1)
			cost += 3;	/* push pop dad d */
		n >>= 1;
		cost++;			/* dad h */
	}
	return cost;
}

/* Write the multiply for any value > 0 */
static void write_mul(unsigned n)
{
#if 0
	unsigned pops = 0;
	if ((n & 0xFF) == 0) {
		opcode(OP_MOV, R_L, R_H, "mov h,l");
		opcode(OP_MVI, 0, R_L, "mvi l,0");
		n >>= 8;
	}
	while(n > 1) {
		if (n & 1) {
			pops++;
			opcode(OP_PUSH, R_SP|R_HL, R_SP, "push h");
		}
		opcode(OP_DAD, R_HL, R_HL, "dad h");
		n >>= 1;
	}
	while(pops--) {
		opcode(OP_POP, R_SP, R_SP|R_DE, "pop d");
		opcode(OP_DAD, R_DE|R_HL, R_HL, "dad d");
	}
#endif	
}

static unsigned can_fast_mul(unsigned s, unsigned n)
{
#if 0
	/* Pulled out of my hat 8) */
	unsigned cost = 15 + 3 * opt;
	if (optsize)
		cost = 10;
	if (s > 2)
		return 0;
	if (n == 0 || count_mul_cost(n) <= cost)
		return 1;
#endif		
	return 0;
}

static void gen_fast_mul(unsigned r, unsigned s, unsigned n)
{
#if 0
	if (n == 0)
		opcode(OP_LXI, 0, R_HL, "lxi h,0");
	else
		write_mul(n);
#endif		
}

static unsigned gen_fast_div(unsigned r, unsigned s, unsigned n)
{
	return 0;
#if 0
	if (n & (n - 1))
		return 0;

	while(n > 1) {
		opcode(OP_ARHL, R_HL,  R_HL, "arhl");
		n >>= 1;
	}
#endif
	return 1;	
}

static unsigned gen_fast_udiv(unsigned r, unsigned n, unsigned s)
{
	if (s != 2)
		return 0;
	if (n == 1)
		return 1;
	if (n == 256) {
		load_r_r(r, r + 1);
		load_r_constb(r + 1, 0);
		return 1;
	}
	return 0;
}

static unsigned gen_fast_remainder(unsigned r, unsigned n, unsigned s)
{
/*	unsigned mask; */
	if (s != 2)
		return 0;
	if (n == 1) {
		load_r_const(r, 0, s);
		return 1;
	}
	if (n == 256) {
		load_r_constb(r + 1, 0);
		return 1;
	}
	if (n & (n - 1))
		return 0;
#if 0		
	if (!optsize) {
		mask = n - 1;
		gen_logicc(NULL, s, "ani", mask, 1);
		return 1;
	}
#endif	
	return 0;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 *
 *	On entry the working value in in r0-r3, and the other value is
 *	not yet resolved. Typically this is useful for stuff like a constant
 *	load, but we can do a lot of reg/reg ops this way.
 */
unsigned gen_direct(struct node *n)
{
	unsigned size = get_size(n->type);
	struct node *r = n->right;
	unsigned v;
	unsigned nr = n->flags & NORETURN;
	unsigned x;
	unsigned u = n->type & UNSIGNED;

	/* We only deal with simple cases for now */
	if (r) {
		if (!access_direct(n->right))
			return 0;
		v = r->value;
	}

	/* We can do a lot of stuff because we have r14/15 as a scratch */
	switch (n->op) {
	case T_CLEANUP:
		printf(";cleanup\n");
		gen_cleanup(v);
		return 1;
	case T_NSTORE:
		load_r_name(R_INDEX, n, v);
		store_r_memr(R_AC, R_INDEX, size);
		break;
	case T_LBSTORE:
		load_r_label(R_INDEX, n, v);
		store_r_memr(R_AC, R_INDEX, size);
		return 1;
	case T_LSTORE:
		load_r_local(R_INDEX, v + sp);
		store_r_memr(R_AC, R_INDEX, size);
		set_ac_node(n);
		return 1;
	case T_RSTORE:
		load_r_r(REGBASE + 2 * n->value, R_ACCHAR);
		if (size > 1)
			load_r_r(REGBASE + 2 * n->value + 1, R_ACINT);
		return 1;
	case T_EQ:
		/* We may be able to do better with thought */
		if (r->op == T_CONSTANT) {
			load_r_r(R_INDEX, R_ACPTR);
			load_r_r(R_INDEX + 1, R_ACPTR + 1);
			load_r_const(R_AC, v, size);
			store_r_memr(R_AC, R_INDEX, size);
			return 1;
		}
		return 0;
	/* Some of these would benefit from helpers using r1r/r15 r0-r3 or
	   similar. FIXME : will need to rework the repeated_ stuff when we
	   do tracking into loops of rr_dec etc */
	case T_PLUS:
		if (r->op == T_CONSTANT && n->type != FLOAT) {
			if (!nr)
				add_r_const(R_AC, v, size);
			return 1;
		}
		return 0;
	case T_MINUS:
		if (r->op == T_CONSTANT && n->type != FLOAT) {
			if (!nr)
				add_r_const(R_AC, -v, size);
			return 1;
		}
		return 0;
	case T_STAR:
#if 0	
		if (r->op == T_CONSTANT && n->type != FLOAT) {
			if (nr)
				return 1;
			if (s <= 2 && can_fast_mul(s, r->value)) {
				gen_fast_mul(R_AC, s, 0, r->value);
				return 1;
			}
		}
#endif		
		return 0;
	case T_SLASH:
		if (r->op == T_CONSTANT && size <= 2) {
			if (nr)
				return 1;
			if (n->type & UNSIGNED) {
				if (gen_fast_udiv(R_AC, size, v))
					return 1;
			} else {
				if (gen_fast_div(R_AC, size, v))
					return 1;
			}
		}
		return 0;
	case T_PERCENT:
		if (r->op == T_CONSTANT) {
			if (nr)
				return 1;
			if (n->type & UNSIGNED) {
				if (size <= 2 && gen_fast_remainder(0, size, r->value))
					return 1;
			}
		}
		return 0;
	case T_AND:
		if (r->op == T_CONSTANT) {
			if (!nr)
				logic_r_const(R_AC, r->value, size, OP_AND);
			return 1;
		}
		return 0;
	case T_OR:
		if (r->op == T_CONSTANT) {
			if (!nr)
				logic_r_const(R_AC, r->value, size, OP_OR);
			return 1;
		}
		return 0;
	case T_HAT:
		if (r->op == T_CONSTANT) {
			if (!nr)
				logic_r_const(R_AC, r->value, size, OP_XOR);
			return 1;
		}
		return 0;
	case T_EQEQ:
		if (r->op == T_CONSTANT && n->type != FLOAT) {
			load_r_const(12, r->value , size);
			helper(n, "cceqconst");
			n->flags |= ISBOOL;
			return 1;
		}
		return 0;
	/* The const form helpers do the reverse compare so we use the opposite one */
	case T_GTEQ:
		if (r->op == T_CONSTANT && n->type != FLOAT) {
			load_r_const(12, r->value , size);
			helper_s(n, "cclteqconst");
			n->flags |= ISBOOL;
			return 1;
		}
		return 0;
	case T_GT:
		if (r->op == T_CONSTANT && n->type != FLOAT) {
			load_r_const(12, r->value , size);
			helper_s(n, "ccltconst");
			n->flags |= ISBOOL;
			return 1;
		}
		return 0;
	case T_LTEQ:
		if (r->op == T_CONSTANT && n->type != FLOAT) {
			load_r_const(12, r->value , size);
			helper_s(n, "ccgteqconst");
			n->flags |= ISBOOL;
			return 1;
		}
		return 0;
	case T_LT:
		if (r->op == T_CONSTANT && n->type != FLOAT) {
			load_r_const(12, r->value , size);
			helper_s(n, "ccgtconst");
			n->flags |= ISBOOL;
			return 1;
		}
		return 0;
	case T_BANGEQ:
		if (r->op == T_CONSTANT && n->type != FLOAT) {
			load_r_const(12, r->value , size);
			helper(n, "ccneconst");
			n->flags |= ISBOOL;
			return 1;
		}
		return 0;
	case T_LTLT:
		if (r->op == T_CONSTANT) {
			if (nr)
				return 1;
			/* TODO: optimize byte parts of shifts */
			r->value &= (8 * size) - 1;
			if (r->value == 0)
				return 1;
			if (r->value > 1) {
				load_r_constb(R_INDEX, r->value & 31);
				x = label();
			}
			add_r_r(R_AC, R_AC, size);
			if (r->value > 1)
				djnz_r(R_INDEX, x);
			return 1;
		}
		return 0;
	case T_GTGT:
		if (r->op == T_CONSTANT) {
			if (nr)
				return 1;
			r->value &= (8 * size) - 1;
			if (r->value == 0)
				return 1;
			if (r->value > 1) {
				load_r_constb(R_INDEX, r->value & 31);
				x = label();
			}
			rshift_r(R_AC, size, n->type & UNSIGNED);
			if (r->value > 1)
				djnz_r(R_INDEX, x);
			return 1;
		}
		return 0;
	/* Shorten post inc/dec if result not needed - in which case it's the same as
	   pre inc/dec */
	case T_PLUSPLUS:
		if (r->op != T_CONSTANT || size > 2)
			return 0;
		if (!(n->flags & NORETURN)) {
			/* r2/3 is the pointer,  */
			/* Move it to be safe from the load */
			load_r_r(R_INDEX, R_ACPTR);
			load_r_r(R_INDEX + 1, R_ACPTR + 1);
			load_r_memr(R_AC, R_INDEX, size);
			/* load_r_memr bumps r_index up */
			push_ac(size);
			add_r_const(R_AC, v, size);
			/* Revstore compensates by doing the store
			   bacwards */
			revstore_r_memr(R_AC, R_INDEX, size);
			pop_ac(size);
			return 1;
		}
		/* Noreturn is like pluseq */
	case T_PLUSEQ:
		if (r->op != T_CONSTANT || size > 2)
			return 0;
			
		/* FIXME: will need an "and not register" check */
		if ((n->flags & NORETURN)  && size <= 2) {
			load_r_memr(0, 2, size);
			add_r_const(0, v, size);
			revstore_r_memr(0, 2, size);
			return 1;
		}
		/* Copy the pointer over but keep R14/15 */
		load_r_r(R_WORK, 2);
		load_r_r(R_WORK + 1, 3);
		/* Value into 2/3 */
		load_r_memr(R_AC, R_WORK, size);
		add_r_const(R_AC, v, size);
		revstore_r_memr(R_AC, R_WORK, size);
		return 1;
	case T_MINUSMINUS:
		if (r->op != T_CONSTANT || size > 2)
			return 0;
		if (!(n->flags & NORETURN)) {
			/* r2/3 is the pointer,  */
			load_r_r(R_INDEX, R_ACPTR);
			load_r_r(R_INDEX + 1, R_ACPTR + 1);
			load_r_memr(R_AC, R_INDEX, size);
			push_ac(size);
			add_r_const(0, -v, size);
			revstore_r_memr(0, 2, size);
			pop_ac(size);
			return 1;
		}
		/* Noreturn is like minuseq */
	case T_MINUSEQ:
		if (r->op != T_CONSTANT || size > 2)
			return 0;
		/* FIXME: will need an "not register" check */
		if ((n->flags & NORETURN) && size <= 2) {
			load_r_memr(0, 2, size);
			add_r_const(0, -v, size);
			revstore_r_memr(0, 2, size);
			return 1;
		}
		/* Copy the pointer over but keep R14/15 */
		load_r_r(R_WORK, R_ACPTR);
		load_r_r(R_WORK + 1, R_ACPTR + 1);
		load_r_memr(R_AC, R_WORK, size);
		add_r_const(R_AC, -v, size);
		revstore_r_memr(R_AC, R_WORK, size);
		return 1;
	case T_ANDEQ:
		return logic_eq_const(r, v, size, OP_AND);
	case T_OREQ:
		return logic_eq_const(r, v, size, OP_OR);
	case T_HATEQ:
		return logic_eq_const(r, v, size, OP_XOR);
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
 *	Allow the code generator to short cut any subtrees it can directly
 *	generate.
 */
unsigned gen_shortcut(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned nr = n->flags & NORETURN;

	/* Unreachable code we can shortcut into nothing ..bye.. */
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
	/* We don't know if the result has set the condition flags
	 * until we generate the subtree. So generate the tree, then
	 * either do nice things or use the helper */
	if (n->op == T_BOOL) {
		codegen_lr(r);
		if (r->flags & ISBOOL)
			return 1;
		s = get_size(r->type);
		if (s <= 2 && (n->flags & CCONLY)) {
			if (s == 2)
				printf("\tor r2,r3\n");
			else
				printf("\tor r3,r3\n");
			return 1;
		}
		/* Too big or value needed */
		helper(n, "bool");
		n->flags |= ISBOOL;
		return 1;
	}
#if 0	
	if (l && l->op == T_REG) {
		unsigned v = r->value;
		/* Never a long... */
		switch(n->op) {
		case T_MINUSMINUS:
			v = -v;
		case T_PLUSPLUS:
			/* R is always a constant for ++ */
			if (!nr) {
				load_r_r(0, REGBASE + 2 * n->value);
				if (sz == 2)
					load_r_r(1, REGBASE + 2 * n->value);
			}
			/* TODO catch -ve versions remembering 'v' is unsigned */
			if (v < 10) {	/* TODO pick an amount */
				if (sz == 2)
					repeated_op(v, REGBASE + 2 * n->value, "incw");
				else
					repeated_op(v, REGBASE + 2 * n->value, "inc");
				return 1;
			}
			const_op(REGBASE + 2 * n->value, "add", "adc", v, size);
			return 1;
		case T_PLUSEQ:
			/* Trickier as right might not be reg or const TODO */
			if (reg_canincdec(r, s, v)) {
				reg_incdec(s, v);
				if (nr)
					return 1;
				if (n->op == T_PLUSEQ) {
					loadhl(n, s);
				}
			} else {
				/* Amount to add into HL */
				codegen_lr(r);
				opcode(OP_DAD, R_HL|R_BC, R_HL, "dad b");
				opcode(OP_MOV, R_L, R_C, "mov c,l");
				if (s == 2)
					opcode(OP_MOV, R_H, R_B, "mov b,h");
			}
			if (n->op == T_PLUSPLUS && !(n->flags & NORETURN)) {
				opcode(OP_POP, R_SP, R_HL|R_SP, "pop h");
				sp -= 2;
			}
			return 1;
		case T_MINUSEQ:
			if (reg_canincdec(r, s, -v)) {
				reg_incdec(s, -v);
				loadhl(n, s);
				return 1;
			}
			if (r->op == T_CONSTANT) {
				opcode(OP_LXI, 0, R_HL, "lxi h,%u", -v);
				opcode(OP_DAD, R_HL|R_BC, R_HL, "dad b");
				loadbc(s);
				return 1;
			}
			/* Get the subtraction value into HL */
			codegen_lr(r);
			helper(n, "bcsub");
			/* Result is only left in BC reload if needed */
			loadhl(n, s);
			return 1;
		/* For now - we can do better - maybe just rewrite them into load,
		   op, store ? */
		case T_STAREQ:
			/* TODO: constant multiply */
			if (r->op == T_CONSTANT) {
				if (can_fast_mul(s, v)) {
					loadhl(NULL, s);
					gen_fast_mul(s, v);
					loadbc(s);
					return 1;
				}
			}
			codegen_lr(r);
			helper(n, "bcmul");
			return 1;
		case T_SLASHEQ:
			/* TODO: power of 2 constant divide maybe ? */
			codegen_lr(r);
			helper_s(n, "bcdiv");
			return 1;
		case T_PERCENTEQ:
			/* TODO: spot % 256 case */
			codegen_lr(r);
			helper(n, "bcrem");
			return 1;
		case T_SHLEQ:
			if (r->op == T_CONSTANT) {
				if (s == 1 && v >= 8) {
					opcode(OP_MVI, 0, R_C, "mvi c,0");
					loadhl(n, s);
					return 1;
				}
				if (s == 1) {
					printf("\tmov a,c\n");
					repeated_op("add a", v);
					printf("\tmov c,a\n");
					loadhl(n, s);
					return 1;
				}
				/* 16 bit */
				if (v >= 16) {
					opcode(OP_LXI, 0, R_B, "lxi b,0");
					loadhl(n, s);
					return 1;
				}
				if (v == 8) {
					printf("\tmov b,c\n\tmvi c,0\n");
					loadhl(n, s);
					return 1;
				}
				if (v > 8) {
					printf("\tmov a,c\n");
					repeated_op("add a", v - 8);
					printf("\tmov b,a\nvi c,0\n");
					loadhl(n, s);
					return 1;
				}
				/* 16bit full shifting */
				loadhl(NULL, s);
				repeated_op("dad h", v);
				loadbc(s);
				return 1;
			}
			codegen_lr(r);
			helper(n, "bcshl");
			return 1;
		case T_SHREQ:
			if (r->op == T_CONSTANT) {
				if (v >= 8 && s == 1) {
					opcode(OP_MVI, 0, R_C, "mvi c,0");
					loadhl(n, s);
					return 1;
				}
				if (v >= 16) {
					opcode(OP_LXI, 0, R_BC, "lxi b,0");
					loadhl(n, s);
					return 1;
				}
				if (v == 8 && (n->type & UNSIGNED)) {
					printf("\tmov c,b\nmvi b,0\n");
					loadhl(n, s);
					return 1;
				}
				if (s == 2 && !(n->type & UNSIGNED) && cpu == 8085 && v < 2 + 4 * opt) {
					loadhl(NULL,s);
					repeated_op("arhl", v);
					loadbc(s);
					return 1;
				}
			}
			codegen_lr(r);
			helper_s(n, "bcshr");
			return 1;
		case T_ANDEQ:
			reg_logic(n, s, 0, "bcana");
			return 1;
		case T_OREQ:
			reg_logic(n, s, 1, "bcora");
			return 1;
		case T_HATEQ:
			reg_logic(n, s, 2, "bcxra");
			return 1;
		}
	}
#endif
	return 0;
}

/* Stack the node which is currently in the working register */
unsigned gen_push(struct node *n)
{
	unsigned size = get_size(n->type);

	/* Our push will put the object on the stack, so account for it */
	sp += size;
	
	push_ac(size);
	return 1;
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
	if (!(rt & UNSIGNED))
		return 0;
	if (rs == 1)
		load_r_constb(R_ACINT, 0);
	if (ls == 4) {
		load_r_constb(R_AC, 0);
		load_r_constb(R_AC + 1, 0);
	}
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned v;
	unsigned nr = n->flags & NORETURN;
	unsigned x;
	unsigned u = n->type & UNSIGNED;
	/* We adjust sp so track the pre-adjustment one too when we need it */

	v = n->value;

	/* An operation with a left hand node will have the left stacked
	   and the operation will consume it so adjust the stack.

	   The exception to this is comma and the function call nodes
	   as we leave the arguments pushed for the function call */

	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_size(n->left->type);

	switch (n->op) {
	case T_NREF:
		load_r_name(R_INDEX, n, v);
		load_r_memr(R_AC, R_INDEX, size);
		set_ac_node(n);
		return 1;
	case T_LBREF:
		load_r_label(R_INDEX, n, v);
		load_r_memr(R_AC, R_INDEX, size);
		set_ac_node(n);
		return 1;
	case T_LREF:
		/* We are loading something then not using it, and it's local
		   so can go away */
		if (nr)
			return 1;
		/* Is it already loaded ? */
		if (ac_node.op == T_LREF && ac_node.value == v && 
			get_size(ac_node.type) >= size) {
			printf(";avoided load %d\n", v);
			return 1;
		}
		/* effectively SPL/SPH + n */
		load_r_local(R_INDEX, v + sp);
		load_r_memr(R_AC, R_INDEX, size);
		set_ac_node(n);
		return 1;
	case T_RREF:
		load_r_r(R_ACCHAR, REGBASE + 2 * n->value);
		if (size == 2)
			load_r_r(R_ACINT, REGBASE + 2 * n->value + 1);
		return 1;
	case T_NSTORE:
		load_r_name(R_INDEX, n, v);
		store_r_memr(R_AC, R_INDEX, size);
		return 1;
	case T_LBSTORE:
		load_r_label(R_INDEX, n, v);
		store_r_memr(R_AC, R_INDEX, size);
		return 1;
	case T_LSTORE:
		load_r_local(R_INDEX, v + sp);
		store_r_memr(R_AC, R_INDEX, size);
		set_ac_node(n);
		return 1;
	case T_RSTORE:
		load_r_r(REGBASE + 2 * n->value, R_ACCHAR);
		if (size > 1)
			load_r_r(REGBASE + 2 * n->value + 1, R_ACINT);
		return 1;
		/* Call a function by name */
	case T_CALLNAME:
		invalidate_all();
		printf("\tcall _%s+%u\n", namestr(n->snum), v);
		return 1;
	case T_EQ:
		pop_rr(R_INDEX);
		store_r_memr(R_AC, R_INDEX, size);
		return 1;
	case T_RDEREF:
		load_r_memr(R_AC, REGBASE + 2 * n->value, 0);
		return 1;
	case T_DEREF:
		/* Have to deal with overlap */
		load_r_r(R_INDEX, R_ACPTR);
		load_r_r(R_INDEX + 1, R_ACCHAR);
		load_r_memr(R_AC, R_INDEX, size);
		return 1;
	case T_FUNCCALL:
		invalidate_all();
		/* Rather than mess with indirection use a helper */
		printf("\tcall __jmpr2\n");
		return 1;
	case T_LABEL:
		if (nr)
			return 1;
		load_r_label(R_AC, n, v);
		set_ac_node(n);
		return 1;
	case T_CONSTANT:
		if (nr)
			return 1;
		load_r_const(R_AC, n->value, size);
		return 1;
	case T_NAME:
		if (nr)
			return 1;
		load_r_name(R_AC, n, v);
		set_ac_node(n);
		return 1;
	case T_ARGUMENT:
		v += frame_len + argbase;
	case T_LOCAL:
		if (nr)
			return 1;
		load_r_local(R_AC, v + sp);
		set_ac_node(n);
		return 1;
	case T_REG:
		if (nr)
			return 1;
		/* A register has no address.. we need to sort this out */
		error("rega");
		return 1;
	case T_CAST:
		if (nr)
			return 1;
		return gen_cast(n);
	case T_PLUS:
		/* Tricky as big endian on stack */
		/* FIXME: needs a !reg check */
		if (size == 4 && n->type != FLOAT) {
			/* Games time */
			pop_r(R_WORK);
			op_r_r(3, R_WORK, "add");
			pop_r(R_WORK);
			op_r_r(2, R_WORK, "adc");
			pop_r(R_WORK);
			op_r_r(1, R_WORK, "adc");
			pop_r(R_WORK);
			op_r_r(0, R_WORK, "adc");
			return 1;
		}
		if (size > 2)
			return 0;
		/* FIXME: will need a "not register" check .. otherwise
		   use the R_WORK version above */
		if (size == 2) {
			/* Pop into r0,r1 which are free as accum is 16bit */
			pop_rr(0);
			add_r_r(R_AC, 0, 2);
			return 1;
		}
		/* size 1 */
		pop_r(0);
		add_r_r(R_AC, 0, 1);
		return 1;
	case T_MINUS:
		/* We are doing stack - ac. This isn't ideal but we've
		   dealt with the simple cases already. We could more
		   in gen_shortcut perhaps if it is still an issue */
		if (size == 4 && n->type != FLOAT) {
			/* Lots of joy involved */
			pop_r(R_WORK);
			op_r_r(R_WORK, 3, "sub");
			load_r_r(3, R_WORK);
			pop_r(R_WORK);
			op_r_r(R_WORK, 2, "sbc");
			load_r_r(2, R_WORK);
			pop_r(R_WORK);
			op_r_r(R_WORK, 1, "sbc");
			load_r_r(1, R_WORK);
			pop_r(R_WORK);
			op_r_r(R_WORK, 0, "sbc");
			load_r_r(0, R_WORK);
			return 1;
		}
		if (size > 2)
			return 0;
		if (size == 2) {
			/* Pop into r0,r1 which are free as accum is 16bit */
			pop_rr(0);
			sub_r_r(0, R_ACINT, 2);
			load_r_r(R_ACINT, 0);
			load_r_r(R_ACCHAR, 1);
			return 1;
		}
		/* size 1 */
		pop_r(0);
		sub_r_r(0, R_ACCHAR, 1);
		load_r_r(R_ACCHAR, 0);
		return 1;
	case T_AND:
		pop_op(R_AC, "and", size);
		return 1;
	case T_OR:
		pop_op(R_AC, "or", size);
		return 1;
	case T_HAT:
		pop_op(R_AC, "xor", size);
		return 1;
	/* Shifts */
	case T_LTLT:
		if (nr)
			return 1;
		/* The value to shift by is in r3, the value is stacked */
		load_r_r(R_WORK, 3);
		pop_ac(size);	/* Recover working reg off stack */
		printf("\tor r%u,r%u\n", R_WORK, R_WORK);
		v = ++label_count;
		printf("jr z, X%u\n", v);
		x = label();
		add_r_r(R_AC, R_AC, size);
		djnz_r(R_WORK, x);
		printf("X%u:\n", v);
		return 1;
	case T_GTGT:
		if (nr)
			return 1;
		/* The value to shift by is in r3, the value is stacked */
		load_r_r(R_WORK, 3);
		pop_ac(size);	/* Recover working reg off stack */
		printf("\tor r%u,r%u\n", R_WORK, R_WORK);
		v = ++label_count;
		printf("jr z, X%u\n", v);
		x = label();
		rshift_r(R_AC, size, n->type & UNSIGNED);
		djnz_r(R_WORK, x);
		printf("X%u:\n", v);
		return 1;
	/* Odd mono ops T_TILDE, T_BANG, T_BOOL, T_NEGATE */
	case T_TILDE:
		mono_r(R_AC, size, "com");
		return 1;
	case T_NEGATE:
		mono_r(R_AC, size, "com");
		add_r_const(R_AC, 1, size);
		return 1;
	case T_BOOL:
		if (n->right->flags & ISBOOL)
			return 1;
		/* Until we do cc only */
		cmpne_r_0(R_AC, size);
		n->flags |= ISBOOL;
		return 1;
	case T_BANG:
		/* Until we do cc only. Also if right is bool can do
		   a simple xor */
		if (n->right->flags & ISBOOL)
			printf("\txor r3,#1\n");
		else
			cmpeq_r_0(R_AC, size);
		n->flags |= ISBOOL;
		return 1;
	/* Comparisons T_EQEQ, T_BANGEQ, T_LT. T_LTEQ, T_GT, T_GTEQ */
	/* Use helpers for now */
#if 0	
	case T_EQEQ:
		if (n->type == T_FLOAT)
			return 0;
		pop_compare(size, "nz");
		n->flags |= ISBOOL;
		return 1;
	case T_BANGEQ:
		if (n->type == T_FLOAT)
			return 0;
		pop_compare(size, "z");
		n->flags |= ISBOOL;
		return 1;
	case T_LT:
		if (n->type == T_FLOAT)
			return 0;
		pop_compare(size, u ? "uge" : "ge");
		n->flags |= ISBOOL;
		return 1;
	case T_LTEQ:
		if (n->type == T_FLOAT)
			return 0;
		pop_compare(size, u ? "ugt" : "gt");
		n->flags |= ISBOOL;
		return 1;
	case T_GT:
		if (n->type == T_FLOAT)
			return 0;
		pop_compare(size, u ? "ule" : "le");
		n->flags |= ISBOOL;
		return 1;
	case T_GTEQ:
		if (n->type == T_FLOAT)
			return 0;
		pop_compare(size, u ? "ult" : "lt");
		n->flags |= ISBOOL;
		return 1;
#endif		
	/* Need some kind of similar to the above helper for relatives, but
	   messier so probably best done as helper call. */		
	case T_ANDEQ:
		/* On entry ptr is in ACINT, stack is value to "and" by */
		logic_popeq(size, "and");
		return 1;
	case T_OREQ:
		logic_popeq(size, "or");
		return 1;
	case T_HATEQ:
		logic_popeq(size, "xor");
		return 1;
	/* No hardware multiply/divide
		T_STAR, T_SLASH, T_PERCENT, T_STARTEQ, T_SLASHEQ, T_PERCENTEQ */
	case T_SHLEQ:
		/* Pointer into r14/r15 */
		v = ++label_count;
		pop_rr(R_INDEX);
		/* Save counter */
		load_r_r(R_WORK, R_ACCHAR);
		printf("\tor r%u,r%u\n", R_WORK, R_WORK);
		printf("\tjr z, X%u\n", v);
		/* Value into ac */
		load_r_memr(R_AC, R_INDEX, size);
		x = label();
		add_r_r(R_AC, R_AC, size);
		djnz_r(R_WORK, x);
		revstore_r_memr(R_AC, R_INDEX, size);
		printf("X%u:\n", v);
		return 1;
	case T_SHREQ:
		/* Pointer into r14/r15 */
		v = ++label_count;
		pop_rr(R_INDEX);
		load_r_r(R_WORK, R_ACCHAR);
		printf("\tor r%u,r%u\n", R_WORK, R_WORK);
		printf("\tjr z, X%u\n", v);
		/* Value into ac */
		load_r_memr(R_AC, R_INDEX, size);
		x = label();
		rshift_r(R_AC, size, n->type & UNSIGNED);
		djnz_r(R_WORK, x);
		revstore_r_memr(R_AC, R_INDEX, size);
		printf("X%u:\n", v);
		return 1;
	/* += and -= we can inline except for long size. Only works for non
	   regvar case as written though */
	case T_PLUSEQ:
		if (n->type == FLOAT)
			return 0;
		/* Pointer is on stack, value in ac */
		pop_rr(R_INDEX);
		/* Hardcoded for AC for the moment but not hard to
		   fix */
		x = size;
		add_r_const(R_INDEX, size - 1, 2);
		/* Now points to low byte */
		load_r_memr(R_WORK, R_INDEX, 1);
		rr_decw(R_INDEX);
		printf("\tadd r%u,r%u\n", 3, R_WORK);
		invalidate_ac();
		while(--x) {
			load_r_memr(R_WORK, R_INDEX, 1);
			rr_decw(R_INDEX);
			printf("\tadc r%u,r%u\n", x - 1, R_WORK);
			invalidate_ac();
		}
		/* Result is now in AC, and index points to start of
		   object */
		store_r_memr(R_AC, R_INDEX, size);			
		return 1;
	case T_MINUSEQ:
		if (n->type == FLOAT)
			return 0;
		/* Pointer is on stack, value in ac */
		pop_rr(R_INDEX);
		/* Hardcoded for AC for the moment but not hard to
		   fix */
		x = size;
		add_r_const(R_INDEX, size - 1, 2);
		/* Now points to low byte */
		load_r_memr(R_WORK, R_INDEX, 1);
		rr_decw(R_INDEX);
		printf("\tsub r%u,r%u\n", R_WORK, 3);
		invalidate_ac();
		load_r_r(3, R_WORK);
		while(--x) {
			load_r_memr(R_WORK, R_INDEX, 1);
			rr_decw(R_INDEX);
			printf("\tsbc r%u,r%u\n", R_WORK, x - 1);
			invalidate_ac();
			load_r_r(x - 1, R_WORK);
		}
		/* Result is now in AC, and index points to start of
		   object */
		store_r_memr(R_AC, R_INDEX, size);			
		return 1;
	}
	return 0;
}
