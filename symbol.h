struct symbol
{
    unsigned name;		/* The name of the symbol */
    unsigned type;		/* Type of the symbol */
    unsigned char storage;	/* Storage class */
    unsigned char flags;
#define INITIALIZED	1
    unsigned *idx;		/* Index into object specific data */
    int offset;			/* Offset for locals */
};

#define S_FREE		0	/* Unused */
#define S_AUTO		1	/* Auto */
#define S_REGISTER	2	/* Register */
#define S_ARGUMENT	3	/* Argument */
#define S_LSTATIC	4	/* Static in local scope */
#define S_STATIC	5	/* Static in public scope */
#define S_EXTERN	6	/* External reference */
#define S_EXTDEF	7	/* Exported global */
#define S_STRUCT	8	/* The name of a struct type */
#define S_UNION		9	/* The name of a union type */
#define S_ARRAY		10	/* An array description slot (unnamed) */
#define S_TYPEDEF	11	/* A typedef */
#define S_FUNCDEF	12	/* A function definition */
#define S_ANY		0xFF	/* Used to reserve/lookup entries */

/* For types idx always points to the symbol entry holding the complex type.
   In turn the idx for it points to the desciption blocks.
   Offset for locals gives the base stack offset of the symbol */

extern struct symbol *update_symbol(unsigned name, unsigned storage, unsigned type);
extern struct symbol *find_symbol(unsigned name);
extern struct symbol *alloc_symbol(unsigned name, unsigned local);
extern void pop_local_symbols(struct symbol *top);
extern struct symbol *mark_local_symbols(void);
extern unsigned func_symbol_type(unsigned rtype, unsigned *template);
extern unsigned func_return(unsigned type);
extern unsigned *func_args(unsigned type);
extern struct symbol *symbol_ref(unsigned t);
extern unsigned array_num_dimensions(unsigned type);
extern unsigned array_dimension(unsigned type, unsigned depth);
extern unsigned make_array(unsigned type);
extern void array_add_dimension(unsigned type, unsigned elements);
extern struct symbol *update_struct(unsigned name, unsigned isstruct);
extern unsigned type_of_struct(struct symbol *sym);
extern unsigned *struct_find_member(unsigned name, unsigned fname);

extern void write_bss(void);
