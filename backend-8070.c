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
 *	- Lots of basics to get working yet
 *	- Rewrite x++ and --x forms to use autoindexing
 *	- Implement register tracking
 *	- Peepholes
 *	- Lots of support routines
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

#define ARGBASE		2

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned unreachable;	/* Track unreachable code state */
static unsigned func_cleanup;	/* Zero if we can just ret out */

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
 *	Register tracking (not yet done)
 */
 
void invalidate_ea(void)
{
}

void set_ea(unsigned n)
{
}

void set_a(uint8_t n)
{
}

void set_ea_node(struct node *n)
{
}

void adjust_a(unsigned n)
{
}

void adjust_ea(unsigned n)
{
}

void set_t(unsigned n)
{
}

void invalidate_t(void)
{
}

void flush_writeback(void)
{
}

void invalidate_all(void)
{
}

unsigned free_pointer_nw(unsigned p)
{
	/* Dummy up for now */
	if (p == 3)
		return 2;
	return 3;
}

unsigned free_pointer(void)
{
	return 3;
}

unsigned find_ref(struct node *n, unsigned nw, unsigned offset, int *off)
{
	return 0;
}

void set_ptr_ref(unsigned p, struct node *n)
{
}

/*
 *	Rewriting
 */
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
	if (nt == CSHORT || nt == USHORT || nt == CCHAR || nt == UCHAR || PTR(nt)) {
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

void gen_segment(unsigned s)
{
	switch(s) {
	case A_CODE:
		printf("\t.text\n");
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

	if (size > 10) {
		printf("\tld ea,p1\n\tsub ea,=%d\n\tld p1,ea\n", size);
		return;
	}
	while(size) {
		puts("\tpush ea");
		size -= 2;
	}
}

void gen_cleanup(unsigned size, unsigned save)
{
	if (size > 10 + 2 * save) {
		if (save)
			printf("\tld t,ea\n");
		printf("\tld ea,p1\n\tadd ea,=%d\n\tld p1,ea\n", size);
		if (save)
			printf("\tld ea,t\n");
	} else while(size) {
		puts("\tpop p3");
		size -= 2;
	}
}

/* TODO: no void save / restore EA or return in a P reg or T .. decisions */
void gen_epilogue(unsigned size, unsigned argsize)
{
	unsigned x = func_flags & F_VOIDRET;
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
	printf("\tld p3, Sw%d\n", n);
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

void gen_helpcall(struct node *n)
{
	flush_writeback();
	invalidate_all();
	printf("\tjsr __");
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
		puts("\tld t,ea\n\tld ea,@__high\n\tpush ea\n\tld ea,t\n\tpush ea");
		break;
	default:
		return 0;
	}
	return 1;
}


unsigned load_ptr_ea(void)
{
	/* FIXME: set ptr up according to ea name value in future */
	unsigned ptr = free_pointer_nw(0);
	printf("\tld p%d, ea\n", ptr);
	/* TODO: tracking */
	invalidate_ea();
	return ptr;
}

void load_ea(unsigned sz, unsigned long v)
{
	if (sz == 1) {
		printf("\tld a,=%ld\n", v & 0xFF);
		set_a(v & 0xFF);
	}
	else if (sz == 2) {
		printf("\tld ea,=%ld\n", v & 0xFFFF);
		set_ea(v & 0xFFFF);
	} else {
		printf("\tld ea,=%ld\n", v >> 16);
		puts("\tst ea,@__high");
		printf("\tld ea,=%ld\n", v & 0xFFFF);
		set_ea(v & 0xFFFF);
	}
}

void load_t(unsigned v)
{
	printf("\tld t,=%d\n", v & 0xFFFF);
	set_t(v & 0xFFFF);
}


void repeated_op(const char *op, unsigned n)
{
	while(n--)
		puts(op);
}

void discard_word(void)
{
	unsigned ptr = free_pointer();
	printf("\tpop p%d\n", ptr);
}

/* FIXME: pass sz */
/*
 *	Make a reference to an object. We are passed the object node
 *	an optional 1, 2 or 3 to indicate a pointer to avoid, and an
 *	optional offset return.
 */
unsigned gen_ref_nw(struct node *n, unsigned nw, unsigned offset, int *off)
{
	unsigned sz = get_size(n->type);
	unsigned ptr;
	unsigned v = n->value;

	if (off)
		*off = 0;

	v += offset;

	if (n->op == T_LREF || n->op == T_LSTORE) {
		int r = v + sp;	/* CHECK */
		/* Slightly pessimal for word ops */
		if (nw != 1 && r >= -128 && r <= 128 - sz && off) {
			*off = r;
			/* Dereference directly from SP */
			return 1;
		}
		/* Need to generate a ref. TODO pass whether EA can be mushed */
		ptr = free_pointer_nw(nw);
		printf("\txch ea,p%d\n", ptr);
		puts("\tld ea,p1");
		printf("\tadd ea,%d\n", r);
		printf("\txch ea,p%d\n", ptr);
		return ptr;
	}
	/* See if it is already accessible, often the case */
	ptr = find_ref(n, nw, 0, off);
	if (ptr)
		return ptr;
	/* Make a reference */
	ptr = free_pointer_nw(nw);
	set_ptr_ref(ptr, n);
	if (n->op == T_NREF || n->op == T_NSTORE) { 
		printf("\tld p%d,=_%s+%d\n", ptr, namestr(n->snum), v);
		return ptr;
	}
	if (n->op == T_LBREF || n->op == T_LBSTORE) {
		printf("\tld p%d,=T%d+%d\n", ptr, n->val2, v);
		return ptr;
	}
	return 0;
}

/* For the moment just do the simple case for testing */
unsigned gen_addr_ref(struct node *n, int *off, int offset)
{
	unsigned size = get_size(n->type);
	/*unsigned ptr; */
	unsigned v = n->value;
	/* TODO: support notwith ? T_NAME, T_LABEL */
	if (n->op == T_LOCAL) {
		int r = v + sp;
		if (r >= -128 && r <= 128 - size && off) {
			*off = r;
			return 1;
		}
		return 0;
	}
	return 0;
}

unsigned gen_load_nw(struct node *n, unsigned nw, int offset)
{
	int off;
	unsigned sz = get_size(n->type);
	unsigned ptr;
	if (n->op == T_CONSTANT) {
		load_ea(sz, n->value);
		return 1;
	}
	ptr = gen_ref_nw(n, nw, offset, &off);
	if (ptr == 0)
		return 0;
	if (sz == 1)
		printf("\tld a,%d,p%d\n", off, ptr);
	else if (sz == 2)
		printf("\tld ea,%d,p%d\n", off, ptr);
	else {
		printf("\tld ea,%d,p%d\n", off + 2, ptr);
		puts("\tst ea,@__high");
		printf("\tld ea,%d,p%d\n", off, ptr);
	}
	set_ea_node(n);
	return 1;
}

unsigned gen_load(struct node *n)
{
	return gen_load_nw(n, 0, 0);
}

unsigned gen_load_nw_t(struct node *n, unsigned nw, unsigned offset)
{
	int off;
	unsigned sz = get_size(n->type);
	unsigned ptr;
	if (n->op == T_CONSTANT) {
		load_t(n->value);
		return 1;
	}
	ptr = gen_ref_nw(n, nw, offset, &off);

	if (ptr == 0)
		return 0;

	/* We have to ref an extra byte */
	if (sz <= 2)
		printf("\tld t,%d,p%d\n", off, ptr);
	else	
		error("loadt4");
	return 1;
}

unsigned gen_load_t(struct node *n)
{
	return gen_load_nw_t(n, 0, 0);
}

unsigned gen_ref(struct node *n, int *off)
{
	return gen_ref_nw(n, 0, 0, off);
}

/* Only 8 and 16 bit as 32 is complicated by the lack of adc/sbc stuff */
static unsigned gen_op(unsigned sz, const char *op, struct node *r)
{
	unsigned ptr;
	int off;
	unsigned v = r->value;
	if (r->op == T_CONSTANT) {
		/* TODO: op specific set EA value */
		invalidate_ea();
		if (sz == 1)  {
			printf("\t%s a,=%d\n", op, v & 0xFF);
		} else {
			printf("\t%s ea,=%d\n", op, v & 0xFFFF);
		}
		return 1;
	}
	ptr = gen_ref(r, &off);
	if (ptr == 0)
		return 0;
	invalidate_ea();
	if (sz == 1)
		printf("\t%s a,%d,p%d\n", op, off, ptr);
	else
		printf("\t%s ea,%d,p%d\n", op, off, ptr);
	return 1;
}

/* Ditto for operaions that only have byte forms */
static unsigned gen_op8(unsigned sz, const char *op, struct node *r)
{
	unsigned ptr;
	int off;
	unsigned v = r->value;
	invalidate_ea();
	if (r->op == T_CONSTANT) {
		printf("\t%s a,=%d\n", op, v & 0xFF);
		if (sz == 2) {
			printf("\txch a,e\n");
			printf("\t%s a,=%d\n", op, v >> 8);
			printf("\txch a,e\n");
		}
		return 1;
	}
	ptr = gen_ref(r, &off);
	if (ptr == 0)
		return 0;
	if (sz == 1)
		printf("\t%s a,%d,p%d\n", op, off, ptr);
	else
		printf("\t%s ea,%d,p%d\n", op, off, ptr);
	return 1;
}

/* Generate a JSR to one of the ,T helpers */
static unsigned gen_t_op(unsigned sz, struct node *n, const char *fn)
{
	flush_writeback();
	invalidate_all();
	printf("\tjsr __%s%c", fn, "XbwXl"[sz]);
	if (n->type & UNSIGNED)
		putchar('u');
	putchar('\n');
	return 1;
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
	// TODO shifts

	invalidate_ea();
	if (sz == 1) {
		load_t(value);
		puts("\tmpy");
		return 1;
	}
	if (sz > 2)
		return 0;
	/* Constant on right is positive - can use mpy */
	if (!(value & 0x8000)) {
		load_t(value);
		puts("\tmpy");
		return 1;
	}
	/* Can't shift to avoid problem - use helper */
	if (value & 1)
		return 0;
	/* Shift to keep right side positive */
	puts("\tsl ea\n");
	load_t(value >> 1);
	puts("\tmpy");
	return 0;	
}

/* Start simple: We can in fact do a lot of locals etc */
static unsigned access_direct(struct node *n)
{
	if (get_size(n->type) > 2)
		return 0;
	switch(n->op) {
	case T_CONSTANT:
	case T_NAME:
	case T_LABEL:
		return 1;
	/* Possible but not always doable as it stands. TODO go via T
	   for bigger offsets */
	case T_LOCAL:
	case T_ARGUMENT:
	case T_LREF:
		return 0;
	}
	return 0;
}

/*
 *	Perform an operation directly on n. Must succeed if access_direct
 *	said it could. May fail if not. Existing value is in EA, result ends
 *	up in EA. May trash T or a pointer.
 */
static unsigned op_direct(struct node *n, const char *op, unsigned s)
{
	const char *name;
	unsigned v = n->value;

	switch(n->type) {
	case T_CONSTANT:
		invalidate_ea();
		if (s == 1)
			printf("\t%s a,=%u\n", op, v & 0xFF);
		else
			printf("\t%s ea,=%u\n", op, v & 0xFFFF);
		return 1;
	case T_NAME:
		invalidate_ea();
		name = namestr(n->snum);
		if (s == 1)
			printf("\t%s a,=<%s+%u\n", op, name, v);
		else 
			printf("\t%s, ea,=%s+%u\n", op, name, v);
		return 1;
	case T_LABEL:
		invalidate_ea();
		if (s == 1)
			printf("\t%s a,=<T%u+%u\n", op, n->val2, v);
		else
			printf("\t%s ea,=T%u+%u\n", op, n->val2, v);
		return 1;
	case T_LREF:
		/* Not always possible */
		if (v + sp > 128 - s) {
			/* TODO: swap via T ? */
			return 0;
		}
		invalidate_ea();
		if (s == 1)
			printf("\t%s a,%u,p1\n", op, v + sp);
		else
			printf("\t%s ea,%u,p1\n", op, v + sp);
		return 1;
	}
	return 0;
}

static unsigned op_direct8(struct node *n, const char *op, unsigned s)
{
	const char *name;
	unsigned v = n->value;

	switch(n->type) {
	case T_CONSTANT:
		invalidate_ea();
		printf("\t%s a,=%u\n", op, v & 0xFF);
		if (s == 2) {
			printf("\txch a,e\n");
			printf("\t%s a,=%u\n", op, v >> 8);
			printf("\txch a,e\n");
		}
		return 1;
	case T_NAME:
		invalidate_ea();
		name = namestr(n->snum);
		printf("\t%s a,=<%s+%u\n", op, name, v);
		if (s == 2) {
			printf("\txch a,e\n");
			printf("\t%s, a,=>%s+%u\n", op, name, v);
			printf("\txch a,e\n");
		}
		return 1;
	case T_LABEL:
		invalidate_ea();
		printf("\t%s a,=<T%u+%u\n", op, n->val2, v);
		if (s == 2) {
			printf("\txch a,e\n");
			printf("\t%s ea,=>T%u+%u\n", op, n->val2, v);
			printf("\txch a,e\n");
		}
		return 1;
	case T_LREF:
		/* Not always possible */
		if (v + sp > 128 - s) {
			/* TODO: swap via T ? */
			return 0;
		}
		invalidate_ea();
		printf("\t%s a,%u,p1\n", op, v + sp);
		if (s == 2) {
			printf("\txch a,e\n");
			printf("\t%s ea,%u,p1\n", op, v + sp + 1);
			printf("\txch a,e\n");
		}
		return 1;
	}
	return 0;
}

/* EA holds the left side (ptr) r is the value to handle. Do not use
   T as T is used by caller in postinc/dec usage
   
   Needs work to use ILD/DLD and postinc/predec auto index forms */
unsigned do_preincdec(unsigned sz, struct node *n, unsigned save)
{
	struct node *r = n->right;
	unsigned ptr = load_ptr_ea();
	int nv;

	/* For now at least */
	if (sz > 2)
		return 0;

	/* Rewrite constant forms positive */
	if (n->op == T_MINUSEQ) {
		n->op = T_PLUSEQ;
		r->value = -r->value;
	}
	if (n->op == T_MINUSMINUS) {
		n->op = T_PLUSPLUS;
		r->value = -r->value;
	}
	if (sz == 1 && n->op == T_PLUSEQ) {
		nv = (int)r->value;
		if (nv > 0 && nv < 3 + opt) {
			while(nv--)
				printf("\tild a,0,p%u\n", ptr);
			return 1;
		}
		nv = -nv;
		if (nv > 0 && nv < 3 + opt) {
			while(nv--)
				printf("\tild a,0,p%u\n", ptr);
			return 1;
		}
	}

	gen_load_nw(r, ptr, 0);

	if (save)
		printf("\tld t,0,p%u\n", ptr);
	/* A or EA now holds the data */
	/* Now add to 0,ptr */	
	if (sz == 1)
		printf("\tadd a,0,p%d\n\tst a,0,p%d\n",
			ptr, ptr);
	else {
		printf("\tadd ea,0,p%d\n\tst ea,0,p%d\n",
			ptr, ptr);
		set_ea_node(r);
	}
	if (save) {
		printf("\txch ea,t\n");
		invalidate_ea();
	}
	return 1;
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
	unsigned ptr, ptr2;
	int off;
	char *op;
	if (r)
		v = r->value;

	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		/* Need to decide who cleans up non vararg calls */
		printf(";cleanup %u sp was %u now %u\n",
			v, sp, sp - v);
		sp -= v;
		/* TODO: , 0 if called fn was void or result nr */
		gen_cleanup(v, 1);
		return 1;
	case T_NSTORE:
	case T_LBSTORE:
		if (s <= 2 && gen_op(s, "st", r)) {
			set_ea_node(r);
			return 1;
		}
		/* TODO: can do 4 sanely */
		break;
	case T_EQ:
		/* A bit more complex than this but prob needs rewrite rules */
		if (!access_direct(r))
			return 0;
		n->value = 0;
	case T_EQPLUS:
		/* EQPLUS rewrite rule is responsible for making sure this is always possible */
		/* Nothing to do with writing back yet but this is a write
		   to an unknown object so we must kill any possible aliases */
		flush_writeback();
		/* Store right hand op in EA */
		ptr = load_ptr_ea();	/* Turn EA into a pointer */
		/* Generate the load without using that ptr */
		gen_load_nw(r, ptr, n->val2);
		/* EA now holds the data */
		if (s == 4) {
			if (!(n->flags & NORETURN))
				printf("\tld t,ea");
			printf("\tst ea,%u,p%d\n", v, ptr);
			printf("\tld ea,@__high\n\tst ea,%u,p%d\n", v + 2, ptr);
			if (!(n->flags & NORETURN))
				printf("\tld ea,t\n");
		}
		if (s == 2)
			printf("\tst ea,%u,p%d\n", v, ptr);
		else
			printf("\tst a,%u,p%d\n", v, ptr);
		set_ea_node(r);
		return 1;
	case T_PLUS:
		/* TODO: can inline long I think */
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (s == 1) { 
				printf("\tadd a,=%d\n", v & 0xFF);
				adjust_a(v);
			} else {
				printf("\tadd ea,=%d\n", v);
				adjust_ea(v);
			}
			return 1;
		}
		if (op_direct(r, "add", s))
			return 1;
		invalidate_ea();
		return gen_op(s, "add", r);
	case T_MINUS:
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (s == 1) {
				printf("\tsub a,=%d\n", v & 0xFF);
				adjust_a(v);
			} else { 
				printf("\tsub ea,=%d\n", v);
				adjust_ea(v);
			}
			return 1;
		}
		if (op_direct(r, "sub", s))
			return 1;
		invalidate_ea();
		return gen_op(s, "sub", r);
	case T_STAR:	/* Multiply is a complicated mess */
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			/* Try shifts and MUL op inlined */
			if (s <= 2 && gen_fast_mul(s, v))
				return 1;
		}
		puts("\tld t,ea");
		gen_load(r);
		puts("\tmpy\n");	/* Valid for 16bit as we use it */
		puts("\tld ea,t");
		invalidate_ea();
		break;
	case T_SLASH:
		if (s > 2)
			return 0;
#if 0			
		if (r->op == T_CONSTANT) {
			if (gen_fast_div(s, r))
				return 1;
		}
#endif		
		invalidate_ea();
		puts("\tld t,ea");
		gen_load(r);
		if (n->type & UNSIGNED)
			puts("\tjsr __udiv1616");
		else
			puts("\tjsr __div1616");
		break;
	/* Should do T_PERCENT fast remainder cases */
	case T_AND:
		if (r->op == T_CONSTANT && s <= 2) {
			if (s == 2) {
				if ((v & 0xFF00) == 0x0000)
					printf("\tld e,=0\n");
				else if ((v & 0xFF00) != 0xFF00)
					printf("\txch e,a\n\tand a,=%u\n\txch e,a\n", v >> 8);
			}
			if ((v & 0xFF) == 0x00)
				printf("\tld a,=0\n");
			else if ((v & 0xFF) != 0xFF)
				printf("\tand a,=%u\n", v);
			return 1;
		}
		if (!op_direct8(r, "and", s))
			return gen_op8(s, "and", r);
		return 1;
	case T_OR:
		if (r->op == T_CONSTANT && s <= 2) {
			if (s == 2) {
				if ((v & 0xFF00) == 0xFF00)
					printf("\tld e,=0xFF\n");
				else if ((v & 0xFF00) != 0xFF00)
					printf("\txch e,a\n\tor a,=%u\n\txch e,a\n", v >> 8);
			}
			if ((v & 0xFF) == 0xFF)
				printf("\tld a,=0xFF\n");
			else if ((v & 0xFF) != 0xFF)
				printf("\tor a,=%u\n", v);
			return 1;
		}
		if (!op_direct8(r, "or", s))
			return gen_op8(s, "or", r);
		return 1;
	case T_HAT:
		if (r->op == T_CONSTANT && s <= 2) {
			if (s == 2) {
				if (v & 0xFF00)
					printf("\txch e,a\n\txor a,=%u\n\txch e,a\n", v >> 8);
			}
			if (v & 0xFF)
				printf("\txor a,=%u\n", v);
			return 1;
		}
		if (!op_direct8(r, "xor", s))
			return gen_op8(s, "xor", r);
		return 1;
	case T_EQEQ:
		if (!op_direct(r, "sub", s))
			return 0;
		/* Usual cases we can just call out to sort out the logic
		   checks, and eventually USECC this target */
		return gen_t_op(s, n, "ceqeq");
	case T_GTEQ:
		if (!op_direct(r, "sub", s))
			return 0;
		/* Usual cases we can just call out to sort out the logic
		   checks, and eventually USECC this target */
		return gen_t_op(s, n, "cgteq");
	case T_GT:
		if (!op_direct(r, "sub", s))
			return 0;
		/* Usual cases we can just call out to sort out the logic
		   checks, and eventually USECC this target */
		return gen_t_op(s, n, "cgt");
	case T_LTEQ:
		if (!op_direct(r, "sub", s))
			return 0;
		/* Usual cases we can just call out to sort out the logic
		   checks, and eventually USECC this target */
		return gen_t_op(s, n, "clteq");
	case T_LT:
		if (!op_direct(r, "sub", s))
			return 0;
		/* Usual cases we can just call out to sort out the logic
		   checks, and eventually USECC this target */
		return gen_t_op(s, n, "clt");
	case T_BANGEQ:
		if (!op_direct(r, "sub", s))
			return 0;
		/* Usual cases we can just call out to sort out the logic
		   checks, and eventually USECC this target */
		return gen_t_op(s, n, "cbangeq");
	case T_LTLT:
		if (s > 2)
			return 0;
		/* TODO track shift result */
		invalidate_ea();
		if (r->op == T_CONSTANT) {
			if (v > 15)
				return 1;
			if (v >= 8) {
				if (s == 1)
					return 1;
				v -= 8;
				puts("\tld e,a\n\tld a,=0");
			}
			if (s == 1)
				repeated_op("\tsl a", v);
			else
				repeated_op("\tsl ea", v);
			return 1;
		}
		/* Helper time */
		if (gen_load_t(r))
			return gen_t_op(s, n, "shl_t");
		break;
	case T_GTGT:
		if (s > 2)
			return 0;
		/* TODO track shift result */
		invalidate_ea();
		if ((n->type & UNSIGNED) && s<= 2 && r->op == T_CONSTANT) {
			if (v > 15)
				return 1;
			if (v >= 8) {
				if (s == 1)
					return 1;
				v -= 8;
				puts("\tld a,e\n\tld e,=0\n");
			}
			if (s == 1)
				repeated_op("\tsr a", v);
			else
				repeated_op("\tsr ea", v);
			return 1;
		}
		if (gen_load_t(r))
			return gen_t_op(s, n, "shr_t");
		break;
	case T_PLUSPLUS:
	case T_MINUSMINUS:
		/* Need to look at being smarter here if we know what
		   ea points to */
		flush_writeback();
		return do_preincdec(s, n, 1);
	case T_PLUSEQ:
	case T_MINUSEQ:
		return do_preincdec(s, n, 0);
	case T_ANDEQ:
		op = "and";
		goto doeleq;
	case T_OREQ:
		op = "or";
		goto doeleq;
	case T_HATEQ:
		op = "xor";	
	doeleq:
		/* EA holds the pointer */
		ptr = load_ptr_ea();
		if (s> 2)
			return 0;
		if (s == 2) {
			if (r->op == T_CONSTANT)
				printf("\tld ea,=%u\n", v);
			else {
				ptr2 = gen_ref_nw(r, ptr, 0, &off);
				if (ptr2 == 0)
					return 0;
				printf("\tld ea,%u,%u\n", off, ptr2);
			}
		} else {
			if (r->op == T_CONSTANT)
				printf("\tld a,=%u\n", v);
			else {
				ptr2 = gen_ref_nw(r, ptr, 0, &off);
				if (ptr2 == 0)
					return 0;
				printf("\tld a,%u,%u\n", off, ptr2);
			}
		}
		printf("\t%s a,0,%u\n", op, ptr);
		if (s == 2) {
			printf("\txch a,e\n");
			printf("\t%s a,1,%u\n", op, ptr);
			printf("\txch a,e\n");
		}
		if (s == 2)
			printf("\tst ea,0,%u\an", ptr);
		else 
			printf("\tst a,0,%u\an", ptr);
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
	unsigned ptr;
	int off;
	unsigned noret = n->flags & NORETURN;

	if (unreachable)
		return 1;

	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	if (n->op == T_COMMA) {
		n->left->flags |= NORETURN;
		codegen_lr(n->left);
		codegen_lr(n->right);
		return 1;
	}
	if ((n->op == T_NSTORE || n->op == T_LSTORE) && s < 2) {
		codegen_lr(n->right);
		/* Result is now in EA */
		ptr = gen_ref(n, &off);
		if (ptr == 0)
			return 0;
		/* Ptr loaded */
		if (s == 2)
			printf("\tst ea,%d,p%d\n", off, ptr);
		else
			printf("\tst a,%d,p%da\n", off, ptr);
		set_ea_node(n);
		return 1;
	}
	/* PLUSPLUS is preinc and always constant */
	if (n->op == T_PLUSPLUS) {
		if (s != 2)
			return 0;
		/* Try and avoid getting the pointer into EA */
		ptr = gen_addr_ref(l, &off, 0);
		if (ptr == 0)
			return 0;
		/* Ok we can construct a pointer to the left */
		if (s == 2) {
			printf("\tld ea,%d,p%u\n", off, ptr);
			if (!noret)
				printf("\tld t,ea\n");
			printf("\tadd ea,=%u\n\tst ea,%d,p%u\n", WORD(r->value), off, ptr);
			if (!noret)
				printf("\tld ea,t\n");
			return 1;
		}
		/* TODO size 1 and also use ILD/DLD for it */
	}
	/* MINUSMINUS is preinc and always constant */
	if (n->op == T_MINUSMINUS) {
		if (s != 2)
			return 0;
		/* Try and avoid getting the pointer into EA */
		ptr = gen_addr_ref(l, &off, 0);
		if (ptr == 0)
			return 0;
		/* Ok we can construct a pointer to the left */
		if (s == 2) {
			printf("\tld ea,%d,p%u\n", off, ptr);
			if (!noret)
				printf("\tld t,ea\n");
			printf("\tsub ea,=%u\n\tst ea,%d,p%u\n", WORD(r->value), off, ptr);
			if (!noret)
				printf("\tld ea,t\n");
			return 1;
		}
		/* TODO size 1 and also use ILD/DLD for it */
	}
	/* TODO: size 1 stuff. Also we need a can_gen_ref() so we can do the non const cases by generating the
	   right and trying to gen a ref to the left. In almost call cases this will work as C rarely uses
	   (complex expr) += (complex expr) */
	if (n->op == T_PLUSEQ) {
		printf(";pluseq\n");
		if (s != 2)
			return 0;
		if (r->op == T_CONSTANT) {
			ptr = gen_addr_ref(l, &off, 0);
			if (ptr == 0)
				return 0;
			/* Ok we can construct a pointer to the left */
			if (s == 2) {
				printf("\tld ea,%d,p%u\n", off, ptr);
				printf("\tadd ea,=%u\n\tst ea,%d,p%u\n", WORD(r->value), off, ptr);
			}
			return 1;
		}
	}
	if (n->op == T_MINUSEQ) {
		if (s != 2)
			return 0;
		if (r->op == T_CONSTANT) {
			ptr = gen_addr_ref(l, &off, 0);
			if (ptr == 0)
				return 0;
			/* Ok we can construct a pointer to the left */
			if (s == 2) {
				printf("\tld ea,%d,p%u\n", off, ptr);
				printf("\tsub ea,=%u\n\tst ea,%d,p%u\n", WORD(r->value), off, ptr);
			}
			return 1;
		}
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

	ls = get_size(lt);

	/* Size shrink is free */
	if ((lt & ~UNSIGNED) <= (rt & ~UNSIGNED))
		return 1;
	/* Don't do the harder ones */
	if (!(rt & UNSIGNED) || ls > 2)
		return 0;
	puts("\tld e,0");
	/* FIXME: really turn EA to A */
	invalidate_ea();
	return 1;
}

static unsigned pop_t_op(struct node *n, const char *op, unsigned sz)
{
	if (sz > 2)
		return 0;
	puts("\txch ea,t\n\tpop ea");
	sp -= sz;
	return gen_t_op(sz, n, op);
}

static unsigned pop_ptr(void)
{
	unsigned ptr = free_pointer();
	printf("\tpop p%d\n", ptr);
	sp -= 2;
	return ptr;
}

static unsigned logic_sp_op(struct node *n, const char *op)
{
	unsigned sz = get_size(n->type);
	invalidate_ea();
	if (sz == 1) {
		printf("\t%s a,0,p1\n", op);
		invalidate_ea();
		discard_word();
		return 1;
	}
	if (sz == 2) {
		printf("\t%s a,0,p1\n", op);
		printf("\txch a,e\n\t%s a,1,p1\n\txch a,e\n", op);
		invalidate_ea();
		discard_word();
		return 1;
	}
	/* For now don't inline dword ops */
	return 0;
}

static unsigned logic_ptr_op(unsigned ptr, const char *op, unsigned sz)
{
	/* *ptr op EA */
	if (sz == 1) {
		printf("\txch a,e\n\tld a,0,p%u\n\t%s a,e\n", ptr, op);
		invalidate_ea();
		return 1;
	}
	if (sz == 2) {
		printf("\tst ea,@__tmp\n\tld ea,0,p%u\n", ptr);
		printf("\t%s a,@__tmp\n", op);
		printf("\txch a,e\n\t%s a,@__tmp+1\n\txch a,e\n", op);
		invalidate_ea();
		return 1;
	}
	/* Punt on long */
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned sz = get_size(n->type);
	unsigned v;
	unsigned ptr;
	int off;
	unsigned noret = n->flags & NORETURN;

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
		/* Kill unused local ref */
		if (noret)
			return 1;
		/* TODO: once volatile is cleaned up a bit more kill these
		   too */
	case T_NREF:
	case T_LBREF:
		ptr = gen_ref(n, &off);
		if (ptr == 0)
			return 0;
		if (sz == 1)
			printf("\tld a,%d,p%d\n", off, ptr);
		if (sz == 2)
			printf("\tld ea,%d,p%d\n", off, ptr);
		if (sz == 4) {
			printf("\tld ea,%d+2,p%d\n",off, ptr);
			puts("\tst ea,@__high");
			printf("\tld ea,%d,p%d\n", off, ptr);
		}
		set_ea_node(n);
		return 1;
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
		ptr = gen_ref(n, &off);
		if (ptr == 0)
			return 0;
		if (sz == 1)
			printf("\tst a,%d,p%d\n", off, ptr);
		if (sz == 2)
			printf("\tst ea,%d,p%d\n", off, ptr);
		if (sz == 4) {
			puts("\tld t,ea");
			puts("\tld ea,@__high");
			printf("\tst ea,%d+2,p%d\n",off, ptr);
			puts("\tld ea,t");
			printf("\tst ea,%d,p%d\n", off, ptr);
		}
		set_ea_node(n);
		return 1;
	case T_CALLNAME:
		flush_writeback();
		invalidate_all();
		printf("\tcall _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_EQ:
		/* *EAX = TOS */
		ptr = load_ptr_ea();
		puts("\tpop ea\n");
		invalidate_ea();
		flush_writeback();
		if (sz == 1)
			printf("\tst a,0,p%d\n", ptr);
		else {
			printf("\tst ea,0,p%d\n", ptr);
			if (sz == 4) {
				if (!noret)
					puts("\tld t,ea\n");
				puts("\tpop ea\n");
				printf("\tst ea,2,p%d\n", ptr);
				if (!noret) {
					puts("\tst ea,@__high");
					puts("\tld ea,t\n");
				}
			}
		}
		return 1;
	case T_DEREF:
	case T_DEREFPLUS:
		/* Might be able to be smarter here */
		flush_writeback();
		/* Could noret away once volatile cleaned */
		/* EAX = *EAX */
		ptr = load_ptr_ea();
		if (sz == 1)
			printf("\tld a,%u,p%d\n", v, ptr);
		else if (sz == 2)
			printf("\tld ea,%u,p%d\n", v, ptr);
		else {
			printf("\tld ea,%u,p%d\n", v + 2, ptr);
			puts("\tst ea,@__high");
			printf("\tld ea,%u,p%d\n", v, ptr);
		}
		return 1;
	case T_LDEREF:
		/* val2 offset of variable, val offset of ptr */
		ptr = free_pointer();
		printf("\tld p%u,%u,p1\n", ptr, n->val2);
		if (sz == 1)
			printf("\tld a,%u,p%u\n", v, ptr);
		else if (sz == 2)
			printf("\tld ea,%u,p%u\n", v, ptr);
		else {
			printf("\tld ea,%u,p%u\n", v + 2, ptr);
			printf("\tst ea,@__high\n");
			printf("\tld ea,%u,p%u\n", v, ptr);
		}
		return 1;
	case T_LEQ:
		/* Same idea for writing */
		ptr = free_pointer();
		printf("\tld p%u,%u,p1\n", ptr, n->val2);
		if (sz == 1)
			printf("\tst a,%u,p%u\n", v, ptr);
		else if (sz == 2)
			printf("\tst ea,%u,p%u\n", v, ptr);
		else {
			if (!noret)
				printf("\tld t,ea\n");
			printf("\tst ea,%u,p%u\n", v, ptr);
			printf("\tld ea,@__high\n");
			printf("\tst ea,%u,p%u\n", v + 2, ptr);
			if (!noret)
				printf("\tld ea,t\n");
		}
		return 1;
	case T_FUNCCALL:
		flush_writeback();
		invalidate_all();
		/* EA holds the function ptr */
		puts("\tjsr __callea\n");
		return 1;
	case T_LABEL:
		printf("\tld ea,T%d+%d\n", n->val2, v);
		set_ea_node(n);
		return 1;
	case T_CONSTANT:
		if (sz == 4) {
			printf("\tld ea,=%d\n", v >> 16);
			puts("\tst ea,@__high");
		}
		if (sz > 1) {
			printf("\tld ea,=%d\n", v & 0xFFFF);
			set_ea(v & 0xFFFF);
		} else	{
			printf("\tld a,=%d\n", v & 0xFF);
			set_a(v & 0xFF);
		}
		return 1;
	case T_NAME:
		printf("\tld ea,=_%s+%d\n", namestr(n->snum), v);
		set_ea_node(n);
		return 1;
	case T_ARGUMENT:
		v += frame_len + ARGBASE;
	case T_LOCAL:
		v += sp;
		puts("\tld ea,p1");
		if (v)
			printf("\tadd ea,=%d\n", v);
		set_ea_node(n);
		return 1;
	case T_CAST:
		return gen_cast(n);
	case T_PLUS:
		invalidate_ea();
		if (sz == 1) {
			printf("\tadd a,0,p1\n");
			discard_word();
			return 1;
		}
		if (sz == 2) {
			printf("\tadd ea,0,p1\n");
			discard_word();
			return 1;
		}
		break;
#if 0
	/* Needs to change to be TOS - EA */
	case T_MINUS:
		invalidate_ea();
		if (sz == 1)
			printf("\tsub a,0,p1\n");
		if (sz == 2)
			printf("\tsub ea,0,p1\n");
		discard_word();
		return 1;
#endif
		return pop_t_op(n, "sub_t", sz);		
	case T_STAR:
		return pop_t_op(n, "mul_t", sz);
	case T_SLASH:
		return pop_t_op(n, "div_t", sz);
	case T_AND:
		/* EA &= TOS */
		return logic_sp_op(n, "and");
	case T_OR:
		return logic_sp_op(n, "or");
	case T_HAT:
		return logic_sp_op(n, "xor");
	/* Shift TOS by EA */
	case T_LTLT:
 		return pop_t_op(n, "sl_t", sz);
	case T_GTGT:
 		return pop_t_op(n, "sr_t", sz);
	/* TODO Comparisons, EQ ops */
	case T_PLUSEQ:
		flush_writeback();
		invalidate_ea();
		/* EA holds the value, TOS the ptr */
		if (sz > 2)
			return 0;
		ptr = pop_ptr();
		if (sz == 1) {
			printf("\tadd a,0,p%d", ptr);
			printf("\tst a,0,p%d\n", ptr);
		} else if (sz == 2) {
			printf("\tadd ea,0,p%d\n", ptr);
			printf("\tst ea,0,p%d\n", ptr);
		}
		return 1;
	case T_MINUSEQ:
		flush_writeback();
		invalidate_ea();
		/* Awkward as it is *TOS - EA */
		if (sz == 1) {
			puts("\txch a,e");
			ptr = pop_ptr();
			printf("\tld a,0,p%d\n", ptr);
			puts("\tsub a,e");
			printf("\tst a,0,p%d\n", ptr);
			return 1;
		}
		if (sz == 2) {
			puts("\tst ea,@__tmp");
			ptr = pop_ptr();
			printf("\tld ea,p%u\n", ptr);
			puts("\tsub ea,@__tmp");
			printf("\tst ea,0,p%d\n", ptr);
		}
		return 1;
	/* TOS is the pointer, EA is the value */
	case T_ANDEQ:
		ptr = pop_ptr();
		return logic_ptr_op(ptr, "and", sz);
	case T_OREQ:
		ptr = pop_ptr();
		return logic_ptr_op(ptr, "or", sz);
	case T_HATEQ:
		ptr = pop_ptr();
		return logic_ptr_op(ptr, "xor", sz);
	}
	return 0;
}
