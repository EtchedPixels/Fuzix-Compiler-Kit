extern void bracketed_expression(unsigned mkbool);
extern void expression_or_null(unsigned mkbool, unsigned noret);
extern unsigned const_expression(void);
extern void expression(unsigned comma, unsigned mkbool, unsigned noret);
extern struct node *expression_tree(unsigned comma);
extern struct node *hier1(void);	/* needed for primary bracketed */
