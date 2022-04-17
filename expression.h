extern void bracketed_expression(void);
extern void expression_or_null(void);
extern unsigned const_expression(void);
extern void expression(unsigned comma);
extern struct node *expression_tree(unsigned comma);
extern struct node *hier1(void);	/* needed for primary bracketed */
