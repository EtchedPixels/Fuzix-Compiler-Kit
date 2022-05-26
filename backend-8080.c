#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

#define ARGBASE	2	/* Bytes between arguments and locals */

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */

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

#define T_NREF		(T_USER)
#define T_CALLNAME	(T_USER+1)
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
	if (op == T_NREF)
		return 1;
	return 0;
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

	/* Rewrite references into a load operation */
	if (nt == CSHORT || nt == USHORT || PTR(nt)) {
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
		}
		if (op == T_EQ) {
			if (l->op == T_NAME) {
				squash_left(n, T_NSTORE);
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
	printf("_%s:\n", name);
}

/* Generate the stack frame */
/* TODO: defer this to statements so we can ld/push initializers */
void gen_frame(unsigned size)
{
	frame_len = size;
	sp = 0;
	if (size > 10) {
		printf("\tlxi h,%d\n", -size);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		return;
	}
	if (size & 1) {
		printf("\tdcx sp\n");
		size--;
	}
	while(size) {
		printf("\tpush h\n");
		size -= 2;
	}
}

void gen_epilogue(unsigned size)
{
	if (sp != 0)
		error("sp");
	/* Return in HL, does need care on stack. TOOD: flag void functions
	   where we can burn the return */
	sp -= size;
	if (cpu == 8085 && size <= 255 && size > 4) {
		printf("\tldsi %d\n", size);
		printf("\txchg\n");
		printf("\tsphl\n");
		printf("\txchg\n");
		printf("\tret\n");
		return;
	}
	if (size > 10) {
		printf("\txchg\n");
		printf("\tlxi h,0x%x\n", (uint16_t) - size);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		printf("\txchg\n");
		printf("\tret\n");
		return;
	}
	if (size & 1) {
		printf("\tinx sp\n");
		size--;
	}
	while (size) {
		printf("\tpop d\n");
		size -= 2;
	}
	printf("\tret\n");
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

static void gen_cleanup(unsigned v)
{
	/* CLEANUP is special and needs to be handled directly */
	sp -= v;
	if (v > 10) {
		/* This is more expensive, but we don't often pass that many
		   arguments so it seems a win to stay in HL */
		/* TODO: spot void function and skip xchg */
		printf("\txchg\n");
		printf("\tlxi h,%d\n", -v);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		printf("\txchg\n");
	} else {
		while(v >= 2) {
			printf("\tpop d\n");
			v -= 2;
		}
		if (v)
			printf("\tdcr sp\n");
	}
}

/*
 *	Helper handlers. We use a tight format for integers but C
 *	style for float as we'll have C coded float support if any
 */
void gen_helpcall(struct node *n)
{
	if (n->type == FLOAT)
		gen_push(n->right);
	printf("\tcall __");
}

void gen_helpclean(struct node *n)
{
	unsigned s;

	if (n->type != FLOAT)
		return;

	s = 0;
	if (n->left) {
		s += get_size(n->left->type);
		/* gen_node already accounted for removing this thinking
		   the helper did the work, adjust it back as we didn't */
		sp += s;
	}
	s += get_size(n->right->type);
	gen_cleanup(s);
}

void gen_switch(unsigned n, unsigned type)
{
	printf("\tlxi d,Sw%d\n", n);
	printf("\tjmp __switch");
	helper_type(type & ~UNSIGNED);
	printf("\n");
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
}

void gen_case(unsigned tag, unsigned entry)
{
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
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

/* The label for a literal (currently only strings)
   TODO: if we add other literals we may need alignment here */

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
		printf("\t.word %d\n", w);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		printf("\t.word %d\n", w);
		printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	printf("\t.setcpu %d\n", cpu);
}

void gen_end(void)
{
}

void gen_tree(struct node *n)
{
	codegen_lr(n);
	printf(";\n");
/*	printf(";SP=%d\n", sp); */
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
	/* We can direct access integer or smaller types that are constants
	   global/static or string labels */
	if (n->op != T_CONSTANT && n->op != T_NAME && n->op != T_LABEL && n->op != T_NREF)
		return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;
	return 1;
}

/*
 *	Get something that passed the access_direct check into de. Could
 *	we merge this with the similar hl one in the main table ?
 */

static unsigned load_r_with(const char r, struct node *n)
{
	unsigned v = WORD(n->value);
	const char *name;

	switch(n->op) {
	case T_NAME:
		printf("\tlxi %c,_%s+%d\n", r, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		printf("\tlxi %c,T%d\n", r, v);
		return 1;
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		printf("\tlxi %c,%d\n", r, v);
		return 1;
	case T_NREF:
		name = namestr(n->snum);
		if (r == 'b')
			return 0;
		else if (r == 'h') {
			printf("\tlhld _%s+%d\n", name, v);
			return 1;
		} else if (r == 'd') {
			/* We know it is int or pointer */
			printf("\txchg\n");
			printf("\tlhld _%s+%d\n", name, v);
			printf("\txchg\n");
			return 1;
		}
		break;
	default:
		return 0;
	}
	return 1;
}

static unsigned load_bc_with(struct node *n)
{
	return load_r_with('b', n);
}

static unsigned load_de_with(struct node *n)
{
	return load_r_with('d', n);
}

static unsigned load_hl_with(struct node *n)
{
	return load_r_with('h', n);
}

static unsigned load_a_with(struct node *n)
{
	switch(n->op) {
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		printf("\tmvi a,%d\n", BYTE(n->value));
		break;
	case T_NREF:
		printf("\tlda _%s+%d\n", namestr(n->snum), WORD(n->value));
		break;
	default:
		return 0;
	}
	return 1;
}

static void repeated_op(const char *o, unsigned n)
{
	while(n--)
		printf("\t%s\n", o);
}


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
	if (r->op == T_CONSTANT && r->value == 0) {
		char buf[10];
		strcpy(buf, op);
		strcat(buf, "0");
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

static const char dad_h[8] = "\tdad h\n";

static unsigned gen_fast_mul(unsigned s, unsigned n)
{
	if (s != 2)
		return 0;

	switch(n) {
		case 0:
			printf("\tlxi h,0\n");
			return 1;
		case 1:	/* Should have been removed already */
			return 1;
		case 256:
			printf("\tmov h,l\n");
			printf("\tmvi l,0\n");
			return 1;
		case 32:
			printf(dad_h);
		case 16:
			printf(dad_h);
		case 8:
			printf(dad_h);
		case 4:
			printf(dad_h);
		case 2:
			printf(dad_h);
			return 1;
		case 12:
			printf(dad_h);
		case 6:
			printf(dad_h);
		case 3:
			printf("\tpush h\n\tdad h\n\tpop d\n\tdad d\n");
			return 1;
		case 10:
			printf(dad_h);
		case 5:
			printf("\tpush h\n\tdad h\n\tdad h\n\tpop d\n\tdad d\n");
			return 1;
	}
	return 0;
}

static unsigned gen_fast_div(unsigned n, unsigned s)
{
	if (s != 2)
		return 0;
	if (n == 1)
		return 1;
	if (n == 256) {
		printf("\tmov l,h\n\tmvi h,0\n");
		return 1;
	}
	if (cpu != 8085)
		return 0;

	if (n & (n - 1))
		return 0;

	while(n > 1) {
		printf("\tarhl\n");
		n >>= 1;
	}
	return 1;
}


/* TODO : we could in theory optimize xor 255 with cpl ? */
static unsigned gen_logicc(struct node *n, unsigned s, const char *op, unsigned v, unsigned code)
{
	unsigned h = (v >> 8) & 0xFF;
	unsigned l = v & 0xFF;

	if (s > 2 || (n && n->op != T_CONSTANT))
		return 0;

	/* If we are trying to be compact only inline the short ones */
	if (optsize && ((h != 0 && h != 255) || (l != 0 && l != 255)))
		return 0;

	if (s == 2) {
		if (h == 0) {
			if (code == 1)
				printf("\tmvi h,0\n");
		}
		else if (h == 255 && code != 3) {
			if (code == 2)
				printf("\tmvi h,255\n");
		} else {
			printf("\tmov a,h\n\t%s %d\n\tmov h,a\n", op, h);
		}
	}
	if (l == 0) {
		if (code == 1)
			printf("\tmvi l,0\n");
	} else if (l == 255 && code != 3) {
		if (code == 2)
			printf("\tmvi l,255\n");
	} else {
		printf("\tmov a,l\n\t%s %d\n\tmov l,a\n", op, l);
	}
	return 1;
}

static unsigned gen_fast_remainder(unsigned n, unsigned s)
{
	unsigned mask;
	if (s != 2)
		return 0;
	if (n == 1) {
		printf("\tlxi h,0\n");
		return 1;
	}
	if (n == 256) {
		printf("\tmvi h,0\n");
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
		if (s == 1) {
			printf("\tmov a,l\n");
			printf("\tsta _%s+%d\n", namestr(n->snum), WORD(n->value));
			return 1;
		}
		if (s == 2) {
			printf("\tshld _%s+%d\n", namestr(n->snum), WORD(n->value));
			return 1;
		}
		/* TODO 4/8 for long etc */
		return 0;
	case T_EQ:
		/* The address is in HL at this point */
		if (cpu == 8085 && s == 2 ) {
			printf("\txchg\n");
			if (load_hl_with(r) == 0)
				return 0;
			printf("\tshlx\n");
			return 1;
		}
		if (s == 1) {
			/* We need to end up with the value in l if this is not NORETURN, also
			   we can optimize constant a step more */
			if (r->op == T_CONSTANT && (n->flags & NORETURN))
				printf("\tmvi m,%d\n", ((unsigned)r->value) & 0xFF);
			else {
				if (load_a_with(r) == 0)
					return 0;
				printf("\tmov m,a\n");
				if (!(n->flags & NORETURN))
					printf("\tmov l,a\n");
			}
			return 1;
		}
		return 0;
	case T_PLUS:
		/* Zero should be eliminated in cc1 FIXME */
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (v < 4 && s <= 2) {
				if (s == 1)
					repeated_op("inr l", v);
				else
					repeated_op("inx h", v);
				return 1;
			}
		}
		if (s <= 2) {
			/* LHS is in HL at the moment, end up with the result in HL */
			if (s == 1) {
				if (load_a_with(r) == 0)
					return 0;
				printf("\tmov e,a\n");
			}
			if (s > 2 || load_de_with(r) == 0)
				return 0;
			printf("\tdad d\n");
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
				return 1;
			}
		}
		/* TODO - do we care about saving BC ? */
		if (cpu == 8085 && s <= 2) {
			/* LHS is in HL at the moment, end up with the result in HL */
			if (s == 1) {
				if (load_a_with(r) == 0)
					return 0;
				printf("\tmov c,a\n");
			}
			if (s > 2 || load_bc_with(r) == 0)
				return 0;
			printf("\tdsub  ; b\n");
			return 1;
		}
		return 0;
	case T_STAR:
		if (r->op == T_CONSTANT) {
			if (s <= 2 && gen_fast_mul(s, r->value))
				return 1;
		}
		return gen_deop("mulde", n, r, 0);
	case T_SLASH:
		if (r->op == T_CONSTANT && (n->type & UNSIGNED)) {
			if (s <= 2 && gen_fast_div(s, r->value))
				return 1;
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
		return gen_deop("xorde", n, r, 0);
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
		if (s <= 2 && r->op == T_CONSTANT && r->value <= 8) {
			if (r->value < 8)
				repeated_op("dad h", r->value);
			else
				printf("\tmov h,l\n\tmvi l,0\n");
			return 1;
		}
		return gen_deop("shlde", n, r, 0);
	case T_GTGT:
		/* >> by 8 unsigned */
		if (s == 2 && (n->type & UNSIGNED) && r->op == T_CONSTANT && r->value == 8) {
			printf("\tmov l,h\n\tmvi h,0\n");
			return 1;
		}
		/* 8085 has a signed right shift 16bit */
		if (cpu == 8085 && (!(n->type & UNSIGNED)) && s == 2) {
			if (s <= 2 && r->op == T_CONSTANT && r->value < 8) {
				repeated_op("arhl", r->value);
				return 1;
			}
		}
		return gen_deop("shrde", n, r, 1);
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
	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	if (n->op == T_COMMA) {
		n->left->flags |= NORETURN;
		codegen_lr(n->left);
		codegen_lr(n->right);
		return 1;
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
		printf("\tpush h\n");
		return 1;
	case 4:
		if (optsize)
			printf("\tcall __pushl\n");
		else
			printf("\txchg\n\tlhld __hireg\n\tpush h\n\tpush d\n");
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
		rt = CSHORT;
	if (PTR(lt))
		lt = CSHORT;

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
	printf("\tmvi h,0\n");
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned v;
	char *name;
	/* We adjust sp so track the pre-adjustment one too when we need it */
	unsigned spval = sp;

	v = n->value;

	/* An operation with a left hand node will have the left stacked
	   and the operation will consume it so adjust the stack.

	   The exception to this is comma and the function call nodes
	   as we leave the arguments pushed for the function call */

	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_stack_size(n->left->type);

	switch (n->op) {
		/* Load from a name */
	case T_NREF: /* NREF/STORE FIXME for long/float/double */
		if (size == 1) {
			printf("\tlda _%s+%d\n", namestr(n->snum), v);
			printf("\tmov l,a\n");
		} else if (size == 2) {
			printf("\tlhld _%s+%d\n", namestr(n->snum), v);
			return 1;
		}
		break;
	case T_LREF:
		/* We are loading something then not using it, and it's local
		   so can go away */
		if (n->flags & NORETURN)
			return 1;
/*		printf(";L sp %d spval %d %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		if (v + sp == 0 && size == 2) {
			printf("\tpop h\n\tpush h\n");
			return 1;
		}
		v += sp;
		if (cpu == 8085 && v <= 255) {
			printf("\tldsi %d\n", v);
			if (size == 2)
				printf("\tlhlx\n");
			else
				printf("\tldax d\n\tmov d,a\n");
			return 1;
		}
		/* Via helper magic for compactness on 8080 */
		if (size == 1)
			name = "ldbyte";
		else
			name = "ldword";
		if (v < 24)
			printf("\tcall %s%d\n", name, v);
		else if (v < 255)
			printf("\tcall %s\n\t.byte %d\n", name, v);
		else
			printf("\tcall %sw\n\t.word %d\n", name, v);
		return 1;
	case T_NSTORE:
		if (size > 2)
			return 0;
		if (size == 1)
			printf("\tmov a,l\n\tsta");
		else
			printf("\tshld");
		printf(" _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_LSTORE:
/*		printf(";L sp %d spval %d %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		if (v + sp == 0 && size == 2 ) {
			printf("\tpop psw\n\tpush h\n");
			return 1;
		}
		v += sp;
		if (cpu == 8085 && v + sp <= 255) {
			printf("\tldsi %d\n", v);
			if (size == 2)
				printf("\tshlx\n");
			else
				printf("\tmov a,l\n\tstax d\n");
			return 1;
		}
		/* Via helper magic for compactness on 8080 */
		/* Can rewrite some of them into rst if need be */
		if (size == 1)
			name = "stbyte";
		else
			name = "stword";
		if (v < 24)
			printf("\tcall %s%d\n", name, v);
		else if (v < 255)
			printf("\tcall %s\n\t.byte %d\n", name, v);
		else
			printf("\tcall %sw\n\t.word %d\n", name, v);
		return 1;
		/* Call a function by name */
	case T_CALLNAME:
		printf("\tcall _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_EQ:
		if (size == 2) {
			if (cpu == 8085)
				printf("\tpop d\n\tshlx\n");
			else {
				printf("\txchg\n\tpop h\n\tmov m,e\n\tinx h\n\tmov m,d\n");
				if (!(n->flags & NORETURN))
					printf("\txchg\n");
			}
			return 1;
		}
		if (size == 1) {
			printf("\tpop d\n\txchg\n\tmov m,e\n");
			if (!(n->flags & NORETURN))
				printf("\txchg\n");
			return 1;
		}
		break;
	case T_DEREF:
		if (size == 2) {
			if (cpu == 8085)
				printf("\txchg\n\tlhlx\n");
			else
				printf("\tmov e,m\n\tinx h\n\tmov d,m\n\txchg\n");
			return 1;
		}
		if (size == 1) {
			printf("\tmov l,m\n");
			return 1;
		}
		if (size == 4 && cpu == 8085 && !optsize) {
			printf("\txchg\n\tinx d\n\tinx d\n\tlhlx\n\tshld __hireg\t\n\tdcx d\n\tdcx d\n\tlhlx\n");
			return 1;
		}
		break;
	case T_FUNCCALL:
		printf("\tcall __callhl\n");
		return 1;
	case T_LABEL:
		/* Used for const strings */
		printf("\tlxi h,T%d\n", v);
		return 1;
	case T_CONSTANT:
		switch(size) {
		case 4:
			printf("\tlxi h,%u\n", ((v >> 16) & 0xFFFF));
			printf("\tshld __hireg\n");
		case 2:
			printf("\tlxi h,%d\n", (v & 0xFFFF));
			return 1;
		case 1:
			printf("\tmvi l,%d\n", (v & 0xFF));
			return 1;
		}
		break;
	case T_NAME:
		printf("\tlxi h,");
		printf("_%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_LOCAL:
		v += sp;
/*		printf(";LO sp %d spval %d %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		if (cpu == 8085 && v <= 255) {
			printf("\tldsi %d\n", v);
			printf("\txchg\n");
		} else {
			printf("\tlxi h,%d\n", v);
			printf("\tdad sp\n");
		}
		return 1;
	case T_ARGUMENT:
		v += frame_len + ARGBASE + sp;
/*		printf(";AR sp %d spval %d %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		if (cpu == 8085 && v <= 255) {
			printf("\tldsi %d\n", v);
			printf("\txchg\n");
		} else {
			printf("\tlxi h,%d\n", v);
			printf("\tdad sp\n");
		}
		return 1;
	case T_CAST:
		return gen_cast(n);
	case T_PLUS:
		if (size <= 2) {
			printf("\tpop d\n\tdad d\n");
			return 1;
		}
		break;
	}
	return 0;
}
