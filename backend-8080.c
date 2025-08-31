/*
 *	Things to improve
 *	- Use the BC register either via a register hint in the compiler or by looking for
 *	  a non pointer arg candidate (especially on 8080 where the loads are expensive)
 *	- Improve initialized variable generation in stack frame creation
 *	- Avoid the two xchg calls on a void function cleanup (see gen_cleanup)
 *	- Do we want a single "LBREF or NREF name print" function
 *	- Optimize xor 0xff with cpl ?
 *	- Inline load and store of long to static/global/label (certainly for -O2)
 *	- See if we can think down support routines that use the retaddr patching (ideally
 *	  remove them, if not fix Fuzix task switch to save/restore it)
 *	- Track registers more, track hireg
 *	- Consider tracking DE v SP for LDSI stuff on 8085 ?
 *	- Not clear xthl is worth using for the number of times its the wrong
 *	  choice as we needed the value ?
 *	- Rewrite ops that are dad sp; call helper to helper_sp
 *	- Optimise push constant long ?
 *	- More reg vars by using memory fixed addresses ?
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

#define LWDIRECT 24	/* Number of __ldword1 __ldword2 etc forms for fastest access */

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

static unsigned get_size(unsigned t);

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned argbase;	/* Argument offset in current function */
static unsigned unreachable;	/* Code following an unconditional jump */
static unsigned func_cleanup;	/* Zero if we can just ret out */
static unsigned label;		/* Used to hand out local labels in the form X%u */

/*
 *	Delayed ops
 */

static unsigned xchg_pending;	/* Clean up repeated xchg */
static unsigned pushhl_pending;	/* Count of push/pop hl pairs to eliminate */
/*
 *	Register tracking - minimal for now
 */

static uint16_t bc_value;
static uint16_t de_value;
static uint16_t hl_value;
static unsigned bc_valid;
static unsigned de_valid;
static unsigned hl_valid;
static struct node bc_node;
static struct node de_node;
static struct node hl_node;

static void invalidate_bc(void)
{
	bc_valid = 0;
}

static void invalidate_de(void)
{
	de_valid = 0;
}

static void invalidate_hl(void)
{
	hl_valid = 0;
}

static void set_bc_value(uint16_t v)
{
	bc_value = v;
	bc_valid = 1;
}

static void set_de_value(uint16_t v)
{
	de_value = v;
	de_valid = 1;
}

static void set_hl_value(uint16_t v)
{
	hl_value = v;
	hl_valid = 1;
}
unsigned map_op(register unsigned op)
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
	/* Don't do other matches for now */
	default:
		return 0;
	}
	return op;
}

static void set_bc_node(struct node *n)
{
	if (get_size(n->type) > 2)
		invalidate_bc();
	else {
		memcpy(&bc_node, n, sizeof(bc_node));
		bc_node.op = map_op(n->op);
		bc_valid = 2;
	}
}

static void set_de_node(struct node *n)
{
	if (get_size(n->type) > 2)
		invalidate_de();
	else {
		memcpy(&de_node, n, sizeof(de_node));
		de_node.op = map_op(n->op);
		de_valid = 2;
	}
}

static void set_hl_node(struct node *n)
{
	if (get_size(n->type) > 2)
		invalidate_hl();
	else {
		memcpy(&hl_node, n, sizeof(hl_node));
		hl_node.op = map_op(n->op);
		hl_valid = 2;
	}
}

static void set_bc_hl(void)
{
	bc_valid = hl_valid;
	bc_value = hl_value;
	memcpy(&bc_node, &hl_node, sizeof(bc_node));
}

static void set_hl_bc(void)
{
	hl_valid = bc_valid;
	hl_value = bc_value;
	memcpy(&hl_node, &bc_node, sizeof(hl_node));
}

static void set_de_bc(void)
{
	de_valid = bc_valid;
	de_value = bc_value;
	memcpy(&de_node, &bc_node, sizeof(de_node));
}


static void writeflush(struct node *n)
{
	switch(n->op) {
	case T_NREF:
	case T_LREF:
	case T_LBREF:
		n->op = 0xFFFF;	/* invalidate for matches */
		break;
	}
}

static void flush_writeback(void)
{
	if (bc_valid == 2)
		writeflush(&bc_node);
	if (de_valid == 2)
		writeflush(&de_node);
	if (hl_valid == 2)
		writeflush(&hl_node);
}

static void invalidate_all(void)
{
	invalidate_hl();
	invalidate_de();
	invalidate_bc();
	flush_writeback();
}

/*
 *	Output side logic. For now dumb but route everything here so
 *	we can do more useful stuff later
 */


static void opcode(const char *p, ...)
{
	va_list v;
	va_start(v, p);

	/* Finish and write any data */
	while (pushhl_pending) {
		puts("\tpush h");
		pushhl_pending--;
	}
	if (xchg_pending) {
		puts("\txchg");
		xchg_pending = 0;
	}

	if (p) {
		if (*p == ':') {
			p++;
		} else
			putchar('\t');
		vprintf(p, v);
		putchar('\n');
	}
	va_end(v);
}

static void opcode_flush(void)
{
	opcode(NULL);
}

static void set_segment(unsigned seg)
{
	/* Track segments for output */
}

/*
 *	Operation helpers
 */

/* Find a constant if it's in a register somewhere */
static char find_byte(uint_fast8_t v)
{
	if (bc_valid == 1) {
		if ((bc_value & 0xFF) == v)
			return 'c';
		if ((bc_value >> 8) == v)
			return 'b';
	}
	if (de_valid == 1) {
		if ((de_value & 0xFF) == v)
			return 'e';
		if ((de_value >> 8) == v)
			return 'd';
	}
	if (hl_valid == 1) {
		if ((hl_value & 0xFF) == v)
			return 'l';
		if ((hl_value >> 8) == v)
			return 'h';
	}
	return 0;
}

static void load_hl(uint16_t v)
{
	if (hl_valid == 1) {
		if (hl_value == v) {
			printf(";HL is already %u\n", v);
			return;
		}
		if ((hl_value & 0xFF) == (v & 0xFF)) {
			opcode("mvi h,%u", v >> 8);
			return;
		}
		if ((hl_value & 0xFF00) == (v & 0xFF00)) {
			opcode("mvi l,%u", v & 0xFF);
			return;
		}
	}
	/* TODO: There are some other odd cases to do later like when h
	   holds half the value we need so we can mov h,l and l,h etc */
	opcode("lxi h,%u", v);
	set_hl_value(v);
}

static void load_de(uint16_t v)
{
	if (de_valid == 1) {
		if (de_value == v) {
			printf(";DE is already %u\n", v);
			return;
		}
		if ((de_value & 0xFF) == (v & 0xFF)) {
			opcode("mvi d,%u", v >> 8);
			return;
		}
		if ((de_value & 0xFF00) == (v & 0xFF00)) {
			opcode("mvi e,%u", v & 0xFF);
			return;
		}
	}
	/* TODO: There are some other odd cases to do later like when h
	   holds half the value we need so we can mov h,l and l,h etc */
	opcode("lxi d,%u", v);
	set_de_value(v);
}

static void load_bc(uint16_t v)
{
	if (bc_valid == 1) {
		if (bc_value == v) {
			printf(";BC is already %u\n", v);
			return;
		}
		if ((bc_value & 0xFF) == (v & 0xFF)) {
			opcode("mvi b,%u", v >> 8);
			return;
		}
		if ((bc_value & 0xFF00) == (v & 0xFF00)) {
			opcode("mvi c,%u", v & 0xFF);
			return;
		}
	}
	/* TODO: There are some other odd cases to do later like when h
	   holds half the value we need so we can mov h,l and l,h etc */
	opcode("lxi b,%u", v);
	set_bc_value(v);
}

/* Load HL with SP +n, preserve DE - ought to track this properly if we can an*/
static void load_hl_spoff(unsigned n)
{
	load_hl(n);
	opcode("dad sp");
	invalidate_hl();
}

static unsigned hl_contains(register struct node *n)
{
	if (hl_valid == 1 && n->op == T_CONSTANT && WORD(n->value) == hl_value)
		return 1;
	if (hl_valid != 2)
		return 0;
	if (hl_node.op == map_op(n->op) && hl_node.value == n->value &&
	    hl_node.val2 == n->val2 && hl_node.snum == n->snum &&
	    hl_node.type == n->type)
	    	return 1;
	return 0;
}

static unsigned de_contains(register struct node *n)
{
	if (de_valid == 1 && n->op == T_CONSTANT && WORD(n->value) == de_value)
		return 1;
	if (de_valid != 2)
		return 0;
	if (de_node.op == map_op(n->op) && de_node.value == n->value &&
	    de_node.val2 == n->val2 && de_node.snum == n->snum &&
	    de_node.type == n->type)
	    	return 1;
	return 0;
}

static unsigned bc_contains(register struct node *n)
{
	if (bc_valid == 1 && n->op == T_CONSTANT && WORD(n->value) == bc_value)
		return 1;
	if (bc_valid == 0)
		return 0;
	if (bc_node.op == map_op(n->op) && bc_node.value == n->value &&
	    bc_node.val2 == n->val2 && bc_node.snum == n->snum &&
	    bc_node.type == n->type)
	    	return 1;
	return 0;
}

static void load_a(unsigned v)
{
	v &= 0xFF;
	if (v == 0)
		opcode("xra a");
	else
		opcode("mvi a,%u", v);
}

/* Might be making de/hl nodes pointers but that's also an expense on 8bit
   processors */
static void op_xchg(void)
{
	struct node tmp;
	unsigned v = de_valid;
	unsigned val = de_value;
	/* Will be a handy hook point for xchg suppression etc later too */
	memcpy(&tmp, &de_node, sizeof(tmp));
	de_valid = hl_valid;
	de_value = hl_value;
	memcpy(&de_node, &hl_node, sizeof(de_node));
	hl_valid = v;
	hl_value = val;
	memcpy(&hl_node, &tmp, sizeof(hl_node));
	/* We could be smarter here */
	if (pushhl_pending)
		opcode_flush();
	xchg_pending ^= 1;
}

/* Strip out push hl/pop hl pairs (not clear we need this) */
static void op_pushhl(void)
{
	/* Flush out any xchg before it affects hl */
	if (xchg_pending)
		opcode("push h");
	else
		pushhl_pending++;
}

static void op_pophl(void)
{
	if (pushhl_pending) {
		printf(";poppushhl avoided\n");
		pushhl_pending--;
	} else {
		opcode("pop h");
		invalidate_hl();
	}
}

static void inx_h(void)
{
	if (hl_valid != 1)
		invalidate_hl();
	else
		hl_value++;
	opcode("inx h");
}

static void inx_d(void)
{
	if (de_valid != 1)
		invalidate_de();
	else
		de_value++;
	opcode("inx d");
}

static void dcx_h(void)
{
	if (hl_valid != 1)
		invalidate_hl();
	else
		de_value++;
	hl_value--;
	opcode("dcx h");
}

static void dcx_d(void)
{
	if (de_valid != 1)
		invalidate_de();
	else
		de_value++;
	opcode("dcx d");
}

static void modify_hl(unsigned n)
{
	if (hl_valid != 1)
		invalidate_hl();
	else
		hl_value += n;
}

static void dad_de(void)
{
	if (hl_valid != 1 || de_valid != 1)
		invalidate_hl();
	else
		hl_value += de_value;
	opcode("dad d");
}

/* Load HL with SP+n can trash DE - fast on 8085 */
/* If modifying note gen_epilogue assumes old HL ends up in DE for
   the specific use case it has */
static void ldsi_hl(unsigned v)
{
	if (cpu == 8085 && v <= 255) {
		opcode("ldsi %u", v);
		invalidate_de();
		op_xchg();
	} else
		load_hl_spoff(v);
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

static unsigned get_stack_size(unsigned t)
{
	unsigned n = get_size(t);
	if (n == 1)
		return 2;
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
 *	Heuristic for guessing what to put on the right. This is very
 *	processor dependent. For 8080 we are quite limited especially
 *	with locals. In theory we could extend some things to 8bit
 *	locals on 8085 (ldsi, ldax d, mov e,a)
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
	/* We can load this directly into a register but may need xchg pairs */
	if (op == T_NREF || op == T_LBREF)
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
	opcode(".export _%s", name);
}

void gen_segment(unsigned segment)
{
	set_segment(segment);
	switch(segment) {
	case A_CODE:
		opcode(".%s", codeseg);
		break;
	case A_DATA:
		opcode(".data");
		break;
	case A_BSS:
		opcode(".bss");
		break;
	case A_LITERAL:
		opcode(".literal");
		break;
	default:
		error("gseg");
	}
}

/* Generate the function prologue - may want to defer this until
   gen_frame for the most part */
void gen_prologue(const char *name)
{
	opcode(":_%s:", name);
	unreachable = 0;
	invalidate_all();
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
	if (func_flags & F_REG(1)) {
		opcode("push b");
		argbase += 2;
	}
	if (size > 10) {
		load_hl_spoff(WORD(-size));
		opcode("sphl");
		return;
	}
	if (size & 1) {
		opcode("dcx sp");
		size--;
	}
	while(size) {
		op_pushhl();
		size -= 2;
	}
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	unsigned x = func_flags & F_VOIDRET;
	if (sp != 0)
		error("sp");

	if (unreachable)
		return;

	/* Return in HL, does need care on stack. TODO: flag void functions
	   where we can burn the return */
	sp -= size;
	if (cpu == 8085 && size <= 255 && size > 4) {
		ldsi_hl(size);
		opcode("sphl");
		op_xchg();
	} else if (size > 10) {
		if (!x)
			op_xchg();
		load_hl_spoff(size);
		opcode("sphl");
		if (!x)
			op_xchg();
	} else {
		if (size & 1) {
			opcode("inx sp");
			size--;
		}
		while (size) {
			opcode("pop d");
			invalidate_de();
			size -= 2;
		}
	}
	if (func_flags & F_REG(1)) {
		opcode("pop b");
		invalidate_bc();
	}
	/* TODO: make this a little "ret" func as it has several users */
	if (x)
		opcode("ret");
	else
		opcode("ret");
}

void gen_label(const char *tail, unsigned n)
{
	unreachable = 0;
	/* A branch label means the state is unknown so force any
	   existing state and don't assume anything */
	opcode(":L%u%s:", n, tail);
	invalidate_all();
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
		opcode("ret");
		unreachable = 1;
		return 1;
	}
}

void gen_jump(const char *tail, unsigned n)
{
	/* Force anything deferred to complete before the jump */
	opcode("jmp L%u%s", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	opcode("jz L%u%s", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	opcode("jnz L%u%s", n, tail);
}

static void gen_cleanup(unsigned v)
{
	/* CLEANUP is special and needs to be handled directly */
	sp -= v;
	if (v > 10) {
		/* This is more expensive, but we don't often pass that many
		   arguments so it seems a win to stay in HL */
		unsigned x = func_flags & F_VOIDRET;
		if (!x)
			op_xchg();
		load_hl_spoff(v);
		opcode("sphl");
		if (!x)
			op_xchg();
	} else {
		while(v >= 2) {
			opcode("pop d");
			invalidate_de();
			v -= 2;
		}
		if (v)
			opcode("inx sp");
	}
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
	/* Hack until we clean up help generation in backend.c */
	opcode_flush();
	printf("\tcall __");
	/* TODO: once we sort out helper calling from backed only invalidate
	   bc for "bcXX" helpers */
	invalidate_all();
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
		if (n->flags & ISBOOL) {
			load_a(0);
			opcode("add l");
		}
	}
}

void gen_switch(unsigned n, unsigned type)
{
	opcode("lxi d,Sw%u", n);
	/* Nothing is preserved over a switch */
	/* TODO tidy once helper is tidied */
	printf("\tjmp __switch");
	helper_type(type, 0);
	putchar('\n');
}

void gen_switchdata(unsigned n, unsigned size)
{
	opcode(":Sw%u:", n);
	opcode(".word %u", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
	invalidate_all();
	opcode(":Sw%u_%u:", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	opcode(".word Sw%u_%u", tag, entry);
}

void gen_data_label(const char *name, unsigned align)
{
	opcode(":_%s:", name);
}

void gen_space(unsigned value)
{
	opcode(".ds %u", value);
}

void gen_text_data(struct node *n)
{
	opcode(".word T%u", n->val2);
}

/* The label for a literal (currently only strings) */
void gen_literal(unsigned n)
{
	if (n)
		opcode(":T%u:", n);
}

void gen_name(struct node *n)
{
	opcode(".word _%s+%u", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	unsigned w = WORD(value);
	if (PTR(type)) {
		opcode(".word %u", w);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		opcode(".byte %u", BYTE(w));
		break;
	case CSHORT:
	case USHORT:
		opcode(".word %u", w);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		opcode(".word %u", w);
		opcode(".word %u", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	opcode(".setcpu %u", cpu);
}

void gen_end(void)
{
	opcode_flush();
}

void gen_tree(struct node *n)
{
	codegen_lr(n);
	opcode(";");
/*	printf(";SP=%d\n", sp); */
}

/*
 *	Get a local variable into HL or DE.
 *
 *	Loading into HL may not fail (return 0 is a compiler abort) but may
 *	trash DE as well. Loading into DE may fail and is not permitted
 *	to trash HL. Caller is responsible for invalidations of reg caches
 */
unsigned gen_lref(unsigned v, unsigned size, unsigned to_de)
{
	const char *name;
	/* Trivial case: if the variable is top of stack then just pop and
	   push it back */
	if (v == 0 && size == 2) {
		if (to_de) {
			opcode("pop d");
			opcode("push d");
		} else {
			op_pophl();
			op_pushhl();
		}
		return 1;
	}
	/* The 8085 has LDSI and LHLX so we can fast access anything up to
	   255 bytes from SP. However we end up trashing DE in doing so. For
	   now don't use this for DE loads, look at saving stuff later TODO */
	if (cpu == 8085 && v <= 255 && !to_de) {
		opcode("ldsi %u", v);
		invalidate_de();
		if (size == 2)
			opcode("lhlx");
		else {
			opcode("ldax d");
			opcode("mov l,a");
		}
		return 1;
	}
	/*
	 *	We can get at the second variable fastest by popping two
	 *	things but must destroy DE so can only use this for HL
	 */
	if (!to_de && v == 2 && size == 2) {
		invalidate_de();
		opcode("pop d");
		op_pophl();
		op_pushhl();
		opcode("push d");
		return 1;
	}
	/* Byte load is shorter inline for most cases */
	if (size == 1 && (!optsize || v >= LWDIRECT || to_de)) {
		if (to_de)
			op_xchg();
		load_hl_spoff(v);
		opcode("mov l,m");
		if (to_de)
			op_xchg();
		return 1;
	}
	/* Longer reach for 8085 is via addition games, but only for HL
	   as lhlx requires we use DE and HL */
	if (cpu == 8085 && size == 2 && !to_de) {
		load_hl_spoff(v);
		op_xchg();
		opcode("lhlx");
		return 1;
	}
	/* Word load is long winded on 8080 but if the user asked for it */
	/* This also gets used for 8085 as a fallback for the DE case */
	if (size == 2 && opt > 2) {
		if (to_de)
			op_xchg();
		load_hl_spoff(v);
		opcode("mov a,m");
		inx_h();
		opcode("mov h,m");
		opcode("mov l,a");
		if (to_de)
			op_xchg();
		return 1;
	}
	/* The 8080 has a bunch of helpers as we just xchg around them for
	   the DE case. The 8085 has only a word helper and for both the
	   word helper has it use HL and DE. Those cases go via stack */
	if (to_de && (cpu == 8085 || v >= 253))
		return 0;	/* Go via stack */

	/* Via helper magic for compactness on 8080 */
	if (size == 1)
		name = "ldbyte";
	else if (size == 2)
		name = "ldword";
	else
		return 0;	/* Can't happen currently but trap it */
	/* We do a call so the stack offset is two bigger */
	if (to_de)
		op_xchg();

	if (v < LWDIRECT)
		opcode("call __%s%u", name, v + 2);
	else if (v < 253) {
		opcode("call __%s", name);
		opcode(".byte %u", v + 2);
	} else {
		opcode("call __%sw", name);
		opcode(".word %u", v + 2);
	}

	if (to_de)
		op_xchg();

	return 1;
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

/*
 *	Get something that passed the access_direct check into de. Could
 *	we merge this with the similar hl one in the main table ?
 *
 *	Caller is responsible for reg track proper. (TODO move into here)
 */

static unsigned load_r_with(const char r, struct node *n)
{
	unsigned v = WORD(n->value);
	const char *name;

	switch(n->op) {
	case T_NAME:
		opcode("lxi %c,_%s+%u", r, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		opcode("lxi %c,T%u+%u", r, n->val2, v);
		return 1;
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		opcode("lxi %c,%u", r, v);
		return 1;
	case T_NREF:
		name = namestr(n->snum);
		if (r == 'b')
			return 0;
		else if (r == 'h') {
			opcode("lhld _%s+%u", name, v);
			set_hl_node(n);
			return 1;
		} else if (r == 'd') {
			/* We know it is int or pointer */
			op_xchg();
			opcode("lhld _%s+%u", name, v);
			set_hl_node(n);
			op_xchg();
			return 1;
		}
		break;
	/* TODO: fold together cleanly with NREF */
	case T_LBREF:
		if (r == 'b')
			return 0;
		else if (r == 'h') {
			opcode("lhld T%u+%u", n->val2, v);
			set_hl_node(n);
			return 1;
		} else if (r == 'd') {
			/* We know it is int or pointer */
			op_xchg();
			opcode("lhld T%u+%u", n->val2, v);
			set_hl_node(n);
			op_xchg();
			return 1;
		}
		break;
	case T_RREF:
		if (r == 'd') {
			opcode("mov d,b");
			opcode("mov e,c");
			set_de_bc();
		} else if (r == 'h') {
			opcode("mov h,b");
			opcode("mov l,c");
			set_hl_bc();
		}
		/* Assumes that BC isn't corrupted yet so is already the right value. Use
		   this quirk with care (eg T_MINUS) */
		return 1;
	default:
		return 0;
	}
	return 1;
}

static unsigned load_bc_with(struct node *n)
{
	/* No lref direct to BC option for now */
	if (load_r_with('b', n)) {
		set_bc_node(n);
		return 1;
	}
	return 0;
}

static unsigned load_de_with(struct node *n)
{
	if (n->op == T_LREF)
		return gen_lref(n->value + sp, 2, 1);
	return load_r_with('d', n);
}

static unsigned load_hl_with(struct node *n)
{
	if (n->op == T_LREF) {
		if (gen_lref(n->value + sp, 2, 0)) {
			set_hl_node(n);
			return 1;
		}
		return 0;
	}
	if (load_r_with('h', n)) {
		set_hl_node(n);
		return 1;
	}
	return 0;
}

static unsigned load_a_with(struct node *n)
{
	switch(n->op) {
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		load_a(BYTE(n->value));
		break;
	case T_NREF:
		opcode("lda _%s+%u", namestr(n->snum), WORD(n->value));
		break;
	case T_LBREF:
		opcode("lda T%u+%u", n->val2, WORD(n->value));
		break;
	case T_RREF:
		opcode("mov a,c");
		break;
	case T_LREF:
		/* We don't want to trash HL as we may be doing an HL:A op */
		op_xchg();
		load_hl_spoff(n->value);
		opcode("mov a,m");
		op_xchg();
		break;
	default:
		return 0;
	}
	return 1;
}

static void repeated_op(const char *o, unsigned n)
{
	while(n--)
		opcode("%s",o);
}

static void hl_from_reg(struct node *n, unsigned s)
{
	if (n && (n->flags & NORETURN))
		return;
	opcode("mov l,c");
	if (s == 2)
		opcode("mov h,b");
	set_hl_bc();
}

static void bc_to_reg(unsigned s)
{
	opcode("mov c,l");
	if (s == 2)
		opcode("mov b,h");
	set_bc_hl();
}

/* We use "DE" as a name but A as register for 8bit ops... probably ought to rework one day */
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

/* TODO: someone needs to own eliminating no side effect impossible
   or true expressions like unsigned < 0 */
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
		return 1;
	}
	if (gen_deop(op, n, r, sign)) {
		n->flags |= ISBOOL;
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

/* Write the multiply for any value > 0 */
static void write_mul(unsigned n)
{
	unsigned pops = 0;
	if ((n & 0xFF) == 0) {
		opcode("mov h,l");
		opcode("mvi l,0");
		n >>= 8;
	}
	while(n > 1) {
		if (n & 1) {
			pops++;
			op_pushhl();
		}
		opcode("dad h");
		n >>= 1;
	}
	while(pops--) {
		opcode("pop d");
		opcode("dad d");
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

	if (n == 0) {
		load_hl(0);
	} else {
		invalidate_de();
		invalidate_hl();
		write_mul(n);
	}
}

static unsigned gen_fast_div(unsigned s, unsigned n)
{
	int m = n - 1;

	if (cpu != 8085)
		return 0;

	if (n & (n - 1))
		return 0;

	opcode("mov a,h");
	opcode("ora a");
	opcode("jp X%u", ++label);
	/* We can trash DE */
	if (m > 0 && m <= 4)
		repeated_op("inx h", m);
	else if (m < 0 && m >= -4)
		repeated_op("dcx h", -m);
	else {
		load_de(WORD(n - 1));
		opcode("dad d");
	}
	opcode(":X%u:", label);
	while(n > 1) {
		opcode("arhl");
		n >>= 1;
	}
	invalidate_de();
	invalidate_hl();
	return 1;
}

static unsigned gen_fast_udiv(unsigned n, unsigned s)
{
	if (s != 2)
		return 0;
	if (n == 1)
		return 1;
	if (n == 256) {
		opcode("mov l,h");
		opcode("mvi h,0");
		invalidate_hl();
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

	/* If we are trying to be compact only inline the short ones */
	if (optsize && ((h != 0 && h != 255) || (l != 0 && l != 255)))
		return 0;

	invalidate_hl();

	if (s == 2) {
		if (h == 0) {
			if (code == 1)
				opcode("mvi h,0");
		}
		else if (h == 255 && code != 3) {
			if (code == 2)
				opcode("mvi h,255");
		} else {
			opcode("mov a,h");
			if (code == 3 && h == 255)
				opcode("cpl");
			else
				opcode("%s %u", op, h);
			opcode("mov h,a");
		}
	}
	if (l == 0) {
		if (code == 1)
			opcode("mvi l,0");
	} else if (l == 255 && code != 3) {
		if (code == 2)
			opcode("mvi l,255");
	} else {
		opcode("mov a,l");
		if (code == 3&& l == 255)
			opcode("cpl");
		else
			opcode("%s %u", op, l);
		opcode("mov l,a");
	}
	return 1;
}

static unsigned gen_fast_remainder(unsigned n, unsigned s)
{
	unsigned mask;
	if (s != 2)
		return 0;
	if (n == 1) {
		load_hl(0);
		return 1;
	}
	if (n == 256) {
		opcode("mvi h,0");
		hl_value &= 0xFF;
		return 1;
	}
	if (n & (n - 1))
		return 0;
	if (!optsize) {
		mask = n - 1;
		gen_logicc(NULL, s, "ani", mask, 1);
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
		/* FIXME: opcode */
		if (s == 1) {
			opcode("mov a,l");
			opcode("sta _%s+%u", namestr(n->snum), WORD(n->value));
		} else
			opcode("shld _%s+%u", namestr(n->snum), WORD(n->value));
		set_hl_node(n);
		return 1;
	case T_LBSTORE:
		if (s > 2)
			return 0;
		if (s == 1) {
			opcode("mov a,l");
			opcode("sta T%u+%u", n->val2, v);
		} else
			opcode("shld T%u+%u", n->val2, v);
		set_hl_node(n);
		return 1;
	case T_RSTORE:
		bc_to_reg(s);
		return 1;
	case T_EQ:
		/* The address is in HL at this point */
		/* We have to avoid the right being an LREF because we need
		   HL and DE to resolve that sometimes, and the overhead
		   is worse than push/pop the non shortcut way */
		if (cpu == 8085 && s == 2 && r->op != T_LREF) {
			op_xchg();
			if (load_hl_with(r) == 0)
				error("teq");
			opcode("shlx");
			return 1;
		}
		if (s == 1) {
			/* We need to end up with the value in l if this is not NORETURN, also
			   we can optimize constant a step more */
			if (r->op == T_CONSTANT && nr)
				opcode("mvi m,%u", ((unsigned)r->value) & 0xFF);
			else {
				if (load_a_with(r) == 0)
					return 0;
				opcode("mov m,a");
				if (!nr)
					opcode("mov l,a");
				else
					invalidate_hl();
			}
			flush_writeback();
			return 1;
		}
		return 0;
	case T_PLUS:
		/* Zero should be eliminated in cc1 FIXME */
		printf(";T_PLUS hlvalid %u hlvalue %u\n",
			hl_valid, hl_value);
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (v < 4 && s <= 2) {
				if (s == 1) {
					repeated_op("inr l", v);
					modify_hl((hl_value & 0xFF00) +
						(((hl_value & 0xFF) + 1) & 0xFF));
				} else {
					repeated_op("inx h", v);
					modify_hl(v);
				}
				return 1;
			}
			if (s <= 2) {
				load_de(v);
				dad_de();
				return 1;
			}
		}
		invalidate_hl();
		if (s <= 2) {
			/* LHS is in HL at the moment, end up with the result in HL */
			if (s == 1) {
				if (load_a_with(r) == 0)
					return 0;
				invalidate_de();
				opcode("mov e,a");
			}
			/* Short cut register case */
			if (r->op == T_REG) {
				opcode("dad b");
				invalidate_hl();
				return 1;
			}
			if (s > 2 || load_de_with(r) == 0)
				return 0;
			set_de_node(r);
			opcode("dad d");
			invalidate_hl();
			return 1;
		}
		return 0;
	case T_MINUS:
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (v < 6 && s <= 2) {
				if (s == 1)
					repeated_op("dcr l", v);
				else
					repeated_op("dcx h", v);
				modify_hl(-v);
				return 1;
			}
			load_de(WORD(65536 - v));
			dad_de();
			return 1;
		}
		/* load into de then ld a,e sub l ld l,a ld a,d sbc h ld h,s
		   so 6 + load bytes */
		invalidate_hl();
		if (cpu == 8085 && s <= 2)  {
			/* Shortcut subtracting register from working value */
			if (r->op == T_REG) {
				opcode("dsub");
				invalidate_hl();
				return 1;
			}
			if (access_direct_b(r)) {
				opcode("push b");
				sp += 2;
				/* Must not corrupt B before we are ready */
				/* LHS is in HL at the moment, end up with the result in HL */
				if (s == 1) {
					if (load_a_with(r) == 0)
						error("min1");
					opcode("mov c,a");
				} else {
					if (load_bc_with(r) == 0)
						error("min2");
				}
				opcode("dsub");
				opcode("pop b");
				invalidate_bc();	/* TODO save/restore */
				invalidate_hl();
				sp -= 2;
				return 1;
			}
		}
		/* Inlined it's 6 + the de load (~5), out of line it's
		   4 + the HL load (~3) */
		if (s == 2 && load_de_with(r)) {
			set_de_node(r);
			opcode("mov a,l");
			opcode("sub e");
			opcode("mov l,a");
			opcode("mov a,h");
			opcode("sbb d");
			opcode("mov h,a");
			invalidate_hl();
			return 1;
		}
		if (s == 1 && load_a_with(r)) {
			opcode("mov h,a");
			opcode("mov a,l");
			opcode("sub h");
			opcode("mov l,a");
			invalidate_hl();
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
		if (gen_logicc(r, s, "ani", r->value, 1))
			return 1;
		return gen_deop("bandde", n, r, 0);
	case T_OR:
		if (gen_logicc(r, s, "ori", r->value, 2))
			return 1;
		return gen_deop("borde", n, r, 0);
	case T_HAT:
		if (gen_logicc(r, s, "xri", r->value, 3))
			return 1;
		return gen_deop("bxorde", n, r, 0);
	case T_EQEQ:
		return gen_compc("cmpeq", n, r, 0);
	case T_GTEQ:
		return gen_compc("cmpgteq", n, r, 1);
	case T_GT:
		return gen_compc("cmpgt", n, r, 1);
	case T_LTEQ:
		return gen_compc("cmplteq", n, r, 1);
	case T_LT:
		return gen_compc("cmplt", n, r, 1);
	case T_BANGEQ:
		return gen_compc("cmpne", n, r, 0);
	case T_LTLT:
		invalidate_hl();
		if (s <= 2 && r->op == T_CONSTANT) {
			if (r->value >= 8) {
				opcode("mov h,l");
				opcode("mvi l,0");
			}
			repeated_op("dad h", (r->value & 7));
			return 1;
		}
		return gen_deop("shlde", n, r, 0);
	case T_GTGT:
		invalidate_hl();
		/* >> by 8 unsigned */
		if (s == 2 && (n->type & UNSIGNED) && r->op == T_CONSTANT && r->value == 8) {
			opcode("mov l,h");
			opcode("mvi h,0");
			return 1;
		}
		/* 8085 has a signed right shift 16bit */
		if (cpu == 8085 && (!(n->type & UNSIGNED))) {
			if (s == 2 && r->op == T_CONSTANT && v < 8) {
				repeated_op("arhl", v);
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
		invalidate_hl();
		if (s == 1) {
			if (r->op == T_CONSTANT && r->value < 4 && nr)
				repeated_op("inr m", r->value);
			else {
				if (load_a_with(r) == 0)
					return 0;
				opcode("add m");
				opcode("mov m,a");
				if (!nr)
					opcode("mov l,a");
			}
			return 1;
		}
		if (s == 2 && nr && r->op == T_CONSTANT && (r->value & 0x00FF) == 0) {
			inx_h();
			if ((r->value >> 8) < 4) {
				repeated_op("inr m", r->value >> 8);
				return 1;
			}
			load_a(r->value >> 8);
			opcode("add m");
			opcode("mov m,a");
			return 1;
		}
		return gen_deop("pluseqde", n, r, 0);
	case T_MINUSMINUS:
		if (!(n->flags & NORETURN))
			return 0;
	case T_MINUSEQ:
		invalidate_hl();
		if (s == 1) {
			/* Shortcut for small 8bit values */
			if (r->op == T_CONSTANT && r->value < 4 && (n->flags & NORETURN)) {
				repeated_op("dcr m", r->value);
			} else {
				/* Subtraction is not transitive so this is
				   messier */
				if (r->op == T_CONSTANT) {
					if (r->value == 1) {
						opcode("mov a,m");
						opcode("dcr a");
						opcode("mov m,a");
					} else {
						opcode("mov a,m");
						opcode("sbi %u", BYTE(r->value));
						opcode("mov m,a");
					}
				} else {
					if (load_a_with(r) == 0)
						return 0;
					opcode("cma");
					opcode("inr a");
					opcode("add m");
					opcode("mov m,a");
				}
				if (!(n->flags & NORETURN))
					opcode("mov l,a");
			}
			return 1;
		}
		return gen_deop("minuseqde", n, r, 0);
	case T_ANDEQ:
		invalidate_hl();
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			opcode("ana m");
			opcode("mov m,a");
			if (!(n->flags & NORETURN))
				opcode("mov l,a");
			return 1;
		}
		return gen_deop("andeqde", n, r, 0);
	case T_OREQ:
		invalidate_hl();
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			opcode("ora m");
			opcode("mov m,a");
			if (!(n->flags & NORETURN))
				opcode("mov l,a");
			return 1;
		}
		return gen_deop("oreqde", n, r, 0);
	case T_HATEQ:
		invalidate_hl();
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			opcode("xra m");
			opcode("mov m,a");
			if (!(n->flags & NORETURN))
				opcode("mov l,a");
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
			repeated_op("dcr c", -v);
		else
			repeated_op("inr c", v);
	} else {
		if (v < 0)
			repeated_op("dcx b", -v);
		else
			repeated_op("inx b", v);
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
			opcode("mov a,b");
			opcode("%s h", i + 2);
			opcode("mov b,a");
			opcode("mov h,a");
		}
		opcode("mov a,c");
		opcode("%s c", i + 2);
		opcode("mov c,a");
		opcode("mov l,a");
		invalidate_hl();
	} else {
		helper(n, i);
		hl_from_reg(n, s);
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
			if (s == 2) {
				opcode("mov a,h");
				opcode("ora l");
			} else {
				opcode("mov a,l");
				opcode("ora a");
			}
			return 1;
		}
		/* Too big or value needed */
		helper(n, "bool");
		n->flags |= ISBOOL;
		return 1;
	}
	/* Re-order assignments we can do the simple way */
	if (n->op == T_NSTORE && s <= 2) {
		codegen_lr(r);
		/* Expression result is now in HL */
		if (s == 2)
			opcode("shld _%s+%u", namestr(n->snum), WORD(n->value));
		else {
			opcode("mov a,l");
			opcode("sta _%s+%u", namestr(n->snum), WORD(n->value));
		}
		set_hl_node(n);
		return 1;
	}
	if (n->op == T_LBSTORE && s <= 2) {
		codegen_lr(r);
		/* Expression result is now in HL */
		if (s == 2)
			opcode("shld T%u+%u", n->val2, WORD(n->value));
		else {
			opcode("mov a,l");
			opcode("sta T%u+%u", n->val2, WORD(n->value));
		}
		set_hl_node(n);
		return 1;
	}
	/* Locals we can do on 8085, 8080 is doable but messy - so not worth it */
	if (n->op == T_LSTORE && s <= 2) {
		if (n->value + sp == 0 && s == 2) {
			/* The one case 8080 is worth doing */
			codegen_lr(r);
			/* Not clear the xthl is a win versu value live */
			if (0 && (n->flags & NORETURN)) {
				opcode("xthl");
				invalidate_hl();
			} else {
				opcode("pop psw");
				op_pushhl();
				set_hl_node(n);
			}
			return 1;
		}
		if (cpu == 8085 && n->value + sp < 255) {
			codegen_lr(r);
			opcode("ldsi %u", WORD(n->value + sp));
			invalidate_de();
			if (s == 2)
				opcode("shlx");
			else {
				opcode("mov a,l");
				opcode("stax d");
			}
			set_hl_node(n);
			return 1;
		}
	}
	/* Shortcut any initialization of BC we can do directly */
	if (n->op == T_RSTORE && nr) {
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
			opcode("mov a,l");
		}
		opcode("stax b");	/* Do in case volatile */
		if (!nr && !in_l)
			opcode("mov l,a");
		if (nr && !in_l)
			invalidate_hl();
		return 1;
	}
	if (n->op == T_AND && l->op == T_RREF) {
		invalidate_hl();
		if (s == 1) {
			if (!load_a_with(r))
				return 0;
			opcode("ana c");
			opcode("mov l,a");
			return 1;
		}
		/* And of register and constant */
		if (s == 2 && r->op == T_CONSTANT) {
			v = r->value;
			if ((v & 0xFF00) == 0x0000)
				opcode("mvi h,0");
			else if ((v & 0xFF00) != 0xFF00) {
				load_a(v >> 8);
				opcode("ana b");
				opcode("mov h,a");
			} else
				opcode("mov h,b");

			if ((v & 0xFF) == 0x00)
				opcode("mvi l,0");
			else if ((v & 0xFF) != 0xFF) {
				load_a(v & 0xFF);
				opcode("ana c");
				opcode("mov l,a");
			} else
				opcode("mov l,c");

			return 1;
		}
	}
	if (n->op == T_OR && l->op == T_RREF) {
		invalidate_hl();
		if (s == 1) {
			if (!load_a_with(r))
				return 0;
			opcode("ora c");
			opcode("mov l,a");
			return 1;
		}
		/* or of register and constant */
		if (s == 2 && r->op == T_CONSTANT) {
			v = r->value;
			if ((v & 0xFF00) == 0xFF00)
				opcode("mvi h,0xff");
			else if (v & 0xFF00) {
				load_a(v >> 8);
				opcode("ora b");
				opcode("mov h,a");
			}
			if ((v & 0xFF) == 0xFF)
				opcode("mvi l,0xff");
			else if (v & 0xFF) {
				load_a(v & 0xFF);
				opcode("ora c");
				opcode("mov l,a");
			}
			return 1;
		}
	}
	/* TODO XOR */
	/* ?? LBSTORE */
	/* Register targetted ops. These are totally different to the normal EQ ops because
	   we don't have an address we can push and then do a memory op */
	if (l && l->op == T_REG) {
		v = r->value;
		switch(n->op) {
		case T_PLUSPLUS:
			if (reg_canincdec(r, s, v)) {
				hl_from_reg(n, s);
				reg_incdec(s, v);
				return 1;
			}
			if (!nr) {
				opcode("push b");
				sp += 2;
			}
			/* Fall through */
		case T_PLUSEQ:
			if (reg_canincdec(r, s, v)) {
				reg_incdec(s, v);
				if (nr)
					return 1;
				if (n->op == T_PLUSEQ) {
					hl_from_reg(n, s);
				}
			} else {
				/* Amount to add into HL */
				codegen_lr(r);
				opcode("dad b");
				opcode("mov c,l");
				if (s == 2)
					opcode("mov b,h");
			}
			if (n->op == T_PLUSPLUS && !(n->flags & NORETURN)) {
				op_pophl();
				sp -= 2;
			}
			return 1;
		case T_MINUSMINUS:
			if (!(n->flags & NORETURN)) {
				if (reg_canincdec(r, s, -v)) {
					hl_from_reg(n, s);
					reg_incdec(s, -v);
					return 1;
				}
				codegen_lr(r);
				if (s == 1) {
					opcode("mov a,c");
					opcode("sub l");
					opcode("mov l,c");
					opcode("mov c,a");
					invalidate_hl();
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
				hl_from_reg(n, s);
				return 1;
			}
			if (r->op == T_CONSTANT) {
				load_hl(-v);
				opcode("dad b");
				invalidate_hl();
				bc_to_reg(s);
				return 1;
			}
			/* Get the subtraction value into HL */
			codegen_lr(r);
			helper(n, "bcsub");
			/* Result is only left in BC reload if needed */
			hl_from_reg(n, s);
			return 1;
		/* For now - we can do better - maybe just rewrite them into load,
		   op, store ? */
		case T_STAREQ:
			/* TODO: constant multiply */
			if (r->op == T_CONSTANT) {
				if (can_fast_mul(s, v)) {
					hl_from_reg(NULL, s);
					gen_fast_mul(s, v);
					bc_to_reg(s);
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
					opcode("mvi c,0");
					hl_from_reg(n, s);
					return 1;
				}
				if (s == 1) {
					opcode("mov a,c");
					repeated_op("add a", v);
					opcode("mov c,a");
					hl_from_reg(n, s);
					return 1;
				}
				/* 16 bit */
				if (v >= 16) {
					load_bc(0);
					hl_from_reg(n, s);
					return 1;
				}
				if (v == 8) {
					opcode("mov b,c");
					opcode("mvi c,0");
					hl_from_reg(n, s);
					return 1;
				}
				if (v > 8) {
					opcode("mov a,c");
					repeated_op("add a", v - 8);
					opcode("mov b,a");
					opcode("mvi c,0");
					hl_from_reg(n, s);
					return 1;
				}
				/* 16bit full shifting */
				hl_from_reg(NULL, s);
				repeated_op("dad h", v);
				bc_to_reg(s);
				return 1;
			}
			codegen_lr(r);
			helper(n, "bcshl");
			return 1;
		case T_SHREQ:
			if (r->op == T_CONSTANT) {
				if (v >= 8 && s == 1) {
					opcode("mvi c,0");
					hl_from_reg(n, s);
					return 1;
				}
				if (v >= 16) {
					load_bc(0);
					hl_from_reg(n, s);
					return 1;
				}
				if (v == 8 && (n->type & UNSIGNED)) {
					opcode("mov c,b");
					opcode("mvi b,0");
					hl_from_reg(n, s);
					return 1;
				}
				if (s == 2 && !(n->type & UNSIGNED) && cpu == 8085 && v < 2 + 4 * opt) {
					hl_from_reg(NULL,s);
					repeated_op("arhl", v);
					invalidate_hl();
					bc_to_reg(s);
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
	case 2:
		op_pushhl();
		return 1;
	case 4:
		if (optsize) {
			opcode("call __pushl");
			invalidate_hl();
		} else {
			op_xchg();
			opcode("lhld __hireg");
			invalidate_hl();	/* TODO: track hireg ? */
			op_pushhl();
			opcode("push d");
		}
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
	if (hl_valid == 1)
		load_hl(hl_value & 0xFF);
	else
		opcode("mvi h,0");
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned v;
	char *name;
	unsigned nr = n->flags & NORETURN;
	unsigned se = n->flags & SIDEEFFECT;
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
		if (!se) {
			if (nr)
				return 1;
			if (hl_contains(n))
				return 1;
			if (bc_contains(n)) {
				hl_from_reg(n, size);
				return 1;
			}
		}
		if (size == 1) {
			opcode("lda _%s+%u", namestr(n->snum), v);
			opcode("mov l,a");
			set_hl_node(n);
		} else if (size == 2) {
			opcode("lhld _%s+%u", namestr(n->snum), v);
			set_hl_node(n);
			return 1;
		} else if (size == 4) {
			opcode("lhld _%s+%u", namestr(n->snum), v + 2);
			opcode("shld __hireg");
			opcode("lhld _%s+%u", namestr(n->snum), v);
			set_hl_node(n);
		} else
			error("nrb");
		return 1;
	case T_LBREF:
		if (!se) {
			if (nr)
				return 1;
			if (hl_contains(n))
				return 1;
			if (bc_contains(n)) {
				hl_from_reg(n, size);
				return 1;
			}
		}
		if (size == 1) {
			opcode("lda T%u+%u", n->val2, v);
			opcode("mov l,a");
			set_hl_node(n);
		} else if (size == 2) {
			opcode("lhld T%u+%u", n->val2, v);
			set_hl_node(n);
		} else if (size == 4) {
			opcode("lhld T%u+%u", n->val2, v + 2);
			opcode("shld __hireg");
			opcode("lhld T%u+%u", n->val2, v);
			set_hl_node(n);
		} else
			error("lbrb");
		return 1;
	case T_LREF:
		/* We are loading something then not using it, and it's local
		   so can go away */
		/* printf(";L sp %u %s(%ld)\n", sp, namestr(n->snum), n->value); */
		if (!se) {
			if (nr)
				return 1;
			if (hl_contains(n))
				return 1;
			if (bc_contains(n)) {
				hl_from_reg(n, size);
				return 1;
			}
		}
		v += sp;
		if (gen_lref(v, size, 0)) {
			set_hl_node(n);
			return 1;
		}
		return 0;
	case T_RREF:
		if (nr)
			return 1;
		if (bc_contains(n))
			return 1;
		if (size == 2)
			hl_from_reg(n, size);
		else {
			/* We could do better for constants TODO */
			opcode("mov l,c");
			invalidate_hl();
		}
		return 1;
	case T_NSTORE:
		if (!se) {
			if (nr)
				return 1;
			if (hl_contains(n))
				return 1;
		}
		if (size == 4) {
			opcode("shld %s+%u", namestr(n->snum), v);
			op_xchg();
			opcode("lhld __hireg");
			invalidate_hl();
			opcode("shld %s+%u", namestr(n->snum), v + 2);
			op_xchg();
			set_hl_node(n);
			return 1;
		}
		if (size == 1) {
			opcode("mov a,l");
			opcode("sta _%s+%u", namestr(n->snum), v);
		} else
			opcode("shld _%s+%u", namestr(n->snum), v);
		set_hl_node(n);
		return 1;
	case T_LBSTORE:
		if (!se) {
			if (nr)
				return 1;
			if (hl_contains(n))
				return 1;
		}
		if (size == 4) {
			opcode("shld T%u+%u", n->val2, v);
			op_xchg();
			opcode("lhld __hireg");
			opcode("shld T%u+%u",	n->val2, v + 2);
			op_xchg();
			set_hl_node(n);
			return 1;
		}
		if (size == 1) {
			opcode("mov a,l");
			opcode("sta T%u+%u", n->val2, v);
		} else
			opcode("shld T%u+%u", n->val2, v);
		set_hl_node(n);
		return 1;
	case T_LSTORE:
/*		printf(";L sp %u spval %u %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		if (!se) {
			if (nr)
				return 1;
			if (hl_contains(n))
				return 1;
		}
		v += sp;
		if (v == 0 && size == 2 ) {
			if (nr && 0) {
				/* Unclear a win so skip versus value live */
				opcode("xthl");
				invalidate_hl();
			} else {
				opcode("pop psw");
				op_pushhl();
				set_hl_node(n);
			}
			return 1;
		}
		if (cpu == 8085 && v <= 255) {
			opcode("ldsi %u", v);
			invalidate_de();
			if (size == 2)
				opcode("shlx");
			else {
				opcode("mov a,l");
				opcode("stax d");
			}
			set_hl_node(n);
			return 1;
		}
		if (v == 2 && size == 2) {
			invalidate_de();
			opcode("pop d");
			if (nr && 0) {
				opcode("xthl");
				invalidate_hl();
			} else {
				opcode("pop psw");
				op_pushhl();
				set_hl_node(n);
			}
			opcode("push d");
			return 1;
		}
		/* Large offsets for word on 8085 are 7 bytes, a helper call is 5 (3 with rst hacks)
		   and much slower. As these are fairly rare just inline it */
		if (cpu == 8085 && size == 2) {
			op_xchg();
			load_hl_spoff(v);
			op_xchg();
			opcode("shlx");
			set_hl_node(n);
			return 1;
		}
		if (size == 1 && (!optsize || v >= LWDIRECT)) {
			opcode("mov a,l");
			load_hl_spoff(v);
			opcode("mov m,a");
			if (!nr) {
				opcode("mov l,a");
				set_hl_node(n);
			}
			return 1;
		}
		/* For -O3 they asked for it so inline the lot */
		/* We dealt with size one above */
		if (opt > 2 && size == 2) {
			op_xchg();
			load_hl_spoff(v);
			opcode("mov m,e");
			inx_h();
			opcode("mov m,d");
			if (!nr) {
				op_xchg();
				set_hl_node(n);
			}
			return 1;
		}
		/* Via helper magic for compactness on 8080 */
		/* Can rewrite some of them into rst if need be */
		if (size == 1)
			name = "stbyte";
		else if (size == 2)
			name = "stword";
		/* FIXME */
		else
			return 0;
		/* Like load the helper is offset by two because of the
		   stack */
		if (v < 24)
			opcode("call __%s%u", name, v + 2);
		else if (v < 253) {
			opcode("call __%s", name);
			opcode(".byte %u\n", v + 2);
		} else {
			opcode("call __%sw", name);
			opcode(".word %u", v + 2);
		}
		invalidate_de();
		set_hl_node(n);
		return 1;
	case T_RSTORE:
		bc_to_reg(size);
		return 1;
		/* Call a function by name */
	case T_CALLNAME:
		opcode("call _%s+%u", namestr(n->snum), v);
		invalidate_all();
		return 1;
	case T_EQ:
		flush_writeback();
		if (size == 2) {
			if (cpu == 8085) {
				opcode("pop d");
				invalidate_de();
				opcode("shlx");
			} else {
				op_xchg();
				op_pophl();
				opcode("mov m,e");
				inx_h();
				opcode("mov m,d");
				if (!nr)
					op_xchg();
			}
			return 1;
		}
		if (size == 1) {
			opcode("pop d");
			invalidate_de();
			op_xchg();
			opcode("mov m,e");
			if (!nr)
				op_xchg();
			return 1;
		}
		break;
	case T_RDEREF:
		/* RREFs on 8080 will always be byte pointers */
		if (nr && !se)
			return 1;
		opcode("ldax b");
		if (!(n->flags & NORETURN)) {
			opcode("mov l,a");
			invalidate_hl();
		}
		return 1;
	case T_DEREF:
		if (nr && !se)
			return 1;
		if (size == 2) {
			if (cpu == 8085) {
				op_xchg();
				opcode("lhlx");
				invalidate_hl();
			} else {
				opcode("mov e,m");
				inx_h();
				invalidate_hl();
				invalidate_de();
				opcode("mov d,m");
				op_xchg();
			}
			return 1;
		}
		if (size == 1) {
			opcode("mov l,m");
			invalidate_hl();
			return 1;
		}
		if (size == 4 && cpu == 8085 && !optsize) {
			op_xchg();
			inx_d();
			inx_d();
			opcode("lhlx");
			opcode("shld __hireg");
			dcx_d();
			dcx_d();
			opcode("lhlx");
			invalidate_hl();
			return 1;
		}
		break;
	case T_FUNCCALL:
		opcode("call __callhl");
		invalidate_all();
		return 1;
	case T_LABEL:
		/* ?? Do we need to clear hireg on size 4 for label/name etc */
		if (nr)
			return 1;
		/* Used for const strings and local static */
		opcode("lxi h,T%u+%u", n->val2, v);
		set_hl_node(n);
		return 1;
	case T_CONSTANT:
		if (nr)
			return 1;
		switch(size) {
		case 4:
			load_hl((n->value >> 16) & 0xFFFF);
			opcode("shld __hireg");
		case 2:
			load_hl(WORD(v));
			return 1;
		case 1:
			hl_value &= 0xFF00;
			hl_value |= v & 0xFF;
			opcode("mvi l,%u", v & 0xFF);
			return 1;
		}
		break;
	case T_NAME:
		if (nr)
			return 1;
		opcode("lxi h, _%s+%u", namestr(n->snum), v);
		set_hl_node(n);
		return 1;
	case T_LOCAL:
		if (nr)
			return 1;
		v += sp;
/*		printf(";LO sp %u spval %u %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		ldsi_hl(v);
		set_hl_node(n);
		return 1;
	case T_ARGUMENT:
		if (nr)
			return 1;
		v += frame_len + argbase + sp;
/*		printf(";AR sp %u spval %u %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		ldsi_hl(v);
		set_hl_node(n);
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
		if (size <= 2) {
			invalidate_de();
			opcode("pop d");
			opcode("dad d");
			invalidate_hl();
			return 1;
		}
		break;
	}
	return 0;
}
