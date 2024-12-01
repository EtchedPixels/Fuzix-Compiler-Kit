/*
 *	The gameboy is an 8080 missing a few bits (some useful) and with
 *	Z80isms and some random other stuff thrown in.
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

#define ARGBASE	2	/* Bytes between arguments and locals if no reg saves */

/* Check if a single bit is set or clear */
int bitcheckb1(uint8_t n)
{
	unsigned m = 1;
	unsigned i;

	for (i = 0; i < 8; i++) {
		if (n == m)
			return i;
		m <<= 1;
	}
	return -1;
}

int bitcheck1(unsigned n, unsigned s)
{
	register unsigned i;
	unsigned m = 1;

	if (s == 1)
		return bitcheckb1(n);
	for (i = 0; i < 16; i++) {
		if (n == m)
			return i;
		m <<= 1;
	}
	return -1;
}

int bitcheck0(unsigned n, unsigned s)
{
	if (s == 1)
		return bitcheckb1((~n) & 0xFF);
	return bitcheck1((~n) & 0xFFFF, 2);
}

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned argbase;	/* Argument offset in current function */
static unsigned unreachable;	/* Code following an unconditional jump */
static unsigned func_cleanup;	/* Zero if we can just ret out */
static unsigned label;		/* Used to hand out local labels in the form X%u */
static unsigned ccvalid;	/* State of condition codes */
#define CC_UNDEF	0	/* Who knows */
#define CC_VALID	1	/* Matches (Z/NZ) */
#define CC_INVERSE	2	/* Matches the inverse (Z NZ) */

/* Set CC correctly */
static void outputcc(const char *p, ...)
{
	va_list v;
	if (strchr(p, ':') == NULL)
		putchar('\t');
	va_start(v, p);
	vprintf(p, v);
	putchar('\n');
	va_end(v);
	ccvalid = CC_VALID;
}

/* CC other */
static void output(const char *p, ...)
{
	va_list v;
	if (strchr(p, ':') == NULL)
		putchar('\t');
	va_start(v, p);
	vprintf(p, v);
	putchar('\n');
	va_end(v);
	ccvalid = CC_UNDEF;
}

/* CC no effect */
static void outputne(const char *p, ...)
{
	va_list v;
	if (strchr(p, ':') == NULL)
		putchar('\t');
	va_start(v, p);
	vprintf(p, v);
	putchar('\n');
	va_end(v);
}

/* CC inverted but valid */
static void outputinv(const char *p, ...)
{
	va_list v;
	if (strchr(p, ':') == NULL)
		putchar('\t');
	va_start(v, p);
	vprintf(p, v);
	putchar('\n');
	va_end(v);
	ccvalid = CC_INVERSE;
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

/* We only have PUSH AF not 8bit push */
static unsigned get_stack_size(unsigned t)
{
	unsigned n = get_size(t);
	if (n == 1)
		return 2;
	return n;
}

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
#define T_BTST		(T_USER+11)		/* single bit testing */

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
 *	processor dependent.
 */

static unsigned is_simple(struct node *n)
{
	unsigned op = n->op;

	/* Multi-word objects are never simple */
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;

	/* We can load these directly into a register */
	if (op == T_CONSTANT || op == T_LABEL || op == T_NAME || op == T_RREF)
		return 10;
	/* We can load this directly into a register but may need xchg pairs */
	if (op == T_NREF || op == T_LBREF)
		return 1;
	return 0;
}

/*
 *	Turn it 8bit
 */
struct node *gen_rewrite(struct node *n)
{
	byte_label_tree(n, BTF_RELABEL);
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

	/* *regptr. Only works for bytes, so if the size is wrong (e.g. the pointer
	   was cast) then force it into HL and the usual path */
	if (op == T_DEREF && r->op == T_RREF && (nt == CCHAR || nt == UCHAR)) {
		n->op = T_RDEREF;
		n->right = NULL;
		n->val2 = 0;
		n->value = r->value;
		free_node(r);
		return n;
	}
	/* *regptr = */
	if (op == T_EQ && l->op == T_RREF && (nt == CCHAR || nt == UCHAR)) {
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
	/* Turn ++ and -- into easier forms when possible */
	if (op == T_PLUSPLUS && (n->flags & NORETURN))
		n->op = T_PLUSEQ;
	if (op == T_MINUSMINUS && (n->flags & NORETURN))
		n->op = T_MINUSEQ;
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
	output("_%s:", name);
	unreachable = 0;
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size;
	sp = 0;

	if (size || func_flags & F_REG(1))
		func_cleanup = 1;
	else
		func_cleanup = 0;

	argbase = ARGBASE;
	if (func_flags & F_REG(1)) {
		outputne("push bc");
		argbase += 2;
	}
	while(size >= 128) {
		outputne("add sp,#-128");
		size -= 128;
	}
	if (size)
		outputne("add sp,-%u", size);
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp != 0)
		error("sp");

	if (unreachable)
		return;
	while(size >= 127) {
		outputne("add sp,#127");
		size -= 127;
	}
	if (size)
		outputne("add sp,#%u", size);
	outputne("ret");
}

void gen_label(const char *tail, unsigned n)
{
	unreachable = 0;
	output("L%u%s:", n, tail);
}

/* A return statement. We can sometimes shortcut this if we have
   no cleanup to do */
unsigned gen_exit(const char *tail, unsigned n)
{
	if (unreachable)
		return 1;
	if (func_cleanup) {
		gen_jump(tail, n);
		unreachable = 1;
		return 0;
	} else {
		outputne("ret");
		unreachable = 1;
		return 1;
	}
}

void gen_jump(const char *tail, unsigned n)
{
	/* Force anything deferred to complete before the jump */
	outputne("jr L%u%s", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	switch(ccvalid) {
	case CC_VALID:
		outputne("jr nz, L%u%s", n, tail);
		break;
	case CC_INVERSE:
		outputne("jr z, L%u%s", n, tail);
		break;
	default:
		error("jfu");
	}
}

void gen_jtrue(const char *tail, unsigned n)
{
	switch(ccvalid) {
	case CC_VALID:
		outputne("jr z, L%u%s", n, tail);
		break;
	case CC_INVERSE:
		outputne("jr nz, L%u%s", n, tail);
		break;
	default:
		error("jtu");
	}
}

static void gen_cleanup(unsigned v)
{
	/* CLEANUP is special and needs to be handled directly */
	sp -= v;
	while(v >= 127) {
		outputne("add sp,#%u", v);
		v -= 127;
	}
	if (v)
		outputne("add sp,#%u", v);
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
	printf("\tcall __");
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
		gen_cleanup(s);
		/* C style ops that are ISBOOL didn't set the bool flags */
		/* Need to think about keeping bool stuff 8bit here */
		if (n->flags & ISBOOL)
			printf("\txor a\n\tcp l\n");
	}
	if (n->flags & ISBOOL)
		ccvalid = CC_VALID;
}

void gen_switch(unsigned n, unsigned type)
{
	outputne("ld de,Sw%u", n);
	printf("\tjp __switch");
	helper_type(type, 0);
	putchar('\n');
}

void gen_switchdata(unsigned n, unsigned size)
{
	outputne("Sw%u:", n);
	outputne(".word %u", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
	output("Sw%u_%u:", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	outputne(".word Sw%u_%u", tag, entry);
}

void gen_data_label(const char *name, unsigned align)
{
	outputne("_%s:", name);
}

void gen_space(unsigned value)
{
	outputne(".ds %u", value);
}

void gen_text_data(struct node *n)
{
	outputne(".word T%u", n->val2);
}

/* The label for a literal (currently only strings) */
void gen_literal(unsigned n)
{
	if (n)
		outputne("T%u:", n);
}

void gen_name(struct node *n)
{
	outputne(".word _%s+%u", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	unsigned w = WORD(value);
	if (PTR(type)) {
		outputne(".word %u", w);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		outputne(".byte %u", BYTE(w));
		break;
	case CSHORT:
	case USHORT:
		outputne(".word %u", w);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		outputne(".word %u\n", w);
		outputne(".word %u\n", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
}

void gen_end(void)
{
}

void gen_tree(struct node *n)
{
	codegen_lr(n);
	outputne(";:");
/*	printf(";SP=%d\n", sp); */
}

/* Make HL = SP + offset */
static void hl_from_sp(unsigned off)
{
	if (off < 128)
		outputne("ldhl sp,%u", off);
	else {
		outputne("ld hl,%u", off);
		output("add hl,sp");
	}
}

/*
 *	Get a local variable into HL or DE. DE can only be used for size 2
 *
 *	Loading into HL may not fail (return 0 is a compiler abort) but may
 *	trash DE as well. Loading into DE may fail and is not permitted
 *	to trash HL.
 */
unsigned gen_lref(unsigned v, unsigned size, unsigned to_de)
{
	/* Trivial case: if the variable is top of stack then just pop and
	   push it back */
	if (v == 0 && size == 2) {
		if (to_de) {
			output("pop de");
			output("push de");
		} else {
			output("pop hl");
			output("push hl");
		}
		return 1;
	}
	/*
	 *	We can get at the second variable fastest by popping two
	 *	things but must destroy DE so can only use this for HL
	 */
	if (!to_de && v == 2 && size == 2) {
		output("pop de");
		output("pop hl");
		output("push hl");
		output("push de");
		return 1;
	}
	/*
	 *	Shortest forms use ldhl sp,#n for range up to 127
	 */
	if (size <= 2) {
		if (to_de)
			outputne("ex de,hl");
		hl_from_sp(v);
		if (size == 2) {
			output("ldi a,(hl)");
			output("ld h,(hl)");
			output("ld l,a");
		} else {
			if (to_de)
				output("ld l,(hl)");
			else
				output("ld a,(hl)");
		}
		if (to_de)
			outputne("ex de,hl");
		return 1;
	}
	return 0;	/* Can't happen currently but trap it */
}

/*
 *	Try and generate shorter code for stuff we can directly access
 */

/*
 *	Return 1 if the node can be turned into direct access. The VOID check
 *	is a special case we need to handle stack clean up of void functions.
 */
static unsigned access_direct(struct node *n)
{
	unsigned op = n->op;

	/* We can direct access integer or smaller types that are constants
	   global/static or string labels */
	if (op != T_CONSTANT && op != T_NAME && op != T_LABEL &&
		 op != T_NREF && op != T_LBREF && op != T_RREF && op != T_LREF)
		 return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;
	return 1;
}

#if 0
static unsigned access_direct_b(struct node *n)
{
	/* We can't access as much via B directly because we've got no easy xchg with b */
	/* TODO: if we want BC we need to know if BC currently holds the reg var */
	if (n->op != T_CONSTANT && n->op != T_NAME && n->op != T_LABEL && n->op != T_REG)
		return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;
	return 1;
}
#endif

static unsigned point_hl_at(struct node *n)
{
	unsigned v = n->value;
	switch(n->op) {
	case T_NREF:
	case T_NSTORE:
	case T_NAME:
		output("ld hl,_%s+%u", namestr(n->snum), v);
		break;
	case T_LBREF:
	case T_LBSTORE:
	case T_LABEL:
		output("ld hl,T%u+%u", n->val2, v);
		break;
	case T_ARGUMENT:
		v += frame_len + argbase;
	case T_LOCAL:
	case T_LREF:
	case T_LSTORE:
		v += sp;
		hl_from_sp(v);
		break;
	default:
		return 0;
	}
	return 1;
}

static void load_via_hl(unsigned r, unsigned s)
{
	if (s == 1) {
		if (r == 'h' || r == 'a')
			output("ld a,(hl)");
		else if (r == 'd')
			output("ld e,(hl)");
		else error("lhlr1");
	} else if (s == 2) {
		if (r == 'h') {
			output("ldi a,(hl)");
			output("ld h,(hl)");
			output("ld l,a");
		} else if (r == 'd') {
			output("ld e,(hl)");
			output("inc hl");
			output("ld d,(hl)");
		} else error("lhlr2");
	} else
		error("lhlrs");
}
	
static void store_via_hl(unsigned r, unsigned s)
{
	if (s == 1) {
		if (r == 'h' || r == 'a')
			outputne("ld (hl),a");
		else if (r == 'd')
			outputne("(hl),e");
		else error("shlr1");
	} else if (s == 2) {
		if (r == 'd') {
			outputne("ld (hl),d");
			outputne("inc hl");
			outputne("ld (hl),d");
		} else error("shlr2");
	} else
		error("shlrs");
}
	

/*
 *	Get something that passed the access_direct check into de. Could
 *	we merge this with the similar hl one in the main table ?
 */
static unsigned load_r_with(const char *r, struct node *n)
{
	unsigned v = WORD(n->value);
	const char *name;

	switch(n->op) {
	case T_NAME:
		output("ld %s,_%s+%u", r, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		output("ld %s,T%u+%u", r, n->val2, v);
		return 1;
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		output("ld %s,%u", r, v);
		return 1;
	case T_NREF:
		name = namestr(n->snum);
		if (*r == 'b')
			return 0;
		if (*r == 'd')
			outputne("ex de,hl");
		output("ld hl, _%s+%u", name, v);
		load_via_hl(*r, 2);
		if (*r == 'd')
			outputne("ex de,hl");
		return 1;
	/* TODO: fold together cleanly with NREF */
	case T_LBREF:
		if (*r == 'b')
			return 0;
		if (*r == 'd')
			outputne("ex de,hl");
		output("ld hl, T%u+%u", n->val2, v);
		load_via_hl(*r, 2);
		if (*r == 'd')
			outputne("ex de,hl");
		return 1;
	case T_RREF:
		if (*r == 'd') {
			outputne("ld d,b");
			outputne("ld e,c");
		} else if (*r == 'h') {
			output("ld h,b");
			output("ld l,c");
		}
		/* Assumes that BC isn't corrupted yet so is already the right value. Use
		   this quirk with care */
		return 1;
	default:
		return 0;
	}
	return 1;
}

static unsigned load_bc_with(struct node *n)
{
	/* No lref direct to BC option for now */
	return load_r_with("bc", n);
}

static unsigned load_de_with(struct node *n)
{
	if (n->op == T_LREF)
		return gen_lref(n->value + sp, 2, 1);
	return load_r_with("de", n);
}

#if 0
static unsigned load_hl_with(struct node *n)
{
	if (n->op == T_LREF)
		return gen_lref(n->value + sp, 2, 0);
	return load_r_with("hl", n);
}
#endif

static unsigned load_a_with(struct node *n)
{
	unsigned v = WORD(n->value);
	switch(n->op) {
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		output("ld a,%u", BYTE(v));
		break;
	case T_NREF:
		output("ld a,(_%s+%u)", namestr(n->snum), v);
		break;
	case T_LBREF:
		output("ld a,(T%u+%u)", n->val2, v);
		break;
	case T_RREF:
		output("ld a,c");
		break;
	case T_LREF:
		/* We don't want to trash HL as we may be doing an HL:A op */
		outputne("ex de,hl");
		hl_from_sp(v);
		output("ld a,(hl)");
		break;
	default:
		return 0;
	}
	return 1;
}

static void repeated_op(const char *o, unsigned n)
{
	while(n--)
		output(o);
}

static void loadhl(struct node *n, unsigned s)
{
	if (n && (n->flags & NORETURN))
		return;
	if (s == 1)
		output("ld a,c");
	else if (s == 2) {
		output("ld l,c");
		output("ld h,b");
	} else
		error("ldhl");
}

static void loadbc(unsigned s)
{
	if (s == 1)
		outputne("ld c,a");
	else if (s == 2) {
		outputne("ld c,l");
		outputne("ld b,h");
	} else
		error("ldbc");
}

/* We use "DE" as a name but A as register for 8bit ops... probably ought to rework one day */
/* TODO: fix me for byte size deops where A already holds the value */
static unsigned gen_deop(const char *op, struct node *n, struct node *r, unsigned sign)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	if (s == 2) {
		if (load_de_with(r) == 0)
			return 0;
	} else {
		if (load_a_with(r) == 0)
			return 0;
	}
	if (sign)
		helper_s(n, op);
	else
		helper(n, op);
	return 1;
}

static unsigned gen_compc(const char *op, struct node *n, struct node *r, unsigned sign)
{
	if (r->op == T_CONSTANT && r->value == 0 && r->type != FLOAT) {
		char buf[10];
		strcpy(buf, op);
		strcat(buf, "0");
		if (sign)
			helper_s(n, buf);
		else
			helper(n, buf);
		n->flags |= ISBOOL;
		ccvalid = CC_VALID;
		return 1;
	}
	if (gen_deop(op, n, r, sign)) {
		n->flags |= ISBOOL;
		ccvalid = CC_VALID;
		return 1;
	}
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

/* TODO: mul logic has to allow for 8bit now - so A * n */
/* Write the multiply for any value > 0 */
static void write_mul(unsigned n)
{
	unsigned pops = 0;
	if ((n & 0xFF) == 0) {
		output("ld h,l");
		output("ld l,0");
		n >>= 8;
	}
	while(n > 1) {
		if (n & 1) {
			pops++;
			outputne("push hl");
		}
		output("add hl,hl");
		n >>= 1;
	}
	while(pops--) {
		outputne("pop de");
		output("add hl,de");
	}
}

static unsigned can_fast_mul(unsigned s, unsigned n)
{
	/* Pulled out of my hat 8) */
	unsigned cost = 15 + 3 * opt;
	/* The base cost of the helper is 6 lxi de, n; call, but this may be too aggressive
	   given the cost of mulde TODO */
	if (optsize)
		cost = 10;
	if (s > 2)
		return 0;
	if (n == 0 || count_mul_cost(n) <= cost)
		return 1;
	return 0;
}

static void gen_fast_mul(unsigned s, unsigned n)
{

	if (n == 0)
		output("ld hl,0");
	else
		write_mul(n);
}

static unsigned gen_fast_div(unsigned s, unsigned n)
{
	return 0;
}

static unsigned gen_fast_udiv(unsigned n, unsigned s)
{
	if (s != 2)
		return 0;
	if (n == 1)
		return 1;
	if (n == 256) {
		output("ld l,h");
		output("ld h,0");
		return 1;
	}
	return 0;
}

static unsigned gen_logicc(struct node *n, unsigned s, const char *op, unsigned v, unsigned code)
{
	unsigned h = (v >> 8) & 0xFF;
	unsigned l = v & 0xFF;

	if (s > 2 || (n && n->op != T_CONSTANT))
		return 0;


	if (s == 2) {
		/* If we are trying to be compact only inline the short ones */
		if (optsize && ((h != 0 && h != 255) || (l != 0 && l != 255)))
			return 0;
		if (h == 0) {
			if (code == 1)
				output("ld h,0");
		}
		else if (h == 255 && code != 3) {
			if (code == 2)
				output("ld h, 255");
		} else {
			output("ld a,h");
			if (code == 3 && h == 255)
				output("cpl");
			else
				output("%s %u", op, h);
			output("ld a,h");
		}
		if (l == 0) {
			if (code == 1)
				output("ld l,0");
		} else if (l == 255 && code != 3) {
			if (code == 2)
				output("ld l,255");
		} else {
			output("ld a,l");
			if (code == 3 && l == 255)
				output("cpl");
			else
				output("%s %u", op, l);
			output("ld l,a");
		}
	} else {
		outputcc("%s %u", op, l);
	}
	return 1;
}

static unsigned gen_fast_remainder(unsigned n, unsigned s)
{
	unsigned mask;
	if (s != 2)
		return 0;
	if (n == 1) {
		output("ld hl,0");
		return 1;
	}
	if (n == 256) {
		output("ld h, 0");
		return 1;
	}
	if (n & (n - 1))
		return 0;
	if (!optsize) {
		mask = n - 1;
		gen_logicc(NULL, s, "and", mask, 1);
		return 1;
	}
	return 0;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 *
 *	The 8080 is pretty basic so there isn't a lot we turn around here. As
 *	proof of concept we deal with the add case. Other processors may be
 *	able to handle a lot more.
 *
 *	If your processor is good at subtracts you may also want to rewrite
 *	constant on the left subtracts in the rewrite rules into some kind of
 *	rsub operator.
 */
unsigned gen_direct(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned v;
	unsigned nr = n->flags & NORETURN;
	int b;
	unsigned is_byte = (n->flags & (BYTETAIL | BYTEOP)) == (BYTETAIL | BYTEOP);

	/* We only deal with simple cases for now */
	if (r) {
		if (!access_direct(n->right))
			return 0;
		v = r->value;
	}

	switch (n->op) {
	case T_CLEANUP:
		gen_cleanup(v);
		return 1;
	case T_NSTORE:
		if (s > 2)
			return 0;
		if (s == 2)
			outputne("ex de,hl");
		output("ld hl,_%s+%u", namestr(n->snum), v);
		store_via_hl('d', s);
		if (s == 2)
			outputne("ex de,hl");
		/* TODO 4/8 for long etc */
		return 0;
	case T_LBSTORE:
		if (s > 2)
			return 0;
		if (s == 2)
			outputne("ex de,hl");
		output("ld hl,T%u+%u", n->val2, v);
		store_via_hl('d', s);
		if (s == 2)
			outputne("ex de,hl");
		return 1;
	case T_RSTORE:
		loadbc(s);
		return 1;
	case T_EQ:
		/* The address is in HL at this point */
		if (r->op == T_CONSTANT && nr) {
			outputne("ld (hl),%u", v & 0xFF);
			if (s == 2) {
				outputne("inc hl");
				outputne("ld (hl),%u", v >> 8);
			}
			return 1;
		}
		/* We can do this via HL and A */
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			outputne("ld (hl),a");
			return 1;
		}
		return 0;
	case T_PLUS:
		if (r->op == T_CONSTANT) {
			if (s == 1) {
				outputcc("add %u", BYTE(v));
				return 1;
			} else  if (v < 4 && s == 2) {
				repeated_op("inc hl", v);
				return 1;
			}
		}
		if (s <= 2) {
			/* TODO: need instead I think to point HL at target
			   and add hl */
			if (s == 1) {
				if (point_hl_at(r) == 0)
					return 0;
				outputcc("add a,(hl)");
				return 1;
			}
			/* Short cut register case */
			if (r->op == T_RREF) {
				outputcc("add hl,bc");
				return 1;
			}
			if (s > 2 || load_de_with(r) == 0)
				return 0;
			outputcc("add hl,de");
			return 1;
		}
		return 0;
	case T_MINUS:
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (s == 1) {
				outputcc("sub %u", BYTE(v));
				return 1;
			}
			if (v < 6 && s == 2) {
				repeated_op("dec hl", v);
				return 1;
			}
			outputne("ld de,%u", 65536 - v);
			outputcc("add hl,de");
			return 1;
		}
		return 0;
	case T_STAR:
		if (r->op == T_CONSTANT) {
			if (s <= 2 && can_fast_mul(s, r->value)) {
				gen_fast_mul(s, r->value);
				return 1;
			}
		}
		return gen_deop("mulde", n, r, 0);
	case T_SLASH:
		if (r->op == T_CONSTANT && s <= 2) {
			if (n->type & UNSIGNED) {
				if (gen_fast_udiv(s, v))
					return 1;
			} else {
				if (gen_fast_div(s, v))
					return 1;
			}
		}
		return gen_deop("divde", n, r, 1);
	case T_PERCENT:
		if (r->op == T_CONSTANT && (n->type & UNSIGNED)) {
			if (s <= 2 && gen_fast_remainder(s, r->value))
				return 1;
		}
		return gen_deop("remde", n, r, 1);
	case T_AND:
		/* Better to use bit for single bit set on word */
		/* TODO: we could do similar for register targeted |= ? */
		if (s == 2 && r->op == T_CONSTANT && (n->flags & CCONLY)) {
			b = bitcheck0(v, s);
			if (b >= 0) {
				/* Single set bit */
				if (b < 8)
					printf("\tres %u,l\n", b);
				else
					printf("\tres %u,h\n", b - 8);
				return 1;
			}
		}
		if (gen_logicc(r, s, "and", r->value, 1))
			return 1;
		return gen_deop("bandde", n, r, 0);
	case T_OR:
		/* Better to use bit for single bit set on word */
		/* TODO: we could do similar for register targeted |= ? */
		if (s == 2 && r->op == T_CONSTANT && (n->flags & CCONLY)) {
			b = bitcheck1(v, s);
			if (b >= 0) {
				/* Single set bit */
				if (b < 8)
					printf("\tset %u,l\n", b);
				else
					printf("\tset %u,h\n", b - 8);
				return 1;
			}
		}
		if (gen_logicc(r, s, "or", r->value, 2))
			return 1;
		return gen_deop("borde", n, r, 0);
	case T_HAT:
		if (gen_logicc(r, s, "xor", r->value, 3))
			return 1;
		return gen_deop("bxorde", n, r, 0);
	case T_EQEQ:
		if (is_byte && r->op == T_CONSTANT && (n->flags & CCONLY)) {
			outputinv("cp %u", BYTE(v));
			n->flags |= ISBOOL;
			return 1;
		}
		return gen_compc("cmpeq", n, r, 0);
	/* TODO: byte forms of these - need more ccvalid states for branch
		 type flipping (jr nc/c etc) */
	case T_GTEQ:
		return gen_compc("cmpgteq", n, r, 1);
	case T_GT:
		return gen_compc("cmpgt", n, r, 1);
	case T_LTEQ:
		return gen_compc("cmplteq", n, r, 1);
	case T_LT:
		return gen_compc("cmplt", n, r, 1);
	case T_BANGEQ:
		if (s == 1 && r->op == T_CONSTANT && (n->flags & CCONLY)) {
			outputcc("cp %u", BYTE(v));
			n->flags |= ISBOOL;
			return 1;
		}
		return gen_compc("cmpne", n, r, 0);
	case T_LTLT:
		if (s == 1 && r->op == T_CONSTANT) { 
			repeated_op("add a,a", r->value & 7);
			return 1;
		}
		if (s == 2 && r->op == T_CONSTANT) {
			v = r->value;
			if (v >= 8) {
				output("ld h,l");
				output("ld l,0");
				v = v & 7;
			}
			repeated_op("add h,hl", v);
			return 1;
		}
		return gen_deop("shlde", n, r, 0);
	case T_GTGT:
		/* >> by 8 unsigned */
		if ((n->type & UNSIGNED) && r->op == T_CONSTANT) {
			v = r->value;
			if (s == 2 && v == 8) {
				output("ld l,h");
				output("ld h,0");
				return 1;
			}
			if (s == 1) {
				v &= 7;
				while(v--) {
					outputcc("or a,a");
					outputcc("rra");
				}
				return 1;
			}
		}
		return gen_deop("shrde", n, r, 1);
	/* Shorten post inc/dec if result not needed - in which case it's the same as
	   pre inc/dec */
	case T_PLUSPLUS:
		if (!(n->flags & NORETURN))
			return 0;
	case T_PLUSEQ:
		if (s == 1) {
			if (r->op == T_CONSTANT && r->value < 4 && nr)
				repeated_op("inc (hl)", r->value);
			else {
				if (load_a_with(r) == 0)
					return 0;
				outputcc("add a,(hl)");
				outputne("ld (hl),a");
			}
			return 1;
		}
		if (s == 2 && nr && r->op == T_CONSTANT) {
			if ((r->value & 0x00FF) == 0) {
				outputne("inc hl");
				if ((r->value >> 8) < 4) {
					repeated_op("inc (hl)", r->value >> 8);
					return 1;
				}
				output("ld a,%u", r->value >> 8);
				output("add (hl)");
				output("ld (hl),a");
				return 1;
			}
			if (r->value == 1) {
				output("inc (hl)");
				output("jr nz, X%u", ++label);
				output("inc hl");
				output("inc (hl)");
				output("X%u:", label);
				return 1;
			}
		}
		return gen_deop("pluseqde", n, r, 0);
	case T_MINUSMINUS:
		if (!(n->flags & NORETURN))
			return 0;
	case T_MINUSEQ:
		if (s == 1) {
			/* Shortcut for small 8bit values */
			if (r->op == T_CONSTANT && r->value < 4 && (n->flags & NORETURN)) {
				repeated_op("dec (hl)", r->value);
			} else {
				/* Subtraction is not transitive so this is
				   messier */
				if (r->op == T_CONSTANT) {
					if (r->value == 1) {
						output("dec (hl)");
						output("ld a,(hl)");
					} else {
						output("ld a,(hl)");
						output("sub %u", BYTE(r->value));
						output("ld (hl),a");
					}
				} else {
					if (load_a_with(r) == 0)
						return 0;
					output("cpl");
					output("inc a");
					output("add (hl)");
					output("ld (hl),a");
				}
			}
			return 1;
		}
		if (s == 2 && nr && r->op == T_CONSTANT) {
			if ((r->value & 0x00FF) == 0) {
				output("inc hl");
				if ((r->value >> 8) < 4) {
					repeated_op("dec (hl)", r->value >> 8);
					return 1;
				}
				output("ld a,(hl)");
				output("sub %u", r->value >> 8);
				output("ld (hl),a");
				return 1;
			}
			if (r->value == 1) {
				output("dec (hl)");
				output("jr nc, X%u", ++label);
				output("inc hl");
				output("dec (hl)");
				output("X%u:", label);
				return 1;
			}
		}
		return gen_deop("minuseqde", n, r, 0);
	case T_ANDEQ:
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			outputcc("and (hl)");
			outputne("ld (hl),a");
			return 1;
		}
		return gen_deop("andeqde", n, r, 0);
	case T_OREQ:
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			outputcc("or (hl)");
			outputne("ld (hl),a");
			return 1;
		}
		return gen_deop("oreqde", n, r, 0);
	case T_HATEQ:
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			outputcc("xor (hl)");
			outputne("ld (hl),a");
			return 1;
		}
		return gen_deop("xoreqde", n, r, 0);
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
	register struct node *r = n->right;
	unsigned v;
	/* There are some uni operations on registers we can do
	   non destructively but directly on BC */
	if (r && r->op == T_RREF && r->value == 1) {
		if (n->op == T_BTST) {
			/* Bit test on BC - always CCONLY */
			v = n->value;
			if (v < 8)
				printf("\tbit %u,c\n", v);
			else
				printf("\tbit %u,b\n", v - 8);
		}
	}
	return 0;
}

static unsigned reg_canincdec(struct node *n, unsigned s, int v)
{
	/* We only deal with short and char for register */
	/* Is the shortcut worth it ? */
	if (n->op != T_CONSTANT || v > 8 || v < -8)
		return 0;
	return 1;
}

static unsigned reg_incdec(unsigned s, int v)
{
	if (s == 1) {
		if (v < 0)
			repeated_op("dec c", -v);
		else
			repeated_op("inc c", v);
	} else {
		if (v < 0)
			repeated_op("dec bc", -v);
		else
			repeated_op("inc bc", v);
	}
	return 1;
}

static void reg_logic(struct node *n, unsigned s, unsigned op, const char *i)
{
	/* TODO : constant optimization */
	codegen_lr(n->right);
	/* HL is now the value to combine with BC */
	if (opt > 1) {
		/* TODO - can avoid the reload into HL if NORETURN */
		if (s == 2) {
			output("ld a,b");
			output("%s h", i + 2);
			output("ld b,a");
			output("ld h,a");
			output("ld a,c");
			output("%s l", i + 2);
			output("ld c, a");
			output("ld l,a");
		}
		if (s == 1) {
			outputcc("%s c", i + 2);
			outputne("ld c,a");
		}
	} else {
		helper(n, i);
		loadhl(n, s);
	}
}

/* Operators where we can push CCONLY downwards */
static unsigned is_ccdown(struct node *n)
{
	register unsigned op = n->op;
	if (op == T_ANDAND || op == T_OROR)
		return 1;
	if (op == T_BOOL)
		return 1;
	if (op == T_BANG && !(n->flags & CCFIXED))
		return 1;
	return 0;
}

/* Operators that we known to handle as CCONLY if possible
   TODO: add logic ops as we can BIT many of them */
static unsigned is_cconly(struct node *n)
{
	register unsigned op = n->op;
	if (op == T_EQEQ || op == T_BANGEQ ||
		op == T_ANDAND || op == T_OROR ||
		op == T_BOOL || op == T_BTST)
		return 1;
	if (op == T_BANG && !(n->flags & CCFIXED))
		return 1;
	return 0;
}

/*
 *	Try and push CCONLY down through the tree
 */
static void propogate_cconly(register struct node *n)
{
	register struct node *l, *r;
	unsigned sz = get_size(n->type);
	unsigned val;

	l = n->left;
	r = n->right;


/*	printf("; considering %x %x\n", n->op, n->flags); */
	/* Only do this for nodes that are CCONLY. For example if we hit
	   an EQ (assign) then whilst the result of the assign may be
	   CC only, the subtree of the assignment is most definitely not */
	if (n->op != T_AND && !is_cconly(n) && !(n->flags & CCONLY))
		return;

	/* We have to special case BIT unfortunately, and this is ugly */

	/* A common C idiom is if (a & bit) which we can rewrite into
	   bit n,h or bit n,l */

	if (n->op == T_AND) {
/*		printf(";AND %x %x %x\n", n->op, r->op, n->flags); */
		if (r->op == T_CONSTANT && sz == 2) {
			val = bitcheck1(r->value, sz);
			if (val != -1) {
				n->op = T_BTST;
				n->value = val;
				free_node(r);
				n->right = l;
				n->left = NULL;
				r = l;
				l = NULL;
			}
		} else
			return;
	}
	n->flags |= CCONLY;
	/* Deal with the CCFIXED limitations for now */
	if (n->flags & CCFIXED) {
		if (l)
			l->flags |= CCFIXED;
		if (r)
			r->flags |= CCFIXED;
	}
/*	printf(";made cconly %x\n", n->op); */
	/* Are we a node that can CCONLY downwards */
	if (is_ccdown(n)) {
/*		printf(";ccdown of %x L\n", n->op); */
		if (l)
			propogate_cconly(l);
/*		printf(";ccdown cont %x R\n", n->op); */
		if (r)
			propogate_cconly(r);
/*		printf(";ccdown done %x\n", n->op); */
	}
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
	unsigned v;
	unsigned nr = n->flags & NORETURN;

	/* Unreachable code we can shortcut into nothing whee.be.. */
	if (unreachable)
		return 1;

	/* Try and rewrite this node subtree for CC only */
	if (n->flags & CCONLY)
		propogate_cconly(n);

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
			if (ccvalid == CC_UNDEF) {
				if (s == 2 && !(n->flags & BYTEOP)) {
					outputne("ld a,h");
					outputcc("or l");
				} else
					outputcc("or a");
			}
			return 1;
		}
		/* TODO: Can we get the case where we have a bool of a cc */
		/* If we will need to turn a CC into a value */
		/* Too big or value needed */
		helper(n, "bool");
		n->flags |= ISBOOL;
		ccvalid = CC_VALID;
		return 1;
	}
	if (n->op == T_BANG) {
		codegen_lr(r);
		if (r->flags & ISBOOL)
			return 1;
		s = get_size(r->type);
		if (s <= 2 && (n->flags & CCONLY)) {
			if (ccvalid == CC_INVERSE)
				ccvalid = CC_VALID;
			else if (ccvalid == CC_UNDEF) {
				if (s == 2 && !(n->flags & BYTEOP)) {
					outputne("ld a,h");
					outputinv("or l");
				} else
					outputinv("or a");
			}
			return 1;
		}
		/* Too big or value needed */
		helper(n, "not");
		n->flags |= ISBOOL;
		ccvalid = CC_VALID;
		return 1;
	}
	/* Re-order assignments we can do the simple way */
	if (n->op == T_NSTORE && s <= 2) {
		codegen_lr(r);
		/* TODO: no ld (nnnn),hl */
		/* Expression result is now in HL */
		if (s == 2) {
			printf("\tshld");
			/* TODO */
		} else
			outputne("ld (_%s+%u),a", namestr(n->snum), WORD(n->value));
		return 1;
	}
#if 0	
	if (n->op == T_LSTORE && s <= 2) {
		if (n->value + sp == 0 && s == 2) {
			/* The one case 8080 is worth doing */
			codegen_lr(r);
			if (n->flags & NORETURN)
				printf("\txthl\n");
			else
				printf("\tpop psw\n\tpush h\n");
			return 1;
		}
		if (cpu == 8085 && n->value + sp < 255) {
			codegen_lr(r);
			o(OP_LDSI, R_SP, R_DE, "ldsi %u",WORD(n->value + sp));
			if (s == 2)
				printf("\tshlx\n");
			else
				printf("\tmov a,l\n\tstax d\n");
			return 1;
		}
	}
#endif	
	if (s == 2 && n->op == T_PLUSEQ && l->op == T_LOCAL && l->value == 0 &&
		r->op == T_CONSTANT && r->value <= 4) {
		outputne("pop hl");
		repeated_op("inc hl", r->value);
		outputne("push hl");
		return 1;
	}
	if (s == 2 && n->op == T_MINUSEQ && l->op == T_LOCAL && l->value == 0 &&
		r->op == T_CONSTANT && r->value <= 4) {
		outputne("pop hl");
		repeated_op("dec hl", r->value);
		outputne("push hl");
		return 1;
	}
		
	/* Shortcut any initialization of BC we can do directly */
	if (n->op == T_RSTORE && nr) {
		/* We can do LREF in this case but not some others */
		if (r->op == T_LREF) {
			point_hl_at(r);
			output("ld c,(hl)");
			if (s == 2) {
				output("inc hl");
				output("ld b,(hl)");
			}
			return 1;
		}
		if (load_bc_with(r))
			return 1;
		return 0;
	}
	/* Assignment to *BC, byte pointer always */
	if (n->op == T_REQ) {
		unsigned in_l = 0;
		/* Try and get the value into A */
		if (!load_a_with(r)) {
			codegen_lr(r);		/* If not then into HL */
			in_l = 1;
			output("ld a,l");
		}
		outputne("ld (bc),a");	/* Do in case volatile */
		if (!nr && !in_l && s == 2)
			output("ld l,a");
		return 1;
	}
	if (n->op == T_AND && l->op == T_RREF) {
		if (s == 1) {
			if (!load_a_with(r))
				return 0;
			outputcc("and c");
			return 1;
		}
		/* And of register and constant */
		if (s == 2 && r->op == T_CONSTANT) {
			v = r->value;
			if ((v & 0xFF00) == 0x0000)
				output("ld h,0");
			else if ((v & 0xFF00) != 0xFF00) {
				output("ld a, %u", v >> 8);
				output("and b");
				output("ld h,a");
			} else
				output("ld h,b");

			if ((v & 0xFF) == 0x00)
				output("ld l,0");
			else if ((v & 0xFF) != 0xFF) {
				output("ld a, %u", BYTE(v));
				output("and c");
				output("ld l,a");
			} else
				output("ld l,c");

			return 1;
		}
	}
	if (n->op == T_OR && l->op == T_RREF) {
		if (s == 1) {
			if (!load_a_with(r))
				return 0;
			outputcc("or c");
			return 1;
		}
		/* or of register and constant */
		if (s == 2 && r->op == T_CONSTANT) {
			v = r->value;
			if ((v & 0xFF00) == 0xFF00)
				output("ld h,0xff");
			else if (v & 0xFF00) {
				output("ld a, %u", v >> 8);
				output("or b");
				output("ld h,a");
			}
			if ((v & 0xFF) == 0xFF)
				output("ld l,0xff");
			else if (v & 0xFF) {
				output("ld a, %u", BYTE(v));
				output("or c");
				output("ld l,a");
			}
			return 1;
		}
	}
	/* TODO: XOR RREF ditto on 8080 */
	/* ?? LBSTORE */
	/* Register targetted ops. These are totally different to the normal EQ ops because
	   we don't have an address we can push and then do a memory op */
	if (l && l->op == T_REG) {
		v = r->value;
		switch(n->op) {
		case T_PLUSPLUS:
			if (reg_canincdec(r, s, v)) {
				loadhl(n, s);
				reg_incdec(s, v);
				return 1;
			}
			if (!nr) {
				if (s == 2)
					output("tpush bc\n");
				else {
					/* Check AF order TODO */
					output("mov c,a");
					output("push af");
				}
				sp += 2;
			}
			/* Fall through */
		case T_PLUSEQ:
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
				if (s == 1) {
					outputcc("add c");
					outputne("ld c,a");
				}
				else if (s == 2) {
					output("add hl,bc");
					output("ld c,l");
					output("ld b,h");
				}
			}
			if (n->op == T_PLUSPLUS && !(n->flags & NORETURN)) {
				if (s == 2)
					output("pop hl");
				else
					output("pop af");
				sp -= 2;
			}
			return 1;
		case T_MINUSMINUS:
			if (!(n->flags & NORETURN)) {
				if (reg_canincdec(r, s, -v)) {
					loadhl(n, s);
					reg_incdec(s, -v);
					return 1;
				}
				codegen_lr(r);
				if (s == 1) {
					output("ld e,c");
					output("ld a,c");
					output("sub l");
					output("ld c,a");
					output("ld a,e");
					return 1;
				}
				/* Not worth messing with inlined constants as we need the original value */
				helper(n, "bcsub");
				return 1;
			}
			/* If we don't care about the return they look the same so fall
			   through */
		case T_MINUSEQ:
			if (reg_canincdec(r, s, -v)) {
				reg_incdec(s, -v);
				loadhl(n, s);
				return 1;
			}
			if (r->op == T_CONSTANT) {
				if (s == 1) {
					output("ld a,%u", WORD(-v));
					outputcc("add a,c");
				} else {
					output("ld hl,%u", WORD(-v));
					outputcc("add hl,bc");
				}
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
					output("ld c,0");
					loadhl(n, s);
					return 1;
				}
				if (s == 1) {
					printf("\tld a,c\n");
					repeated_op("add a", v);
					printf("\tld c,a\n");
					return 1;
				}
				/* 16 bit */
				if (v >= 16) {
					output("ld bc,0");
					loadhl(n, s);
					return 1;
				}
				if (v == 8) {
					output("ld b,c");
					output("ld c,0");
					loadhl(n, s);
					return 1;
				}
				if (v > 8) {
					output("ld a,c");
					repeated_op("add a", v - 8);
					output("ld b,a");
					output("ld c,0");
					loadhl(n, s);
					return 1;
				}
				/* 16bit full shifting */
				loadhl(NULL, s);
				repeated_op("add hl,hl", v);
				loadbc(s);
				return 1;
			}
			codegen_lr(r);
			helper(n, "bcshl");
			return 1;
		case T_SHREQ:
			if (r->op == T_CONSTANT) {
				if (v >= 8 && s == 1) {
					outputcc("xor a");
					loadbc(s);
					return 1;
				}
				if (v >= 16) {
					output("ld bc,0");
					if (s == 2)
						loadhl(n, s);
					else
						output("xor a");
					return 1;
				}
				if (v == 8 && (n->type & UNSIGNED)) {
					output("ld c,b");
					output("ld b,0");
					loadhl(n, s);
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
	return 0;
}

/* Stack the node which is currently in the working register */
unsigned gen_push(struct node *n)
{
	unsigned size = get_stack_size(n->type);

	/* Our push will put the object on the stack, so account for it */
	sp += size;

	switch(size) {
	case 1:
		outputne("push af");	/* TODO check */
		return 1;
	case 2:
		outputne("push hl");
		return 1;
	case 4:
		outputne("call __pushl");
		return 1;
	default:
		return 0;
	}
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

	/* No type casting needed as computing byte sized */
	if (n->flags & BYTEOP)
		return 1;

	ls = get_size(lt);
	rs = get_size(rt);

	/* Size shrink is not always free for us as it's a reg change */
	if (ls < rs) {
		if (ls > 1 && rs == 1)
			outputne("ld a,l");
		return 1;
	}
	/* Don't do the harder ones */
	if (!(rt & UNSIGNED) || ls > 2)
		return 0;
	output("ld l,a");
	output("ld h,0");
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned v;
	unsigned nr = n->flags & NORETURN;

	/* We adjust sp so track the pre-adjustment one too when we need it */
	v = n->value;

	/* An operation with a left hand node will have the left stacked
	   and the operation will consume it so adjust the stack.

	   The exception to this is comma and the function call nodes
	   as we leave the arguments pushed for the function call */

	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_stack_size(n->left->type);

	switch (n->op) {
		/* Load from a name */
	case T_NREF:
		if (size == 1) {
			output("ld a,(_%s+%u)", namestr(n->snum), v);
			return 1;
		}
		if (size == 2) {
			point_hl_at(n);
			load_via_hl('h', size);
			return 1;
		} else
			error("nrb");
		return 1;
	case T_LBREF:
		if (size == 1) {
			output("ld a,(T%u+%u)\n", n->val2, v);
			return 1;
		}
		point_hl_at(n);
		load_via_hl('h', size);
		return 1;
	case T_LREF:
		/* We are loading something then not using it, and it's local
		   so can go away */
		/* printf(";L sp %u %s(%ld)\n", sp, namestr(n->snum), n->value); */
		if (nr)
			return 1;
		v += sp;
		return gen_lref(v, size, 0);
	case T_RREF:
		if (nr)
			return 1;
		loadhl(n, size);
		return 1;
	case T_NSTORE:
		if (size == 1)
			outputne("ld (_%s+%u),a", namestr(n->snum), v);
		else {
			outputne("ex de,hl");
			point_hl_at(n);
			store_via_hl('d', size);
			if (!nr)
				outputne("ex de,hl");
		}
		return 1;
	case T_LBSTORE:
		if (size == 1) {
			outputne("ld (T%u+%u),a", n->val2, v);
		} else {
			outputne("ex de,hl");
			point_hl_at(n);
			store_via_hl('d', size);
			if (!nr)
				outputne("ex de,hl");
		}
		return 1;
	case T_LSTORE:
		/* Store A or HL at TOS */
		if (size == 1) {
			hl_from_sp(v);
			outputne("ld (hl),a");
			return 1;
		}
		/* Word is tricker */
		if (v == 0 && size == 2 ) {
			output("pop af");
			outputne("push hl");
			return 1;
		}
		if (v == 2 && size == 2) {
			outputne("pop de");
			output("pop af");
			outputne("push hl");
			outputne("push de");
			return 1;
		}
		outputne("ex de,hl");
		hl_from_sp(v);
		outputne("ld (hl),e");
		outputne("inc hl");
		outputne("ld (hl),d");
		if (!nr)
			outputne("ex de,hl");
		return 1;
	case T_RSTORE:
		loadbc(size);
		return 1;
		/* Call a function by name */
	case T_CALLNAME:
		output("call _%s+%u", namestr(n->snum), v);
		return 1;
	case T_EQ:
		/* (TOS) = hl and (TOS) = a */
		if (size == 1) {
			outputne("pop hl");
			outputne("ld (hl),a");
			return 1;
		}
		if (size == 2) {
			outputne("ex de,hl");
			outputne("pop hl");
			outputne("ld (hl),e");
			outputne("inc hl");
			outputne("ld (hl),d");
			if (!nr)
				outputne("ex de,hl");
			return 1;
		}
		break;
	case T_RDEREF:
		/* RREFs on gb will always be byte pointers */
		/* Can't get rid of the ldax until we have proper volatiles */
		output("ld a,(bc)");
		return 1;
	case T_DEREF:
		/* Get (hl) */
		if (size == 1) {
			output("ld a,(hl)");
			return 1;
		}
		if (size == 2) {
			output("ldi a,(hl)");
			output("ld h,(hl)");
			output("ld h,a");
			return 1;
		}
		break;
	case T_FUNCCALL:
		output("call __callhl");
		return 1;
	case T_LABEL:
		if (nr)
			return 1;
		/* Used for const strings and local static */
		point_hl_at(n);
//		opcode(OP_LXI, 0, R_HL, "lxi h,T%u+%u", n->val2, v);
		return 1;
	case T_CONSTANT:
		if (nr)
			return 1;
		switch(size) {
		case 4:
			output("ld hl,%u", ((n->value >> 16) & 0xFFFF));
//TODO			opcode(OP_SHLD, R_HL, R_MEM, "shld __hireg");
		case 2:
			output("ld hl,%u", WORD(v));
			return 1;
		case 1:
			output("ld a,%u", BYTE(v));
			return 1;
		}
		break;
	case T_NAME:
		if (nr)
			return 1;
//		opcode(OP_LXI, 0, R_HL, "lxi h, _%s+%u", namestr(n->snum), v);
		point_hl_at(n);
		return 1;
	/* FIXME: LBNAME ?? */
	case T_ARGUMENT:
		v += frame_len + argbase;
	case T_LOCAL:
		if (nr)
			return 1;
		v += sp;
		hl_from_sp(v);
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
		if (size == 1) {
			outputne("pop de");
			outputcc("add a,e");	/* Check where push af put it */
			return 1;
		} else if (size <= 2) {
			outputne("pop de");
			outputcc("add hl,de");
			return 1;
		}
		break;
	case T_BANG:
		if (n->flags & CCONLY) {
			n->flags |= ISBOOL;
			if (ccvalid) {
				/* Just remember flags are reversed */
				ccvalid = 3 - ccvalid;
				return 1;
			}
			if (size == 1) {
				outputinv("or a");
				return 1;
			}
			if (size == 2) {
				output("ld a,h");
				outputinv("or l");
				return 1;
			}
		}
		printf(";not cconly - %x\n", n->flags);
	case T_BTST:
		/* Always CCONLY */
		if (v < 8)
			printf("\tbit %u, l\n", v);
		else
			printf("\tbit %u, h\n", v - 8);
		return 1;
	}
	return 0;
}
