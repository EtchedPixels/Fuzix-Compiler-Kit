/*
 *	Tree operations to build a node tree
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "defs.h"

#define NUM_NODES 100

static struct node node_table[NUM_NODES];
static struct node *nodes;

struct node *new_node(void)
{
    struct node *n;
    if (nodes == NULL) {
        error("too complex");
        exit(1);
    }
    n = nodes;
    nodes = n->right;
    n->left = n->right = NULL;
    n->value = 0;
    n->flags = 0;
    n->ptr = NULL;
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


struct node *tree(unsigned op, struct node *l, struct node *r)
{
    struct node *n = new_node();
    fprintf(stderr, "tree %04x [", op);
    if (l)
     fprintf(stderr, "%04x ", l->op);
    if (r)
     fprintf(stderr, "%04x ", r->op);
    fprintf(stderr, "]\n");
    n->left = l;
    n->right = r;
    n->op = op;
    return n;
}

/* Will need type info.. */
struct node *make_constant(unsigned value)
{
    struct node *n = new_node();
    n->op = T_UINTVAL;
    n->value = value;
    fprintf(stderr, "const %04x\n", value);
    return n;
}

struct node *make_symbol(struct symbol *s)
{
    struct node *n = new_node();
    n->op = T_NAME;
    n->value = 0;
    n->ptr = s;
    n->flags = LVAL;
    fprintf(stderr, "name %04x\n", s->name - 0x8000);
    return n;
}

struct node *make_label(unsigned label)
{
    struct node *n = new_node();
    n->op = T_LABEL;
    n->value = label;
    n->flags = 0;
    fprintf(stderr, "label %04x\n", label);
    return n;
}

unsigned is_constant(struct node *n)
{
    return (n->op >= T_INTVAL && n->op <= T_ULONGVAL) ? 1 : 0;
}

static void nameref(struct node *n)
{
    if (is_constant(n->right) && n->left->op == T_NAME) {
        unsigned value = n->left->value + n->right->value;
        struct node *l = n->left;
        free_node(n->right);
        *n = *n->right;
        n->value = value;
        n->left = NULL;
        n->right = NULL;
        free_node(l);
    }
}
    
void canonicalize(struct node *n)
{
    if (n->left)
        canonicalize(n->left);
    if (n->right)
        canonicalize(n->right);
    /* Get constants onto the right hand side */
    if (n->left && is_constant(n->left)) {
        struct node *t = n->left;
        n->left = n->right;
        n->right = t;
    }
    /* Propogate NOEFF side effect checking. If our children have a side
       effect then we don't even if we were safe ourselves */
    if (!(n->left->flags & NOEFF) || !(n->right->flags & NOEFF))
        n->flags &=- ~NOEFF;

    /* Turn name + constant into name offsets */
    if (n->op == T_PLUS)
        nameref(n);
        
    /* Target rewrites - eg turning DEREF of local + const into a name with
       offset when supported or switching around subtrace trees that are NOEFF */
    //target_canonicalize(n);
}

struct node *make_rval(struct node *n)
{
    if (n->flags & LVAL)
        return tree(T_DEREF, NULL, n);
    return n;
}

static void write_subtree(struct node *n)
{
    write(1, n, sizeof(struct node));
    if (n->left)
        write_subtree(n->left);
    if (n->right)
        write_subtree(n->right);
    free_node(n);
}

void write_tree(struct node *n)
{
    write(1,"%^", 2);
    write_subtree(n);
}

void write_null_tree(void)
{
    write_tree(tree(T_NULL, NULL, NULL));
}
