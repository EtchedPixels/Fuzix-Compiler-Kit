#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

/* FIXME: wire options in backend to this */
static int cpu = 8085;

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */

#define T_NVAL		(T_USER)
#define T_CALLNAME	(T_USER+1)

/*
 *	Our chance to do tree rewriting. We don't do much for the 8080
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *r = n->right;
	/* Convert LVAL flag into pointer type */
	if (n->flags & LVAL)
		n->type++;
	/* Rewrite references into a load operation */
	if (n->type == CINT || n->type == UINT || PTR(n->type)) {
		if (n->op == T_DEREF) {
#if 0
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				n->op = T_LREF;
				n->snum = r->snum;
				n->value = r->value;
				free_node(r);
				n->right = NULL;
				return n;
			}
#endif
			if (r->op == T_NAME) {
				n->op = T_NVAL;
				n->snum = r->snum;
				n->value = r->value;
				free_node(r);
				n->right = NULL;
				return n;
			}
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
	sp += size;

	printf("; frame %d\n", size);
	if (size > 10) {
		printf("\tlxi h,0x%x\n", size);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		return;
	}
	if (size & 1) {
		printf("\tdcr sp\n");
		size--;
	}
	while(size) {
		printf("\tpush h\n");
		size -= 2;
	}
}

void gen_epilogue(unsigned size)
{
	printf("; eframe %d sp %d\n", size, sp);
	if (sp != size)
		error("sp");
	/* Return in DE so we can adjust the stack more easily. The caller
	   will xchg it back after cleanup */
	sp -= size;
	if (cpu == 8085 && size <= 255 && size > 4) {
		printf("\tldsi %d\n", size);
		printf("\txchg\n");
		printf("\tsphl\n");
		return;
	}
	printf("\txchg\n");
	if (size > 10) {
		printf("\tlxi h,0x%x\n", (uint16_t) - size);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		return;
	}
	if (size % 1) {
		printf("\tinr sp\n");
		size--;
	}
	while (size) {
		printf("\tpop b\n");
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
	printf("T%d:\n", n);
}

void gen_value(unsigned type, unsigned long value)
{
	if (PTR(type)) {
		printf(".word %u\n", (unsigned) value);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		printf(".byte %u\n", (unsigned) value & 0xFF);
		break;
	case CINT:
	case UINT:
		printf(".word %d\n", (unsigned) value & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
		/* We are little endian */
		printf(".word %d\n", (unsigned) (value & 0xFFFF));
		printf(".word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
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

void gen_push(unsigned type)
{
	if (type >= CLONG && !PTR(type)) {
		sp += 4;
		printf("\txchg\n\tlhld hireg\n\tpush h\n\tpush d\n");
	} else {
		sp += 2;
		printf("\tpush h\n");
	}
}

/*
 *	Generate a helper call according to the types
 */
static void do_helper(struct node *n, const char *h, unsigned mod)
{
	unsigned t = n->type;
	unsigned spmod = 2;
	printf("\tcall %s", h);
	if (PTR(n->type))
		n->type = CINT;
	switch (n->type) {
	case UCHAR:
		putchar('u');
	case CCHAR:
		putchar('c');
		break;
	case UINT:
		putchar('u');
	case CINT:
		break;
	case ULONG:
		spmod = 4;
		putchar('u');
	case CLONG:
		spmod = 4;
		putchar('l');
		break;
	case FLOAT:
		spmod = 4;
		putchar('f');
		break;
	case DOUBLE:
		spmod = 8;
		putchar('d');
		break;
	default:
		fprintf(stderr, "*** bad type %x\n", t);
	}
	if (mod)
		sp -= spmod;
}

static void helper(struct node *n, const char *h)
{
	do_helper(n, h, 0);
}

static void helper_sp(struct node *n, const char *h)
{
	do_helper(n, h, 1);
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
	if (n->op != T_CONSTANT && n->op != T_NAME && n->op != T_LABEL && n->op != T_NVAL)
		return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CINT)
		return 0;
	return 1;
}

/*
 *	Get something that passed the access_direct check into de. Could
 *	we merge this with the similar hl one in the main table ?
 */

static void load_de_with(struct node *n)
{
	switch(n->op) {
	case T_NAME:
		printf("\tlxi d, _%s+%d\n", namestr(n->snum), n->value);
		break;
	case T_LABEL:
		printf("\tlxi d, T%d\n", n->value);
		break;
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		printf("\tlxi d, %d\n", n->value);
		break;
	case T_NVAL:
		/* We know it is int or pointer */
		printf("\txchg\n");
		printf("\tlhld (_%s+%d)\n", namestr(n->snum), n->value);
		printf("\txchg\n");
		break;
	default:
		error("ldew");
	}
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
	/* We only deal with simple cases for now */
	if (n->right && !access_direct(n->right))
		return 0;

	switch (n->op) {
	case T_CLEANUP:
		/* CLEANUP is special and needs to be handled directly */
		/* TODO - optimizations like inx sp and pops */
		if (n->right->value) {
			printf("\tlxi h, %d\n", n->right->value);
			printf("\tdad sp\n");
			printf("\tsphl\n");
			sp -= n->right->value;
		}
		/* The call return is in DE at this point due to stack juggles
		   so put it into HL */
		printf("\txchg\n");
		return 1;
	case T_PLUS:
		/* LHS is in HL at the moment, end up with the result in HL */
		load_de_with(n->right);
		printf("\tdad d\n");
		return 1;
	}
	return 0;
}

void gen_node(struct node *n)
{
	switch (n->op) {
		/* Load from a name */
	case T_NVAL:
		printf("\tlhld _%s+%d", namestr(n->snum), n->value);
		break;
		/* Call a function by name */
	case T_CALLNAME:
		printf("\tcall _%s+%d", namestr(n->snum), n->value);
		break;
	case T_NULL:
		/* Dummy 'no expression' node */
		break;
	case T_SHLEQ:
		helper_sp(n, "shleq");
		break;
	case T_SHREQ:
		helper_sp(n, "shreq");
		break;
	case T_PLUSPLUS:
		helper_sp(n, "preinc");
		break;
	case T_MINUSMINUS:
		helper_sp(n, "postinc");
		break;
	case T_EQEQ:
		helper_sp(n, "cceq");
		break;
	case T_LTLT:
		helper_sp(n, "shl");
		break;
	case T_GTGT:
		helper_sp(n, "shr");
		break;
	case T_OROR:
		helper_sp(n, "lor");
		break;
	case T_ANDAND:
		helper_sp(n, "land");
		break;
	case T_PLUSEQ:
		helper_sp(n, "pluseq");
		break;
	case T_MINUSEQ:
		helper_sp(n, "minuseq");
		break;
	case T_SLASHEQ:
		helper_sp(n, "diveq");
		break;
	case T_STAREQ:
		helper_sp(n, "muleq");
		break;
	case T_HATEQ:
		helper_sp(n, "xoreq");
		break;
	case T_BANGEQ:
		helper_sp(n, "noteq");
		break;
	case T_OREQ:
		helper_sp(n, "oreq");
		break;
	case T_ANDEQ:
		helper_sp(n, "andeq");
		break;
	case T_PERCENTEQ:
		helper_sp(n, "modeq");
		break;
	case T_AND:
		helper_sp(n, "band");
		break;
	case T_STAR:
		helper_sp(n, "mul");
		break;
	case T_SLASH:
		helper_sp(n, "div");
		break;
	case T_PERCENT:
		helper_sp(n, "mod");
		break;
	case T_PLUS:
		helper_sp(n, "plus");
		break;
	case T_MINUS:
		helper_sp(n, "minus");
		break;
		/* TODO: This one will need special work */
	case T_QUESTION:
		helper_sp(n, "question");
		break;
	case T_COLON:
		helper_sp(n, "colon");
		break;
	case T_HAT:
		helper_sp(n, "xor");
		break;
	case T_LT:
		helper_sp(n, "cclt");
		break;
	case T_GT:
		helper_sp(n, "ccgt");
		break;
	case T_OR:
		helper_sp(n, "or");
		break;
	case T_TILDE:
		helper(n, "neg");
		break;
	case T_BANG:
		helper(n, "not");
		break;
	case T_EQ:
		if (n->type == CINT || n->type == UINT || PTR(n->type)) {
			if (cpu == 8085)
				printf("\txchg\n\tpop h\n\tshlx");
			else
				printf("\txchg\n\tpop h\n\tmov m,e\n\tinx h\n\tmov m,d");
			sp -= 2;
		} else
			helper_sp(n, "assign");
		break;
	case T_DEREF:
		if (n->type == CINT || n->type == UINT || PTR(n->type)) {
			if (cpu == 8085)
				printf("\tlhlx");
			else
				printf("\tmov e,m\n\tinx h\n\tmov d,m\n\txchg");
		} else
			helper(n, "deref");
		break;
	case T_NEGATE:
		helper(n, "negate");
		break;
	case T_POSTINC:
		helper(n, "postinc");
		break;
	case T_POSTDEC:
		helper(n, "postdec");
		break;
	case T_FUNCCALL:
		printf("\tcall callhl");
		break;
	case T_CLEANUP:
		/* Should never occur except direct */
		error("tclu");
		break;
	case T_LABEL:
		/* Used for const strings */
		printf("\tlxi h,T%d", n->value);
		break;
	case T_CAST:
		printf("cast ");
		break;
	case T_CONSTANT:
		switch(n->type) {
		case CLONG:
		case ULONG:
			printf("lxi h,%u", ((n->value >> 16) & 0xFFFF));
			printf("shld hireg");
		/* For readability */
		case UCHAR:
		case UINT:
			printf("\tlxi h,%u", (n->value & 0xFFFF));
			break;
		case CCHAR:
		case CINT:
			printf("\tlxi h,%d", (n->value & 0xFFFF));
			break;
		}
		break;
	case T_COMMA:
		/* Used for function arg chaining - just ignore */
		return;
	case T_BOOL:
		helper(n, "bool");
		break;
	case T_NAME:
		printf("\tlxi h,");
		printf("_%s+%d", namestr(n->snum), n->value);
		break;
	case T_LOCAL:
		/* FIXME: add long, char etc to this and argument */
		if (cpu == 8085 && n->value + sp <= 255) {
			printf("\tldsi %d", n->value + sp);
		} else {
			printf("\tlxi h,%d\n", n->value + sp);
			printf("\tdad sp");
		}
		break;
	case T_ARGUMENT:
		if (cpu == 8085 && n->value + 2 + frame_len + sp <= 255) {
			printf("ldsi %d\n", n->value + sp);
		} else {
			printf("\tlxi h,%d\n", n->value + 2 + frame_len + sp);
			printf("\tdad sp");
		}
		break;
	default:
		fprintf(stderr, "Invalid %04x ", n->op);
		exit(1);
	}
	printf("\n");
}
