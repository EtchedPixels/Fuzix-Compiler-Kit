struct node
{
    struct node *left;
    struct node *right;
    unsigned op;
    unsigned flags;
#define LVAL			1
#define NOEFF			2
    unsigned value;
    void *ptr;			/* symbol ? */
};

extern void init_nodes(void);

extern struct node *tree(unsigned op, struct node *l, struct node *r);
extern void free_node(struct node *n);
extern struct node *new_node(void);

extern struct node *make_rval(struct node *);
extern struct node *make_constant(unsigned n);
extern struct node *make_symbol(struct symbol *s);
extern struct node *make_label(unsigned n);

extern void write_tree(struct node *n);
extern void write_null_tree(void);
