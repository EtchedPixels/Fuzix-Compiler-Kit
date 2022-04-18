#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "compiler.h"

unsigned frame_size;

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

#define T_LOAD		(T_USER)
#define T_CALLNAME	(T_USER+1)

/* Just a test example for now */
/* TODO: rewrite consts to the right, rewrite all >= T_SYMBOL
   to T_SYMBOL and set n->symbol (get rid of symnum ?) */
static struct node *rewrite_node(struct node *n)
{
    if (n->type == CINT || n->type == UINT || PTR(n->type)) {
    /* Rewrite * name into a load */
    if (n->op == T_DEREF && n->right->op >= T_SYMBOL && (n->flags & NAMEAUTO|NAMEARG) == 0) {
        n->op = T_LOAD;
        n->snum = n->right->op;
        n->value = n->right->value;
        free_node(n->right);
        n->right = NULL;
    }
    }
    if (n->op == T_FUNCCALL && n->right->op >= T_SYMBOL) {
        n->op = T_CALLNAME;
        n->snum = n->right->op;
        n->value = n->right->value;
        free_node(n->right);
        n->right = NULL;
    }
    return n;
}

static struct node *rewrite_tree(struct node *n)
{
    struct node *l, *r;
    if (n->left)
        n->left = rewrite_tree(n->left);
    if (n->right)
        n->right = rewrite_tree(n->right);
    return rewrite_node(n);
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
    /* Custom nodes */
    case T_LOAD:
        printf("\tlhld _%s+%d", namestr(n->snum), n->value);
        break;
    case T_CALLNAME:
        printf("\tcall _%s+%d", namestr(n->snum), n->value);
        break;
    /* System nodes */
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
    case T_BOOL:
        helper(n, "bool");
        break;
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
            if (n->flags & NAMEAUTO) {
                printf("\tlxi h,%x\n", n->value);
                printf("\tdad b");
            } else if (n->flags & NAMEARG) {
                printf("\tlxi h,%x\n", n->value + 2 + frame_size);
                printf("\tdad b");
            } else {
                printf("\tlxi h,");
                printf("_%s+%d", namestr(n->op), n->value);
            }
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
        /* This is wrong for other types */
        if (n->left->type >= CLONG)
            printf("\txchg\n\tlhld acchi\n\tpush h\n\tpush d\n");
        else
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
    printf(";exp\n");
    dumptree(rewrite_tree(load_tree()));
    printf(";endexp\n");
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
    "frame",
    "export"
};

static char *headertype(unsigned t)
{
    static char buf[16];
    if (t <= H_EXPORT)
        return hnames[t];
    snprintf(buf, 16, "??%d??", t);
    return buf;
}

static unsigned func_ret;

static void dumpheader(void)
{
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
    case H_EXPORT:
        printf("\t.export _%s\n", namestr(h.h_name));
        break;
    case H_FUNCTION:
        printf("_%s:\n", namestr(h.h_data));
        func_ret = h.h_name;
        break;
    case H_FRAME:
        printf("\tpush b\n");
        printf("\tlxi h,0x%x\n", h.h_name);
        printf("\tdad sp\n");
        printf("\tsphl\n");
        printf("\tmov b,h\n");
        printf("\tmov c,l\n");
        frame_size = h.h_name;
        break;
    case H_FUNCTION|H_FOOTER:
        printf("L%d_r:\n", h.h_name);
        printf("\txchg\n");
        printf("\tlxi h,0x%x\n",(uint16_t)-frame_size);
        printf("\tdad sp\n");
        printf("\tsphl\n");
        printf("\txchg\n");
        printf("\tpop b\n");
        printf("\tret\n");
        break;
    case H_FOR:
        compile_expression();
        printf("L%d_c:\n", h.h_data);
        compile_expression();
        printf("\tjz L%d_b\n", h.h_data);
        printf("\tjmp L%d_n\n", h.h_data);
        compile_expression();
        break;
    case H_FOR|H_FOOTER:
        printf("L%d_b:\n", h.h_data);
        break;
    case H_WHILE:
        printf("L%d_c:\n", h.h_data);
        compile_expression();
        printf("\tjz L%d_b\n", h.h_data);
        break;
    case H_WHILE|H_FOOTER:
        printf("\tjmp L%d_c\n", h.h_data);
        printf("L%d_b:\n", h.h_data);
        break;
    /* double check continue behaviour and write place for L_c */
    case H_DO:
        printf("\tL%d_c:\n", h.h_data);
        break;
    case H_DOWHILE:
        compile_expression();
        printf("\tjnz L%d_c\n", h.h_name);
        break;
    case H_DO|H_FOOTER:
        printf("\tL%d_b:\n", h.h_data);
        break;
    case H_BREAK:
        printf("\tjmp L%d_b\n", h.h_name);
        break;
    case H_CONTINUE:
        printf("\tjmp L%d_c\n", h.h_name);
        break;
    case H_IF:
        compile_expression();
        /* FIXME: these need to deal with bigger types */
        printf("\tjz L%d_e\n", h.h_name);
        break;
    case H_ELSE:
        printf("\tjmp L%d_f\n", h.h_name);
        printf("\tL%dd_e\n", h.h_name);
        break;
    case H_IF|H_FOOTER:
        /* If we have an else then _f is needed, if not _e is */
        if (h.h_data)
            printf("L%d_f\n", h.h_name);
        else
            printf("L%d_e:\n", h.h_name);
        break;
    case H_RETURN:
        printf("\tjmp L%d_r\n", func_ret);
        break;
    case H_LABEL:
        printf("\tL%d:\n", h.h_name);
        break;
    case H_GOTO:
        printf("\tjmp L%d\n", h.h_name);
        break;
    case H_SWITCH:
        /* Will need to stack switch case tables somehow */
        compile_expression();	/* need the type of it back */
        printf("\tlxi d, sw_%d\n", h.h_name);
        printf("\tjmp doswitch\n");	/* needs to be by type */
        break;
    case H_CASE:
        if (h.h_data)
            printf("\tjmp L%d_s\n", h.h_name);
        /* save_case to table - case [h.h_data] */
        /* Need a constant expression resolver */
        compile_expression();	/* FIXME */
        break;
    case H_SWITCH|H_FOOTER:
        printf("L%d_s:\n", h.h_name);
        /* dump_switch_table(h.h_name)) */
        break;
    }
}

static void percentify(void)
{
    int c;
    c = getchar();
    if (c == '^') {
        printf(";exp\n");
        dumptree(rewrite_tree(load_tree()));
        printf(";endexp\n");
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
    printf("\t\t.code\n\n");
    while((c = getchar()) != EOF) {
        if (c == '%')
            percentify();
        else {
            printf("[%c]", c);
        }
    }
}
