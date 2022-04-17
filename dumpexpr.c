#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct symbol;		/* Dummy */

#include "compiler.h"

#define NUM_NODES 100

static struct node node_table[NUM_NODES];
static struct node *nodes;

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
            printf("%d (%d) ", n->op - T_SYMBOL, n->value);
        } else {
            printf("Invalid %04x ", n);
            exit(1);
        }
    }
}    

static void dumptree(struct node *n)
{
    if (n->left)
        dumptree(n->left);
    if (n->right)
        dumptree(n->right);
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
    if (fread(&h, sizeof(struct header), 1, stdin) != 1) {
        fprintf(stderr, "short read\n");
        exit(1);
    }
    if (h.h_type & H_FOOTER)
        printf("Footer %s %d %d\n", headertype(h.h_type & 0x7FFF), h.h_name,
            h.h_data);
    else
        printf("Header %s %d %d\n", headertype(h.h_type), h.h_name,
            h.h_data);
}

static void percentify(void)
{
    int c;
    c = getchar();
    if (c == '^') {
        dumptree(load_tree());
        printf("\n");
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
    
int main(int argc, char *argv[])
{
    int c;
    init_nodes();
    while((c = getchar()) != EOF) {
        if (c == '%')
            percentify();
        else
            putchar(c);
    }
}
