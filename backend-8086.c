#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

/*
 *	TODO: assembler sort out and add syntax annotations for word
 *	ops mem,const where size is not implied
 *	- Optimize various mem,const cases we can use
 *	- Track values and pointers for writeback supression
 *	- Volatile/elimination of nstore etc
 *	- Conditions
 *	- CCONLY handling
 *	- XOR for constant 0 set
 *	- Register variables using SI and DI and maybe CX as we could
 *	  push it for the few cases we use it
 *	- Shift eq ops (>>= etc)
 *	- 32bit math helpers
 *	- *= /= %=
 *	- Boolean ops
 */
#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned argbase = 4;	/* Will vary once we add reg vars */
static unsigned unreachable;
static unsigned labelid;

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

static unsigned get_label(void)
{
	return ++labelid;
}

#define T_CALLNAME	(T_USER+0)		/* Function call by name */
#define T_NREF		(T_USER+1)		/* Load of C global/static */
#define T_LBREF		(T_USER+2)		/* Ditto for labelled strings or local static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_NSTORE	(T_USER+4)		/* Store to a C global/static */
#define T_LBSTORE	(T_USER+5)
#define T_LSTORE	(T_USER+6)

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
 *	Our chance to do tree rewriting. We don't do much
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 *
 *	TODO: lots of other rewrites will help from the other targets
 *	eg EQPLUS. Also we want to rewrite *x++ and *x-- type ops on a local
 *	that do something to the result to a single op so we can generate
 *	mov bx,n[bp],add 1,n[bp] mov [bx],0 etc
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned op = n->op;

	if (op == T_DEREF) {
		if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
			if (r->op == T_ARGUMENT)
				r->value += argbase + frame_len;
			squash_right(n, T_LREF);
			return n;
		}
#if 0
		if (r->op == T_REG) {
			squash_right(n, T_RREF);
			return n;
		}
#endif
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
#if 0
		if (l->op == T_REG) {
			squash_left(n, T_RSTORE);
			return n;
		}
#endif
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
	unreachable = 0;
	printf("_%s:\n", name);
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size;
	/* TODO: push si/di if reg args */
	printf("\tpush bp\n");
	printf("\tmov bp,sp\n");
	if (size)
		printf("\tsub sp,%d\n", size);
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	/* TODO: pop si/di if reg args */
	if (!unreachable) {
		printf("\tmov sp,bp\n");
		printf("\tpop bp\n");
		printf("\tret\n");
	}
	unreachable = 1;
}

void gen_label(const char *tail, unsigned n)
{
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
	printf("\tjmp L%d%s\n", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\tjz L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjnz L%d%s\n", n, tail);
}

void gen_switch(unsigned n, unsigned type)
{
	gen_helpcall(NULL);
	printf("switch");
	helper_type(type, 0);
	printf("\n\t.word Sw%d\n", n);
	unreachable = 1;
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

void gen_helpcall(struct node *n)
{
	printf("\tcall __");
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
	unsigned s = get_stack_size(n->type);
	if (s == 4)
		printf("\npush dx\n");
	printf("\tpush ax\n");
	return 1;
}

/* Perform an operation between (DX:)AX and the right hand side directly
   if possible. */
unsigned op_direct(struct node *r, unsigned size, const char *op, const char *op2)
{
	const char *lr = "ax";
	const char *name;
	if (size == 1)
		lr = "al";
	unsigned v = r->value;

	/* We can't shortcut floats like this */
	if (r->type == FLOAT)
		return 0;

	switch(r->op) {
	case T_CONSTANT:
		if (size == 1) {
			printf("\t%s al,%u\n", op, v & 0xFF);
			return 1;
		}
		printf("\t%s ax,%u\n", op, v);
		if (size == 4)
			printf("\t%s dx,%u\n", op2, (unsigned)((r->value >> 16) & 0xFFFF));
		return 1;
	case T_NREF:
		name = namestr(r->snum);
		printf("\t%s %s,_%s+%u\n", op, lr, name, v);
		if (size == 4)
			printf("\t%s dx,_%s+%u\n", op2, name, v + 2);
		return 1;
	case T_LBREF:
		printf("\t%s %s,[T%u+%u]\n", op, lr, r->val2, v);
		if (size == 4)
			printf("\t%s dx,[T%u+%u]\n", op2, r->val2, v + 2);
		return 1;
	case T_LREF:
		printf("\t%s %s,%u[bp]\n", op, lr, v);
		if (size == 4)
			printf("\t%s dx,%u[bp]\n", op2, v + 2);
		return 1;
	case T_NAME:
		name = namestr(r->snum);
		if (size == 1) {
			printf("\t%s al,<_%s+%u\n", op, name, v);
			return 1;
		}
		printf("\t%s ax,_%s+%u\n", op, name, v);
		if (size == 4)
		 	printf("\t%s dx,0\n", op2);
		return 1;
	case T_LABEL:
		if (size == 1) {
			printf("\t%s al,T%u+%u\n", op, r->val2, v);
			return 1;
		}
		printf("\t%s ax,T%u+%u\n", op, r->val2, v);
		if (size == 4)
		 	printf("\t%s dx,0\n", op2);
		return 1;
	}
	return 0;
}

/* nr is 0 if we need the result, iv is set if the result is the original
   value (x++ and x-- cases) */
unsigned bx_operation(unsigned size, const char *op, const char *op2,
	unsigned nr, unsigned iv)
{
	/* R has been evaluated, L is in BX */

	if (size == 1) {
		if (!nr && iv)
			printf("\tld ah,[bx]\n");
		printf("\t%s [bx],al\n", op);
		if (!nr) {
			if (iv)
				printf("\tmov al,ah\n");
			else
				printf("\tmov al,[bx]\n");
		}
	} else if (size == 2) {
		if (!nr && iv)
			printf("\tmov dx,[bx]\n");
		printf("\t%s [bx],ax\n", op);
		if (!nr) {
			if (iv)
				printf("\tmov ax,dx\n");
			else
				printf("\tmov ax,[bx]\n");
		}
	/* Size 4 never done with iv */
	} else if (size == 4) {
		printf("\t%s [bx],ax\n", op);
		printf("\t%s 2[bx],dx\n", op2);
		if (!nr) {
			printf("\tmov ax,[bx]\n");
			printf("\tmov dx,2[bx]\n");
		}
	}
	return 1;
}

/* Stack relative operations (eg ++ on a local) */
unsigned bp_operation(struct node *n, const char *op, const char *op2,
	unsigned nr, unsigned iv)
{
	unsigned size = get_size(n->type);
	unsigned v = n->value;

	/* R has been evaluated, L is in BX */
	if (n->left->op == T_ARGUMENT)
		v += argbase + frame_len;
	if (size == 1) {
		if (!nr && iv)
			printf("\tld ah,%u[bp]\n", v);
		printf("\t%s %u[bp],al\n", op, v);
		if (!nr) {
			if (iv)
				printf("\tmov al,ah\n");
			else
				printf("\tmov al,%u[bp]\n", v);
		}
	} else if (size == 2) {
		if (!nr && iv)
			printf("\tmov dx,%u[bp]\n", v);
		printf("\t%s %u[bp],ax\n", op, v);
		if (!nr) {
			if (iv)
				printf("\tmov ax,dx\n");
			else
				printf("\tmov ax,%u[bp]\n", v);
		}
	/* Size 4 never done with iv */
	} else if (size == 4) {
		printf("\t%s %u[bp],ax\n", op, v);
		printf("\t%s %u[bp],dx\n", op2, v + 2);
		if (!nr) {
			printf("\tmov ax,%u[bp]\n", v);
			printf("\tmov dx,%u[bp]\n", v + 2);
		}
	}
	return 1;
}

unsigned can_load_bx(struct node *n)
{
	switch(n->op) {
	case T_CONSTANT:
	case T_NAME:
	case T_LABEL:
	case T_LOCAL:
	case T_ARGUMENT:
	case T_LREF:
	case T_LBREF:
	case T_NREF:
		return 1;
	}
	return 0;
}

void load_bx(struct node *n)
{
	unsigned v = n->value;
	switch(n->op) {
	case T_CONSTANT:
		printf("\tmov bx,%u\n", v);
		break;
	case T_NAME:
		printf("\tmov bx,_%s+%u\n", namestr(n->snum), v);
		break;
	case T_LABEL:
		printf("\tmov bx,T%u+%u\n", n->val2, v);
		break;
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		printf("\tlea bx,%u[bp]\n", v);
		break;
	case T_LREF:
		printf("\tmov bx,%u[bp]\n", v);
		break;
	case T_LBREF:
		printf("\tmov bx,[T%u+%u]\n", n->val2, v);
		break;
	case T_NREF:
		printf("\tmov bx,[_%s+%u]\n", namestr(n->snum), v);
		break;
	}
}

/* TODO: we need a shortcut form because we can do
	cmp 4[bp],0 and the like for const + simple forms
	also rewrites to turn 0, 4[bp] into 4[bp],0 and switch
	the tests (lteq to gteq etc) */
/* Use the type of the arguments as the type of the result is the
   boolean */
unsigned cond_direct(struct node *n, const char *opu, const char *op)
{
	struct node *r = n->right;
	unsigned size = get_size(r->type);

	/* Need to chain the right comparisons including signed and
	   unsigned parts. Do this out of line */
	if (size == 4)
		return 0;

	if (r->type & UNSIGNED)
		op = opu;
	/* First try it directly */
	/* We ought to consider reversing it, but that's probably best
	   as a rewrite rule when we add re-ordering to the rewrite
	   rules TODO */
	if (!op_direct(r, size,  "cmp", "cmp"))
		return 0;
	/* Until we have CCONLY sorted */
	printf("\tcall __cc%s\n", op);
	n->flags |= ISBOOL;
	return 1;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 */
unsigned gen_direct(struct node *n)
{
	struct node *r = n->right;
	unsigned size = get_size(n->type);
//	unsigned nr = n->flags & NORETURN;
//	unsigned v = n->value;

	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		if (r->value)
			printf("\tadd sp,%u\n", (unsigned)(r->value & 0xFFFF));
		return 1;
	case T_PLUS:
		return op_direct(r, size, "add", "adc");
	case T_MINUS:
		return op_direct(r, size, "sub", "sbc");
	case T_AND:
		return op_direct(r, size, "and", "and");
	case T_OR:
		return op_direct(r, size, "and", "and");
	case T_HAT:
		return op_direct(r, size, "and", "and");
	/* 80186 has const multishift TODO */
	case T_EQEQ:
		return cond_direct(n, "z", "z");
	case T_BANGEQ:
		return cond_direct(n, "nz", "nz");
	case T_LTEQ:
		return cond_direct(n, "be", "le");
	case T_GTEQ:
		return cond_direct(n, "ae", "ge");
	case T_LT:
		return cond_direct(n, "b", "lt");
	case T_GT:
		return cond_direct(n, "a", "gt");
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
	unsigned size = get_size(n->type);
	unsigned nr = n->flags & NORETURN;
	unsigned v = n->value;
	unsigned rv;

	switch(n->op) {
	case T_LSTORE:
		if (nr && r->op == T_CONSTANT) {
			rv = r->value;
			switch(size) {
			case 1:
				printf("\tmov %u[bp],%u\n", v, rv & 0xFF);
				return 1;
			case 2:
				printf("\tmov %u[bp],%u\n", v, rv & 0xFFFF);
				return 1;
			case 4:
				printf("\tmov %u[bp],%u\n", v, rv & 0xFFFF);
				printf("\tmov %u[bp],%u\n", v + 2,
					((unsigned)(r->value >> 16) & 0xFFFF));
				return 1;
			}
		}
		return 0;
	}
	return 0;
}

/* Try and do the pluseq and other ops by evaluating the right then loading
   the left into bx */
unsigned bx_shortcut(struct node *n, const char *op, const char *op2,
	unsigned nr, unsigned iv)
{
	unsigned size = get_size(n->type);
	struct node *l = n->left;

	/* Can't do floats this way and for now don't deal with
	   PLUSPLUS/MINUSMINUS of dword */
	if (n->type == FLOAT)
		return 0;
	if (size == 4 && !nr && iv)
		return 0;

	/* TODO: optimize constant right forms of these by doing the
	   op between bx and a constant when possible */
	if (l->op == T_LOCAL || l->op == T_ARGUMENT) {
		codegen_lr(n->right);
		return bp_operation(n, op, op2, nr, iv);
	}
	if (can_load_bx(n->left)) {
		/* Get the value into DX:AX */
		codegen_lr(n->right);
		/* Will not damage DX:AX */
		load_bx(n->left);
		/* We are now doing ops on [bx] */
		return bx_operation(size, op, op2, nr, iv);
	}
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
	unsigned size = get_size(n->type);

	if (unreachable)
		return 1;

	switch(n->op) {
	case T_COMMA:
		/* The comma operator discards the result of the left side,
		   then evaluates the right. Avoid pushing/popping and
		   generating stuff that is surplus */
		l->flags |= NORETURN;
		codegen_lr(l);
		/* Parent determines child node requirements */
		codegen_lr(r);
		r->flags |= nr;
		return 1;
	case T_EQ:
		/* Need to look at (complex) = simple form later TODO */
		if (!can_load_bx(n->left))
			return 0;
		/* Optimised path for the usual simple = expression case */
		codegen_lr(n->right);
		load_bx(n->left);
		switch(size) {
		case 1:
			printf("\tmov [bx],al\n");
			return 1;
		case 4:
			printf("\tmov 2[bx],dx\n");
			/* Fall through */
		case 2:
			printf("\tmov [bx],ax\n");
			return 1;
		}
		return 0;
	case T_PLUSEQ:
		return bx_shortcut(n, "add", "adc", nr, 0);
	case T_MINUSEQ:
		return bx_shortcut(n, "add", "adc", nr, 0);
	case T_PLUSPLUS:
		return bx_shortcut(n, "add", "adc", nr, 1);
	case T_MINUSMINUS:
		return bx_shortcut(n, "add", "adc", nr, 1);
	case T_ANDEQ:
		return bx_shortcut(n, "and", "and", nr, 0);
	case T_OREQ:
		return bx_shortcut(n, "and", "and", nr, 0);
	case T_HATEQ:
		return bx_shortcut(n, "and", "and", nr, 0);
	/* SHLEQ/SHREQ */
	}
	return 0;
}

unsigned boolop(struct node *n, const char *opu, const char *op)
{
	struct node *r = n->right;
	unsigned size = get_size(r->type);
	if (r->type & UNSIGNED)
		op = opu;
	if (size == 4)
		return 0;
	printf("\tpop bx\n");
	if (size == 2)
		printf("\tcmp bx,ax\n");
	else
		printf("\tcmp bl,al\n");
	printf("\tcall __cc%s\n", op);
	n->flags |= ISBOOL;
	return 1;
}

unsigned gen_cast(struct node *n)
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
	if (!(rt & UNSIGNED)) {
		/* We have helpers yay! */
		if (rs == 1)
			printf("\tcbw\n");
		if (ls == 4)
			printf("\tcwd\n");
		return 1;
	}
	if (rs == 1)
		printf("\tmov ah,0\n");
	if (ls == 4)
		printf("\txor dx,dx\n");
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned nr = n->flags & NORETURN;
	unsigned v = n->value;
	const char *name;

	switch(n->op) {
	case T_CALLNAME:
		printf("\tcall_%s+%u\n", namestr(n->snum), v);
		return 1;
	case T_FUNCCALL:
		printf("\tmov bx,ax\n");
		printf("\tcall [bx]\n");
		return 1;
	case T_CAST:
		return gen_cast(n);
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		if (nr)
			return 1;
		printf("\tlea ax,%u[bp]\n", v);
		return 1;
	case T_LABEL:
		if (nr)
			return 1;
		printf("\tmov ax,T%u+%u\n", n->val2, v);
		return 1;
	case T_NAME:
		if (nr)
			return 1;
		printf("\tmov ax,_%s+%u\n", namestr(n->snum), v);
		return 1;
	case T_NREF:
		name = namestr(n->snum);
		switch(size) {
		case 1:
			printf("\tmov al,_%s+%u\n", name, v);
			return 1;
		case 2:
			printf("\tmov ax,_%s+%u\n", name, v);
			return 1;
		case 4:
			printf("\tmov ax,_%s+%u\n", name, v);
			printf("\tmov dx,_%s+%u\n", name, v +2);
			return 1;
		}
		break;
	case T_LBREF:
		if (nr)
			return 1;
		switch(size) {
		case 1:
			printf("\tmov al,T%u+%u\n", n->val2, v);
			return 1;
		case 2:
			printf("\tmov ax,T%u+%u\n", n->val2, v);
			return 1;
		case 4:
			printf("\tmov ax,T%u+%u\n", n->val2, v);
			printf("\tmov dx,T%u+%u\n", n->val2, v +2);
			return 1;
		}
		break;
	case T_LREF:
		if (nr)
			return 1;
		switch(size) {
		case 1:
			printf("\tmov al,%u[bp]\n", v);
			return 1;
		case 2:
			printf("\tmov ax,%u[bp]\n", v);
			return 1;
		case 4:
			printf("\tmov ax,%u[bp]\n", v);
			printf("\tmov dx,%u[bp]\n", v + 2);
			return 1;
		}
		break;
	case T_NSTORE:
		/* TODO direct const forms */
		name = namestr(n->snum);
		switch(size) {
		case 1:
			printf("\tmov _%s+%u, al\n", name, v);
			return 1;
		case 2:
			printf("\tmov _%s+%u, ax\n", name, v);
			return 1;
		case 4:
			printf("\tmov _%s+%u, ax\n", name, v);
			printf("\tmov _%s+%u, dx\n", name, v + 2);
			return 1;
		}
		break;
	case T_LBSTORE:
		/* TODO direct const forms */
		switch(size) {
		case 1:
			printf("\tmov T%u+%u, al\n", n->val2, v);
			return 1;
		case 2:
			printf("\tmov T%u+%u, ax\n", n->val2, v);
			return 1;
		case 4:
			printf("\tmov T%u+%u, ax\n", n->val2, v);
			printf("\tmov T%u+%u, dx\n", n->val2, v + 2);
			return 1;
		}
		break;
	case T_LSTORE:
		switch(size) {
		case 1:
			printf("\tmov %u[bp],al\n", v);
			return 1;
		case 2:
			printf("\tmov %u[bp],ax\n", v);
			return 1;
		case 4:
			printf("\tmov %u[bp],ax\n", v);
			printf("\tmov %u[bp],dx\n", v + 2);
			return 1;
		}
		break;
	case T_CONSTANT:
		/* TODO xor for 0 etc */
		if (nr)
			return 1;
		switch(size) {
		case 1:
			printf("\tmov al,%u\n", v & 0xFF);
			break;
		case 4:
			printf("\tmov dx,%u\n", (unsigned)((n->value >> 16) & 0xFFFF));
		case 2:
			printf("\tmov ax,%u\n", v);
		}
		return 1;
	case T_EQ:
		/* This will not get used much once all the optimziations are in,
		   just for complex expressions each side */
		printf("\tmov bx,ax\n");
		switch(size) {
		case 1:
			printf("\tmov [bx],al\n");
			break;
		case 2:
			printf("\tpop ax\n");
			printf("\tmov [bx],ax\n");
			break;
		case 4:
			printf("\tpop ax\n");
			printf("\tpop dx\n");
			printf("\tmov [bx],ax\n");
			printf("\tmov 2[bx],dx\n");
			break;
		}
		return 1;
	case T_DEREF:
		printf("\tmov bx,ax\n");
		switch(size) {
		case 1:
			printf("\tmov al,[bx]\n");
			break;
		case 2:
			printf("\tmov ax,[bx]\n");
			break;
		case 4:
			printf("\tmov ax,[bx]\n");
			printf("\tmov ax,2[bx]\n");
			break;
		}
		return 1;
	case T_PLUS:
		if (n->type == FLOAT)
			return 0;
		printf("\tpop bx\n");
		switch(size) {
		case 1:
			printf("\tadd al,bl\n");
			break;
		case 2:
			printf("\tadd ax,bx\n");
			break;
		case 4:
			printf("\tadd ax,bx\n");
			printf("\tpop bx\n");
			printf("\tadc dx,bx\n");
			break;
		}
		return 1;
	case T_MINUS:
		if (n->type == FLOAT)
			return 0;
		/* Do TOS - val */
		printf("\tpop bx\n");
		switch(size) {
		case 1:
			printf("\tsub bl,al\n");
			printf("\tmov al,bl\n");
			break;
		case 2:
			printf("\tsub bx,ax\n");
			printf("\tmov ax,bx\n");
			break;
		case 4:
			printf("\tsub bx,ax\n");
			printf("\tmov ax,bx\n");
			printf("\tpop bx\n");
			printf("\tsbc bx,dx\n");
			printf("\tmov dx,bx\n");
			break;
		return 1;
		}
	case T_STAR:
		if (size == 4)
			return 0;
		printf("\tpop bx\n");
		printf("\tmul ax,bx\n");
		return 1;
	case T_SLASH:
		if (size == 2) {
			printf("\tmov bx,ax\n");
			printf("\txor dx,dx\n");
			printf("\tpop ax\n");
			if (n->type & UNSIGNED)
				printf("\tdiv ax,bx\n");
			else
				printf("\tidiv ax,bx\n");
			/* Result is in AX */
			return 1;
		}
		break;
	case T_PERCENT:
		if (size == 2) {
			printf("\tmov bx,ax\n");
			printf("\txor dx,dx\n");
			printf("\tpop ax\n");
			if (n->type & UNSIGNED)
				printf("\tdiv ax,bx\n");
			else
				printf("\tidiv ax,bx\n");
			/* Result is in DX */
			printf("\tmov ax,dx\n");
			return 1;
		}
		break;
	case T_AND:
		switch(size) {
		case 1:
			printf("\tpop bx\n");
			printf("\tand al,bl\n");
			return 1;
		case 2:
			printf("\tpop bx\n");
			printf("\tand ax,bx\n");
			return 1;
		case 4:
			printf("\tpop bx\n");
			printf("\tand ax,bx\n");
			printf("\tpop bx\n");
			printf("\tand dx,bx\n");
			return 1;
		}
		break;
	case T_OR:
		switch(size) {
		case 1:
			printf("\tpop bx\n");
			printf("\tor al,bl\n");
			return 1;
		case 2:
			printf("\tpop bx\n");
			printf("\tor ax,bx\n");
			return 1;
		case 4:
			printf("\tpop bx\n");
			printf("\tor ax,bx\n");
			printf("\tpop bx\n");
			printf("\tor dx,bx\n");
			return 1;
		}
		break;
	case T_HAT:
		switch(size) {
		case 1:
			printf("\tpop bx\n");
			printf("\txor al,bl\n");
			return 1;
		case 2:
			printf("\tpop bx\n");
			printf("\txor ax,bx\n");
			return 1;
		case 4:
			printf("\tpop bx\n");
			printf("\txor ax,bx\n");
			printf("\tpop bx\n");
			printf("\txor dx,bx\n");
			return 1;
		}
		break;
	case T_TILDE:
		switch(size) {
		case 1:
			printf("\tnot al\n");
			return 1;
		case 2:
			printf("\tnot ax\n");
			return 1;
		case 4:
			printf("\tnot ax\n");
			printf("\tnot dx\n");
			return 1;
		}
		break;
	case T_NEGATE:
		switch(size) {
		case 1:
			printf("\tneg al\n");
			return 1;
		case 2:
			printf("\tneg ax\n");
			return 1;
		}
		break;
	case T_LTLT:
		/* << for TOS by AX */
		printf("\tmov cx,ax\n");
		switch(size) {
		case 1:
			v = get_label();
			printf("\tpop ax\n");
			printf("X%u:\n", v);
			printf("\tshl al\n");
			/* If it becomes 0 it will stay 0 */
			printf("\tloopne X%u\n", v);
			return 1;
		case 2:
			v = get_label();
			printf("\tpop ax\n");
			printf("X%u:\n", v);
			printf("\tshl ax\n");
			/* If it becomes 0 it will stay 0 */
			printf("\tloopne X%u\n", v);
			return 1;
		}
		break;
	case T_GTGT:
		/* >> for TOS by AX */
		printf("\tmov cx,ax\n");
		switch(size) {
		case 1:
			v = get_label();
			printf("\tpop ax\n");
			printf("X%u:\n", v);
			if (n->type & UNSIGNED)
				printf("\tshr al\n");
			else
				printf("\tsar al\n");
			/* If it becomes 0 it will stay 0 */
			printf("\tloopne X%u\n", v);
			return 1;
		case 2:
			v = get_label();
			printf("\tpop ax\n");
			printf("X%u:\n", v);
			if (n->type & UNSIGNED)
				printf("\tshr ax\n");
			else
				printf("\tsar ax\n");
			/* If it becomes 0 it will stay 0 */
			printf("\tloopne X%u\n", v);
			return 1;
		}
		break;
/*	case T_BANG:
	case T_BOOL: */
	/* TODO: conditions */
	/* TODO T_CAST */
	case T_PLUSEQ:
		switch(size) {
		case 1:
			printf("\tpop bx\n");
			printf("\tadd [bx],al\n");
			if (!nr)
				printf("\tmov al,[bx]\n");
			return 1;
		case 2:
			printf("\tpop bx\n");
			printf("\tadd [bx],ax\n");
			if (!nr)
				printf("\tmov ax,[bx]\n");
			return 1;
		case 4:
			printf("\tpop bx\n");
			printf("\tadd [bx],ax\n");
			printf("\tadc 2[bx],dx\n");
			if (!nr) {
				printf("\tmov ax,[bx]\n");
				printf("\tmov dx,2[bx]\n");
			}
			return 1;
		}
		break;
	case T_MINUSEQ:
		switch(size) {
		case 1:
			printf("\tpop bx\n");
			printf("\tsub [bx],al\n");
			if (!nr)
				printf("\tmov al,[bx]\n");
			return 1;
		case 2:
			printf("\tpop bx\n");
			printf("\tsub [bx],ax\n");
			if (!nr)
				printf("\tmov ax,[bx]\n");
			return 1;
		case 4:
			printf("\tpop bx\n");
			printf("\tsub [bx],ax\n");
			printf("\tsbc 2[bx],dx\n");
			if (!nr) {
				printf("\tmov ax,[bx]\n");
				printf("\tmov dx,2[bx]\n");
			}
			return 1;
		}
		break;
	case T_PLUSPLUS:
		switch(size) {
		case 1:
			printf("\tpop bx\n");
			printf("\tmov ah,[bx]\n");
			printf("\tadd [bx],al\n");
			printf("\tmov al,ah\n");
			return 1;
		case 2:
			printf("\tpop bx\n");
			printf("\tmov dx,[bx]\n");
			printf("\tadd [bx],ax\n");
			printf("\tmov ax,dx\n");
			return 1;
		}
		break;
	case T_MINUSMINUS:
		switch(size) {
		case 1:
			printf("\tpop bx\n");
			printf("\tmov ah,[bx]\n");
			printf("\tsub [bx],al\n");
			printf("\tmov al,ah\n");
			return 1;
		case 2:
			printf("\tpop bx\n");
			printf("\tmov dx,[bx]\n");
			printf("\tsub [bx],ax\n");
			printf("\tmov ax,dx\n");
			return 1;
		}
		break;
	case T_ANDEQ:
		switch(size) {
		case 1:
			printf("\tpop bx\n");
			printf("\tand [bx],al\n");
			if (!nr)
				printf("\tmov al,[bx]\n");
			return 1;
		case 2:
			printf("\tpop bx\n");
			printf("\tand [bx],ax\n");
			if (!nr)
				printf("\tmov ax,[bx]\n");
			return 1;
		case 4:
			printf("\tpop bx\n");
			printf("\tand [bx],ax\n");
			printf("\tand 2[bx],dx\n");
			if (!nr) {
				printf("\tmov ax,[bx]\n");
				printf("\tmov dx,2[bx]\n");
			}
			return 1;
		}
	case T_OREQ:
		switch(size) {
		case 1:
			printf("\tpop bx\n");
			printf("\tor [bx],al\n");
			if (!nr)
				printf("\tmov al,[bx]\n");
			return 1;
		case 2:
			printf("\tpop bx\n");
			printf("\tor [bx],ax\n");
			if (!nr)
				printf("\tmov ax,[bx]\n");
			return 1;
		case 4:
			printf("\tpop bx\n");
			printf("\tor [bx],ax\n");
			printf("\tor 2[bx],dx\n");
			if (!nr) {
				printf("\tmov ax,[bx]\n");
				printf("\tmov dx,2[bx]\n");
			}
			return 1;
		}
		break;
	case T_HATEQ:
		switch(size) {
		case 1:
			printf("\tpop bx\n");
			printf("\txor [bx],al\n");
			if (!nr)
				printf("\tmov al,[bx]\n");
			return 1;
		case 2:
			printf("\tpop bx\n");
			printf("\txor [bx],ax\n");
			if (!nr)
				printf("\tmov ax,[bx]\n");
			return 1;
		case 4:
			printf("\tpop bx\n");
			printf("\txor [bx],ax\n");
			printf("\txor 2[bx],dx\n");
			if (!nr) {
				printf("\tmov ax,[bx]\n");
				printf("\tmov dx,2[bx]\n");
			}
			return 1;
		}
		break;
/*	case T_SHLEQ:
	case T_SHREQ: */
	case T_EQEQ:
		return boolop(n, "z", "z");
	case T_BANGEQ:
		return boolop(n, "nz", "nz");
	case T_LTEQ:
		return boolop(n, "be", "le");
	case T_GTEQ:
		return boolop(n, "ae", "ge");
	case T_LT:
		return boolop(n, "b", "lt");
	case T_GT:
		return boolop(n, "a", "gt");
	}
	return 0;
}
