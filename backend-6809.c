#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */


/*
 *	Private types we rewrite things into
 */
#define T_CALLNAME	(T_USER)
#define T_NREF		(T_USER+1)
#define T_NSTORE	(T_USER+2)
#define T_LREF		(T_USER+3)
#define T_LSTORE	(T_USER+4)

static void squash_node(struct node *n, struct node *o)
{
	n->value = o->value;
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
 *	Our chance to do tree rewriting. We don't do much for the 8080
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *r = n->right;
	struct node *l = n->left;
	/* Rewrite references into a load or store operation */
	if (n->type == CINT || n->type == UINT || PTR(n->type)) {
		if (n->op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += 2 + frame_len;
				squash_right(n, T_LREF);
				return n;
			}
			if (r->op == T_NAME) {
				squash_right(n, T_NREF);
				return n;
			}
		}
		if (n->op == T_EQ) {
			if (l->op == T_LOCAL || l->op == T_ARGUMENT) {
				if (l->op == T_ARGUMENT)
					l->value += 2 + frame_len;
				squash_left(n, T_LSTORE);
				return n;
			}
			if (l->op == T_NAME) {
				squash_left(n, T_NSTORE);
				return n;
			}
		}
		/* We can turn some operators into simpler nodes for
		   code generation */
		if (n->op == T_PLUSPLUS || n->op == T_MINUSMINUS) {
			/* left is a constant scaled value, right is
			   the lval */
			n->val2 = l->value;
			free_node(l);
			n->left = NULL;
		}
	}
	/* Rewrite function call of a name into a new node so we can
	   turn it easily into call xyz */
	if (n->op == T_FUNCCALL && r->op == T_NAME) {
		n->op = T_CALLNAME;
		n->snum = r->snum;
		n->value = r->value;
		free_node(r);
		n->right = NULL;
	}
	return n;
}

/* Export the C symbol */
void gen_export(const char *name)
{
	printf("	.export _%s\n", name);
}

void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
}

/* Generate the stack frame */
void gen_frame(unsigned size)
{
	frame_len = size;
	if (size) {
		sp = 0;	/* Stack is relative to bottom of frame */
		printf("\tleas -%d,s\n", size);
	}
}

void gen_epilogue(unsigned size)
{
	if (sp)
		error("sp");
	if (size)
		printf("\tleas %d,a\n", size);
	printf("\trts\n");
}

void gen_label(const char *tail, unsigned n)
{
	printf("L%d%s:\n", n, tail);
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\tjmp L%d%s\n", n, tail);
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\tjz L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjnz L%d%s\n", n, tail);
}

/* This is still being worked on */
void gen_switch_begin(unsigned n, unsigned type)
{
}

void gen_switch(unsigned n)
{
}

void gen_case(unsigned type)
{
}

/* Output whatever goes in front of a helper call */
void gen_helpcall(void)
{
	printf("\tjsr ");
}

/* TODO: Need to pass alignment */
void gen_data(const char *name)
{
	printf("\t.data\n");
	printf("_%s:\n", name);
}

/* TODO: Need to pass alignment */
void gen_bss(const char *name)
{
	printf("\t.data\n");
	printf("_%s:\n", name);
}

void gen_code(void)
{
	printf("\t.code\n");
}

void gen_space(unsigned value)
{
	printf("\t.ds %d\n", value);
}

void gen_text_label(unsigned n)
{
	printf("\t.word T%d\n", n);
}

void gen_literal(unsigned n)
{
	printf("\t.data\n");
	printf("T%d:\n", n);
}

void gen_name(struct node *n)
{
	printf("\t.word _%s+%d\n", namestr(n->snum), n->value);
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
	case CINT:
	case UINT:
		printf("\t.word %d\n", (unsigned) value & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
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

/*
 *	We can push bytes so everything is native sized
 */
static unsigned get_size(unsigned t)
{
	if (PTR(t))
		return 2;
	if (t == CINT || t == UINT)
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

unsigned gen_push(struct node *n)
{
	unsigned s = get_stack_size(n->type);
	/* Our push will put the object on the stack, so account for it */
	sp += s;
	if (s == 1)
		printf("\tpshs b\n");
	else if (s == 2)
		printf("\tpshs d\n");
	else if (s == 4)	/* TODO check this orders right if
				   we use it for auto inits */
		printf("\tpshs d,u\n");
	else
		return 0;
	return 1;
}

unsigned constval(struct node *n)
{
	if (n->op == T_CONSTANT || n->op == T_LABEL || n->op == T_NAME)
		return 1;
	return 0;
}

/* Turn a constant node into a string - for char/int/ptr */

unsigned gen_constop(const char *op, char r, struct node *n)
{
	unsigned s;
	s = get_size(n->type);
	if (s > 2)
		return 0;
	if (r == 0) {
		r = 'd';
		if (s == 1)
			r = 'b';
	}

	printf(";try to constop %s %x\n", op, n->op);
	/* Objects we know how to directly access */
	switch(n->op) {
	case T_CONSTANT:
		printf("\t%s%c #%d\n", op, r, n->value);
		return 1;
	case T_NAME:
		printf("\t%s%c #%s+%d\n", op, r, namestr(n->snum), n->value);
		return 1;
	case T_LABEL:
		printf("\t%s%c #T%d\n\n", op, r, n->value);
		return 1;
	case T_NREF:
		printf("\t%s%c %s + %d\n", op, r, namestr(n->snum), n->value);
		return 1;
	case T_LREF:
		printf("\t%s%c %d(s)\n", op, r, n->value);
		return 1;
	case T_LOCAL:
		printf("\t%s%c %d(s)\n", op, r, n->value + sp);
		return 1;
	case T_ARGUMENT:
		printf("\t%s%c %d(s)\n", op, r, n->value + frame_len + sp);
		return 1;
	}
	return 0;
}

unsigned can_constop(struct node *n)
{
	if (get_size(n->type) > 2)
		return 0;
	if (n->op == T_CONSTANT || n->op == T_NAME || n->op == T_LABEL ||
		n->op == T_NREF || n->op == T_LREF || n->op == T_LOCAL ||
		n->op == T_ARGUMENT)
		return 1;
	return 0;
}

unsigned gen_constpair(const char *op, struct node *n)
{
	unsigned s;
	unsigned v;
	s = get_size(n->type);
	if (s == 1)
		return gen_constop(op, 0, n);

	v = n->value;

	/* Generate pair forms */
	switch(n->op) {
	case T_CONSTANT:
		printf("\t%sa #%d\n", op, (v >> 8) & 0xFF);
		printf("\t%sb #%d\n", op, (v & 0xFF));
		return 1;
	case T_NAME:
		printf("\t%sa >#%s+%d\n", op, namestr(n->snum), v);
		printf("\t%sb <#%s+%d\n", op, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		printf("\t%sa >#T%d\n\n", op, v);
		printf("\t%sa <#T%d\n\n", op, v);
		return 1;
	case T_NREF:
		printf("\t%sa %s + %d\n", op, namestr(n->snum), v);
		printf("\t%sb %s + %d\n", op, namestr(n->snum), v + 1);
		return 1;
	case T_LREF:
		printf("\t%sa %d(s)\n", op, v);
		printf("\t%sb %d(s)\n", op, v + 1);
		return 1;
	case T_LOCAL:
		printf("\t%sa %d(s)\n", op, v + sp);
		printf("\t%sb %d(s)\n", op, v + sp + 1);
		return 1;
	case T_ARGUMENT:
		printf("\t%sa %d(s)\n", op, v + frame_len + sp);
		printf("\t%sb %d(s)\n", op, v + frame_len + sp + 1);
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
	struct node *r = n->right;
	char reg = 'd';
	int v2 = n->val2;
	unsigned s = get_size(n->type);

	if (s == 1)
		reg = 'b';

	switch(n->op) {
	case T_MINUSMINUS:
		v2 = -v2;
	case T_PLUSPLUS:
		if (!gen_constop("ld", 0, r))
			break;
		printf("\tadd%c #%d\n", reg, v2);
		gen_constop("st", 0, r);
		if (!(n->flags & NORETURN))
			printf("\tsub%c #%d\n", reg, v2);
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
	unsigned v;
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	char *ldop = "ldd";
	char *stop = "std";

	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	if (n->op == T_CLEANUP) {
		v = r->value;
		if (v) {
			printf("\tleas %d,s\n", v);
			sp -= v;
		}
		return 1;
	}

	if (r == NULL)
		return 0;
	v = r->value;
	if (s == 1) {
		ldop = "ldb";
		stop = "stb";
	}

	switch(n->op) {
	case T_NSTORE:
		if (s == 1 || s == 2) {
			printf("\t%s #%d\n", ldop, v);
			printf("\t%s _%s+%d\n", stop, namestr(n->snum), n->value);
			return 1;
		}
		if (s == 4) {
			printf("\tldd #%d\n", v & 0xFFFF);
			printf("\tldu #%d\n", (v >> 16));
			printf("\tstu _%s+%d\n", namestr(n->snum), n->value);
			printf("\tstd _%s+%d\n", namestr(n->snum), n->value + 2);
			return 1;
		}
		break;
	case T_LSTORE:
		if (s == 4) {
			printf("\tldd #%d\n", v & 0xFFFF);
			printf("\tldu #%d\n", (v >> 16));
			printf("\tldu %d(s)\n", n->value);
			printf("\tldd %d(s)\n", n->value + 2);
			return 1;
		}
		if (s == 1 || s == 2) {
			printf("\t%s #%d\n", ldop, v);
			printf("\t%s %d(s)\n", stop, n->value);
			return 1;
		}
		break;
	case T_EQ:
		if (!constval(r))
			return 0;
		printf("\ttfr d,x\n");
		switch(s) {
		case 1:
			if (v == 0) {
				printf("\tclr (x)\n");
				return 1;
			}
		case 2:
			printf("\t%s #%d\n", ldop, v);
			printf("\t%s (x)\n", stop);
			return 1;
		case 4:
			printf("\tldu #%d\n", (v >> 16));
			printf("\tstu (x)\n");
			printf("\tldd #%d\n", v & 0xFFFF);
			printf("\tstd 2(x)\n");
			return 1;
		default:
			return 0;
		}
		break;
	case T_PLUS:
		return gen_constop("add", 0, r);
	case T_MINUS:
		return gen_constop("sub", 0, r);
	case T_AND:
		return gen_constpair("and", r);
	case T_OR:
		return gen_constpair("or", r);
	case T_HAT:
		return gen_constpair("eor", r);
	case T_TILDE:
		return gen_constpair("com", r);
	case T_NEGATE:
		if (s == 1)
			return gen_constpair("neg", r);
		if (gen_constpair("com", r) == 0)
			return 0;
		printf("\taddd #1\n");
		return 1;
	}
	return 0;
}

unsigned gen_shortcut(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned s = get_size(n->type);

	if (n->op == T_PLUSEQ) {
		if (!gen_constop("lea", 'x', l))
			return 0;
		if (!gen_constop("ld", 0, r))
			return 0;
		if (s == 1) {
			printf("\taddb (x)\n");
			printf("\tstb (x)\n");
		} else {
			printf("\taddd (x)\n");
			printf("\tstdd (x)\n");
		}
		return 1;
	}
	if (n->op == T_MINUSEQ) {
		if (!can_constop(r))
			return 0;
		if (!gen_constop("lea", 'x', l))
			return 0;
		/* Load by size helper ? */
		if (s == 1)
			printf("\tldb (x)\n");
		else
			printf("\tldd (x)\n");
		gen_constop("sub", 0, r);
		if (s == 1)
			printf("\tstb (x)\n");
		else
			printf("\tstdd (x)\n");
		return 1;
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned s;
	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_COMMA && n->op != T_FUNCCALL && n->op != T_CALLNAME)
		sp -= get_stack_size(n->left->type);

	s = get_size(n->type);

	switch(n->op) {
	case T_NREF:	/* Get the value held in a name */
		if (s == 1) {
			printf("\tldb _%s+%d\n", namestr(n->snum), n->value);
			return 1;
		}
		if (s == 2) {
			printf("\tldd _%s+%d\n", namestr(n->snum), n->value);
			return 1;
		}
		if (s == 4) {
			printf("\tldu _%s+%d\n", namestr(n->snum), n->value);
			printf("\tldd _%s+%d\n", namestr(n->snum), n->value + 2);
			return 1;
		}
		break;
	case T_LREF:
		if (s == 4) {
			printf("\tldu %d(s)\n", n->value);
			printf("\tldd %d(s)\n", n->value + 2);
			return 1;
		}
		if (s == 2) {
			printf("\tldd %d(s)\n", n->value);
			return 1;
		}
		if (s == 1) {
			printf("\tldb %d(s)\n", n->value);
			return 1;
		}
		break;
	case T_NSTORE:
		if (s == 1) {
			printf("\tstb _%s+%d\n", namestr(n->snum), n->value);
			return 1;
		}
		if (s == 2) {
			printf("\tstd _%s+%d\n", namestr(n->snum), n->value);
			return 1;
		}
		if (s == 4) {
			printf("\tstu _%s+%d\n", namestr(n->snum), n->value);
			printf("\tstd _%s+%d\n", namestr(n->snum), n->value + 2);
			return 1;
		}
		break;
	case T_LSTORE:
		if (s == 4) {
			printf("\tldu %d(s)\n", n->value);
			printf("\tldd %d(s)\n", n->value + 2);
			return 1;
		}
		if (s == 2) {
			printf("\tstdd %d(s)\n", n->value);
			return 1;
		}
		if (s == 1) {
			printf("\tstb %d(s)\n", n->value);
			return 1;
		}
		break;
	case T_CALLNAME:	/* Call function by name */
		printf("\tjsr _%s+%d\n", namestr(n->snum), n->value);
		return 1;
	case T_EQ:
		printf("\tldx (-s)\n");
		if (s == 1)
			printf("\tstb (x)\n");
		else if (s == 2)
			printf("\tstd (x)\n");
		else if (s == 4) {
			printf("\tstd 2(x)\n");
			printf("\tstu (x)\n");
		} else
			return 0;
		return 1;
	case T_DEREF:
		printf("\ttfr d,x\n");
		printf("\tldd (x)\n");
		return 1;
	case T_LABEL:
		printf("\tldd T%d\n", n->value);
		return 1;
	case T_CONSTANT:
		switch(s) {
		case 1:
			printf("\tldb #%d\n", n->value);
			return 1;
		case 4:
			printf("\tldu #%d\n", (n->value >> 16) & 0xFFFF);
		case 2:
			printf("\tldd #%d\n", n->value & 0xFFFF);
			return 1;
		}
		break;
	case T_NAME:
		printf("ldd #%s+%d\n", namestr(n->snum), n->value);
		return 1;
	case T_LOCAL:
		/* FIXME: correct offsets */
		printf("\tleax %d,s\n", n->value + sp);
		/* Will need a lot of peepholing */
		printf("\ttfr x,d\n");
		return 1;
	case T_ARGUMENT:
		/* FIXME: correct offsets */
		printf("\tleax %d,s\n", n->value + frame_len + 2 + sp);
		/* Will need a lot of peepholing */
		printf("\ttfr x,d\n");
		return 1;
	/* We rewrote these into a new form so must handle them. We can also
	   do better probably TODO .. */
	case T_PLUSPLUS:
		helper(n, "preinc");
		printf("\t.word %d\n", n->val2);
		return 1;
	case T_MINUSMINUS:
		helper(n, "predec");
		printf("\t.word -%d\n", n->val2);
		return 1;
	}
	return 0;
}
