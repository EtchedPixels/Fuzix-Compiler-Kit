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

static void print_node(struct node *n)
{
    switch(n->op) {
    case T_SHLEQ:
        printf(">>= ");
        break;
    case T_SHREQ:
        printf("<<= ");
        break;
    case T_POINTSTO:
        printf("-> ");
        break;
    case T_PLUSPLUS:
        printf("++ ");
        break;
    case T_MINUSMINUS:
        printf("-- ");
        break;
    case T_EQEQ:
        printf("== ");
        break;
    case T_LTLT:
        printf("<< ");
        break;
    case T_GTGT:
        printf(">> ");
        break;
    case T_OROR:
        printf("|| ");
        break;
    case T_ANDAND:
        printf("&& ");
        break;
    case T_PLUSEQ:
        printf("+= ");
        break;
    case T_MINUSEQ:
        printf("-= ");
        break;
    case T_SLASHEQ:
        printf("/= ");
        break;
    case T_STAREQ:
        printf("*= ");
        break;
    case T_HATEQ:
        printf("^= ");
        break;
    case T_BANGEQ:
        printf("!= ");
        break;
    case T_OREQ:
        printf("|= ");
        break;
    case T_ANDEQ:
        printf("&= ");
        break;
    case T_PERCENTEQ:
        printf("%%= ");
        break;

    case T_LPAREN:
        printf("( ");
        break;
    case T_RPAREN:
        printf(") ");
        break;
    case T_LSQUARE:
        printf("[ ");
        break;
    case T_LCURLY:
        printf("{ ");
        break;
    case T_RCURLY:
        printf("} ");
        break;
    case T_AND:
        printf("& ");
        break;
    case T_STAR:
        printf("* ");
        break;
    case T_SLASH:
        printf("/ ");
        break;
    case T_PERCENT:
        printf("%% ");
        break;
    case T_PLUS:
        printf("+ ");
        break;
    case T_MINUS:
        printf("- ");
        break;
    case T_QUESTION:
        printf("? ");
        break;
    case T_COLON:
        printf(": ");
        break;
    case T_HAT:
        printf("^ ");
        break;
    case T_LT:
        printf("< ");
        break;
    case T_GT:
        printf("> ");
        break;
    case T_OR:
        printf("| ");
        break;
    case T_TILDE:
        printf("~ ");
        break;
    case T_BANG:
        printf("! ");
        break;
    case T_EQ:
        printf("= ");
        break;
    case T_DOT:
        printf(". ");
        break;
    case T_COMMA:
        printf(", ");
        break;
    case T_ADDROF:
        printf("addrof ");
        break;
    case T_DEREF:
        printf("deref ");
        break;
    case T_NEGATE:
        printf("neg ");
        break;
    case T_POSTINC:
        printf("postinc ");
        break;
    case T_POSTDEC:
        printf("postdec ");
        break;
    case T_FUNCCALL:
        printf("call ");
        break;
    case T_LABEL:
        /* Not yet properly encoded */
        printf("L%d ", n->value);
        break;
    case T_CAST:
        printf("cast ");
        break;
    case T_INTVAL:
        printf("%d ", n->value);
        break;
    case T_UINTVAL:
        printf("%uU ", n->value);
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
                printf("Name ");
            printf("%s (%d) ", namestr(n->op), n->value);
        } else {
            printf("Invalid %04x ", n);
            exit(1);
        }
    }
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
    printf("\n");
        
        
}    

static void dumptree(struct node *n)
{
    if (n->left) {
        dumptree(n->left);
        printf("push\n");
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

static char *hnames[] = {
    "Unused",
    "Function",
    "Local",
    "LocalAssigned",
    "Argument",
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
    "string"
};

static char *headertype(unsigned t)
{
    static char buf[16];
    if (t <= H_STRING)
        return hnames[t];
    snprintf(buf, 16, "??%d??", t);
    return buf;
}

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
    if (h.h_type & H_FOOTER)
        printf("Footer %s %d %s\n", headertype(h.h_type & 0x7FFF), h.h_name, bp);
    else
        printf("Header %s %d %s\n", headertype(h.h_type), h.h_name, bp);
}

static void percentify(void)
{
    int c;
    c = getchar();
    if (c == '^') {
        printf("Expression\n");
        dumptree(load_tree());
        printf("--\n\n");
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
