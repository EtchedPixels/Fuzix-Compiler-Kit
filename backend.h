
extern void error(const char *p);

extern char *namestr(unsigned n);
extern struct node *new_node(void);
extern void free_node(struct node *n);
extern void init_nodes(void);

extern void helper(struct node *n, const char *h);
extern void helper_type(unsigned t);
extern void codegen_lr(struct node *n);

extern struct node *gen_rewrite_node(struct node *n);


extern void gen_export(const char *name);
extern void gen_prologue(const char *name);
extern void gen_frame(unsigned size);
extern void gen_epilogue(unsigned size);
extern void gen_label(const char *t, unsigned n);
extern void gen_jump(const char *t, unsigned n);
extern void gen_jfalse(const char *t, unsigned n);
extern void gen_jtrue(const char *t, unsigned n);

extern void gen_switch(unsigned n, unsigned type);
extern void gen_switchdata(unsigned n, unsigned size);
extern void gen_case(unsigned tag, unsigned entry);
extern void gen_case_label(unsigned tag, unsigned entry);

extern void gen_data(const char *name);
extern void gen_bss(const char *name);
extern void gen_code(void);

extern void gen_space(unsigned value);
extern void gen_text_label(unsigned value);
extern void gen_value(unsigned type, unsigned long value);
extern void gen_name(struct node *n);
extern void gen_literal(unsigned value);

extern void gen_helpcall(void);

extern void gen_start(void);
extern void gen_end(void);

extern void gen_tree(struct node *n);

/* Provide if using codgen_lr */
extern unsigned gen_push(struct node *n);
extern unsigned gen_node(struct node *n);
extern unsigned gen_direct(struct node *n);
extern unsigned gen_uni_direct(struct node *n);
extern unsigned gen_shortcut(struct node *n);
