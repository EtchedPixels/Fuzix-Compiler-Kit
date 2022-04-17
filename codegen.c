#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

struct symbol;		/* Dummy */

#include "compiler.h"

#define NUM_NODES 100

static struct node node_table[NUM_NODES];
static struct node *nodes;

/* Hack for now */
#define NAMELEN 16
struct name {
	char name[NAMELEN];
	uint16_t id;
	struct symbol *next;
};

static struct name names[MAXSYM];

static char *namestr(unsigned n)
{
    if (n < 0x8000) {
        fprintf(stderr, "Bad name %x\n", n);
        exit(1);
    } else {
        return names[n - 0x8000].name;
    }
}

struct node *new_node(void)
{
    struct node *n;
    if (nodes == NULL) {
        fprintf(stderr, "Too many nodes.\n");
        exit(1);
    }
    n = nodes;
    nodes = n->right;
    n->left = n->right = NULL;
    n->value = 0;
    n->flags = 0;
    n->sym = NULL;
    return n;
}

void free_node(struct node *n)
{
    n->right = nodes;
    nodes = n;
}

void init_nodes(void)
{
    int i;
    struct node *n = node_table;
    for (i = 0; i < NUM_NODES; i++)
        free_node(n++);
}

static void helper(struct node *n, const char *h)
{
    unsigned t = n->type;
    printf("\tcall %s", h);
    if (PTR(n->type))
        n->type = CINT;
    switch(n->type){
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

static void print_node(struct node *n)
{
    switch(n->op) {
    case T_SHLEQ:
        helper(n, "shleq");
        break;
    case T_SHREQ:
        helper(n, "shreq");
        break;
    case T_PLUSPLUS:
        printf("++ ");
        break;
    case T_MINUSMINUS:
        printf("-- ");
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
    /* This one will need special work */
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
            printf("\txchg\n\tpop h\n\tmov m,e\n\tinx h\n\tmov m,d");
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
        /* Used for cosnt strings */
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
        /* Used for function arg chaining - just ignore */
        return;
    /* Should never be seen */
    case T_DOT:
        printf("**. ");
        break;
    case T_ADDROF:
        printf("**addrof ");
        break;
    case T_LPAREN:
        printf("**( ");
        break;
    case T_RPAREN:
        printf("**) ");
        break;
    case T_LSQUARE:
        printf("**[ ");
        break;
    case T_LCURLY:
        printf("**{ ");
        break;
    case T_RCURLY:
        printf("**} ");
        break;
    case T_POINTSTO:
        printf("**-> ");
        break;
    /* We are using 32bit longs for target so this isnt portable but ok for
       debugging on Linux */
    case T_LONGVAL:
        printf("%dL ", n->value);
        break;
    case T_ULONGVAL:
        printf("%uUL ", n->value);
        break;
    default:
        if (n->op >= T_SYMBOL) {
            if (n->flags & NAMEAUTO)
                printf("Auto ");
            else if (n->flags & NAMEARG)
                printf("Arg ");
            else
                printf("\tlxi ");
            printf("%s+%d", namestr(n->op), n->value);
        } else {
            printf("Invalid %04x ", n);
            exit(1);
        }
    }
#if 0
    switch(n->type & ~7) {
    case CCHAR:
        printf("char");
        break;
    case UCHAR:
        printf("unsigned char");
        break;
    case CINT:
        printf("int");
        break;
    case UINT:
        printf("unsigned int");
        break;
    case CLONG:
        printf("long");
        break;
    case ULONG:
        printf("unsigned long");
        break;
    case FLOAT:
        printf("float");
        break;
    case DOUBLE:
        printf("double");
        break;
    case VOID:
        printf("void");
        break;
    default:
        if (IS_FUNCTION(n->type))
            printf("function");
        else if (IS_ARRAY(n->type))
            printf("array (bug should be canon");
        else
            printf("[%x]", n->type);
    }
    if (n->type & 7) {
        unsigned x = n->type & 7;
        while(x--)
            printf("*");
    }
#endif
    printf("\n");
}    

static void dumptree(struct node *n)
{
    if (n->left) {
        dumptree(n->left);
        printf("\tpush h\n");
    }
    if (n->right) {
        dumptree(n->right);
    }
    print_node(n);
}

/* I/O buffering stuff can wait - as can switching to a block write method */
static struct node *load_tree(void)
{
    struct node *n = new_node();
    if (fread(n, sizeof(struct node), 1, stdin) != 1) {
        fprintf(stderr, "short read\n");
        exit(1);
    }
    /* The values off disk are old pointers or NULL, that's good enough
       to use as a load flag */
    if (n->left)
        n->left = load_tree();
    if (n->right)
        n->right = load_tree();
    return n;
}

static void compile_expression(void)
{
    if (getchar() != '%' || getchar() != '^') {
        fprintf(stderr, "expression expected.\n");
        exit(1);
    }
    dumptree(load_tree());
}

static char *hnames[] = {
    "unused",
    "function",
    "local",
    "localassigned",	/* Obsolete ? */
    "argument",
    "if",
    "else",
    "while",
    "do",
    "dowhile",
    "for",
    "switch",
    "case",
    "default",
    "break",
    "continue",
    "return",
    "label",
    "goto",
    "string",
    "frame"
};

static char *headertype(unsigned t)
{
    static char buf[16];
    if (t <= H_FRAME)
        return hnames[t];
    snprintf(buf, 16, "??%d??", t);
    return buf;
}

static void dumpheader(void)
{
    static unsigned frame_size;
    struct header h;
    char buf[16];
    char *bp = buf;
    if (fread(&h, sizeof(struct header), 1, stdin) != 1) {
        fprintf(stderr, "short read\n");
        exit(1);
    }
    snprintf(buf, 16, "%u", (unsigned)h.h_data);
    if ((h.h_type & 0x7FFF) == H_FUNCTION)
        bp = namestr(h.h_data);
    if ((h.h_type & 0x7FFF) == H_FRAME)
        bp = namestr(h.h_data);
    if (h.h_type & H_FOOTER)
        printf(";Footer %s %d %s\n", headertype(h.h_type & 0x7FFF), h.h_name, bp);
    else
        printf(";Header %s %d %s\n", headertype(h.h_type), h.h_name, bp);

    switch(h.h_type) {
    case H_FUNCTION:
        printf("%s:\n", namestr(h.h_data));
        break;
    case H_FRAME:
        printf("\tlxi h,0x%x\n", h.h_name);
        printf("\tdad sp\n");
        printf("\tsphl\n");
        frame_size = h.h_name;
        break;
    case H_FUNCTION|H_FOOTER:
        printf("L%d_r:\n", h.h_name);
        printf("\tlxi h,0x%x\n",(uint16_t)-frame_size);
        printf("\tdad sp\n");
        printf("\tsphl\n");
        printf("\tret\n");
        break;
    case H_FOR:
        compile_expression();
        printf("L%d_c:\n", h.h_data);
        compile_expression();
        printf("\tcall bool\n");
        printf("\tjz L%d_b\n", h.h_data);
        printf("\tjmp L%d_n\n", h.h_data);
        compile_expression();
        break;
    case H_FOR|H_FOOTER:
        printf("\tL%d_b:\n", h.h_data);
        break;
    }
}

static void percentify(void)
{
    int c;
    c = getchar();
    if (c == '^') {
        dumptree(load_tree());
        return;
    }
    if (c == 'H') {
        dumpheader();
        return;
    }
    if (c != '%') {
        putchar('%');
        putchar(c);
        return;
    }
    /* Just copy the digits for now */
    putchar('S');
}

/* Quick temporary hack */
void load_symbols(void)
{
    int fd = open(".symtmp", O_RDONLY);
    if (fd == -1) {
        perror(".symtmp");
        exit(1);
    }
    read(fd, names, sizeof(names));
    close(fd);
}
    
    
int main(int argc, char *argv[])
{
    int c;
    load_symbols();
    init_nodes();
    while((c = getchar()) != EOF) {
        if (c == '%')
            percentify();
        else
            putchar(c);
    }
}
