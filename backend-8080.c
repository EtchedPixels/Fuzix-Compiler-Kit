#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

static unsigned frame_len;


#define T_LOAD		(T_USER)
#define T_CALLNAME	(T_USER+1)

/*
 *	Our chance to do tree rewriting. We don't do much for the 8080
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	/* Convert LVAL flag into pointer type */
	if (n->flags & LVAL)
		n->type++;
	/* Rewrite references into a load operation */
	if (n->type == CINT || n->type == UINT || PTR(n->type)) {
		if (n->op == T_DEREF
		    && (n->right->op == T_LOCAL
			|| n->right->op == T_ARGUMENT)) {
			n->op = T_LOAD;
			n->snum = n->right->op;
			n->value = n->right->value;
			free_node(n->right);
			n->right = NULL;
		}
	}
	/* Rewrite function call of a name intoo a new node so we can
	   turn it easily into call xyz */
	if (n->op == T_FUNCCALL && n->right->op == T_NAME) {
		n->op = T_CALLNAME;
		n->snum = n->right->snum;
		n->value = n->right->value;
		free_node(n->right);
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
void gen_frame(unsigned size)
{
	frame_len = size;
	printf("\tpush b\n");
	if (size) {
		printf("\tlxi h,0x%x\n", size);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		printf("\tmov b,h\n");
		printf("\tmov c,l\n");
	} else {
		printf("\tlxi b,0\n");
		printf("\tdad sp\n");
	}
}

void gen_epilogue(unsigned size)
{
	if (size) {
		printf("\txchg\n");
		printf("\tlxi h,0x%x\n", (uint16_t) - size);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		printf("\txchg\n");
	}
	printf("\tpop b\n");
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
	printf("\t.ds %dun", value);
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
}

void gen_push(unsigned type)
{
	if (type >= CLONG && !PTR(type))
		printf("\txchg\n\tlhld hireg\n\tpush h\n\tpush d\n");
	else
		printf("\tpush h\n");
}

/*
 *	Generate a helper call according to the types
 */
static void helper(struct node *n, const char *h)
{
	unsigned t = n->type;
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
		putchar('u');
	case CLONG:
		putchar('l');
		break;
	case FLOAT:
		putchar('f');
		break;
	case DOUBLE:
		putchar('d');
		break;
	default:
		fprintf(stderr, "*** bad type %x\n", t);
	}
}

void gen_node(struct node *n)
{
	switch (n->op) {
		/* Load from a name */
	case T_LOAD:
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
		helper(n, "shleq");
		break;
	case T_SHREQ:
		helper(n, "shreq");
		break;
	case T_PLUSPLUS:
		helper(n, "preinc");
		break;
	case T_MINUSMINUS:
		helper(n, "postinc");
		break;
	case T_EQEQ:
		helper(n, "cceq");
		break;
	case T_LTLT:
		helper(n, "shl");
		break;
	case T_GTGT:
		helper(n, "shr");
		break;
	case T_OROR:
		helper(n, "lor");
		break;
	case T_ANDAND:
		helper(n, "land");
		break;
	case T_PLUSEQ:
		helper(n, "pluseq");
		break;
	case T_MINUSEQ:
		helper(n, "minuseq");
		break;
	case T_SLASHEQ:
		helper(n, "diveq");
		break;
	case T_STAREQ:
		helper(n, "muleq");
		break;
	case T_HATEQ:
		helper(n, "xoreq");
		break;
	case T_BANGEQ:
		helper(n, "noteq");
		break;
	case T_OREQ:
		helper(n, "oreq");
		break;
	case T_ANDEQ:
		helper(n, "andeq");
		break;
	case T_PERCENTEQ:
		helper(n, "modeq");
		break;
	case T_AND:
		helper(n, "band");
		break;
	case T_STAR:
		helper(n, "mul");
		break;
	case T_SLASH:
		helper(n, "div");
		break;
	case T_PERCENT:
		helper(n, "mod");
		break;
	case T_PLUS:
		helper(n, "plus");
		break;
	case T_MINUS:
		helper(n, "minus");
		break;
		/* TODO: This one will need special work */
	case T_QUESTION:
		helper(n, "question");
		break;
	case T_COLON:
		helper(n, "colon");
		break;
	case T_HAT:
		helper(n, "xor");
		break;
	case T_LT:
		helper(n, "cclt");
		break;
	case T_GT:
		helper(n, "ccgt");
		break;
	case T_OR:
		helper(n, "or");
		break;
	case T_TILDE:
		helper(n, "neg");
		break;
	case T_BANG:
		helper(n, "not");
		break;
	case T_EQ:
		if (n->type == CINT || n->type == UINT || PTR(n->type))
			printf
			    ("\txchg\n\tpop h\n\tmov m,e\n\tinx h\n\tmov m,d");
		else
			helper(n, "assign");
		break;
	case T_DEREF:
		if (n->type == CINT || n->type == UINT || PTR(n->type))
			printf("\tmov e,m\n\tinx h\n\tmov d,m\n\txchg");
		else
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
	case T_LABEL:
		/* Used for const strings */
		printf("\tlxi h,T%d", n->value);
		break;
	case T_CAST:
		printf("cast ");
		break;
	case T_INTVAL:
		printf("\tlxi h,%d", n->value);
		break;
	case T_UINTVAL:
		printf("\tlxi h,%u", n->value);
		break;
	case T_COMMA:
		printf("comma\n");
		/* Used for function arg chaining - just ignore */
		return;
	case T_BOOL:
		helper(n, "bool");
		break;
	case T_LONGVAL:
	case T_ULONGVAL:
		printf("lxi h,%d", ((n->value >> 16) & 0xFFFF));
		printf("shld hireg");
		printf("lxi h,%d", (n->value & 0xFFFF));
		break;
	case T_NAME:
		printf("\tlxi h,");
		printf("_%s+%d", namestr(n->snum), n->value);
		break;
	case T_LOCAL:
		printf("\tlxi h,%d\n", n->value);
		printf("\tdad b");
		break;
	case T_ARGUMENT:
		printf("\tlxi h,%d\n", n->value + 2 + frame_len);
		printf("\tdad b");
		break;
	default:
		fprintf(stderr, "Invalid %04x ", n->op);
		exit(1);
	}
	printf("\n");
}
