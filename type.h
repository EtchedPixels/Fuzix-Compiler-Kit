/*
 *	Types. Reorganized so we can do some vaguely proper type management
 *
 */
#define PTR(x)		((x) & 7)		/* 7 deep should be loads */
#define CCHAR		0x00			/* 00-7F - integer */
#define CINT		0x10
#define	CLONG		0x20
#define CLONGLONG	0x30
#define UNSIGNED	0x40
#define UCHAR		0x40
#define UINT		0x50
#define ULONG		0x60
#define ULONGLONG	0x70
#define	FLOAT		0x80
#define DOUBLE		0x90
#define VOID		0xA0
#define ELLIPSIS	0xB0

#define UNKNOWN		0xFE
#define ANY		0xFF

/* For non simple types the information index */
#define INFO(x)		(((x) >> 3) & 0x3FF)

#define CLASS(x)	((x) & 0xC000)
#define IS_SIMPLE(x)	(CLASS(x) == C_SIMPLE)
#define IS_STRUCT(x)	(CLASS(x) == C_STRUCT)
#define IS_FUNCTION(x)	(CLASS(x) == C_FUNCTION)
#define IS_ARRAY(x)	(CLASS(x) == C_ARRAY)

#define BASE_TYPE(x)	((x) & 0xF8)
#define IS_ARITH(x)	(!PTR(x) && BASE_TYPE(x) < VOID)
#define IS_INTARITH(x)	(!PTR(x) && BASE_TYPE(x) < FLOAT)

#define C_SIMPLE	0x0000
#define C_STRUCT	0x4000
#define C_FUNCTION	0x8000
#define C_ARRAY		0xC000		/* and other special later ? */

extern unsigned type_deref(unsigned t);
extern unsigned type_ptr(unsigned t);
extern unsigned type_sizeof(unsigned t);
extern unsigned type_ptrscale(unsigned t);
extern unsigned type_addrof(unsigned t);
extern unsigned type_ptrscale_binop(unsigned op, unsigned l, unsigned r, unsigned *div);

extern void skip_modifiers(void);
extern unsigned type_and_name(unsigned *np, unsigned needname, unsigned deftype);

extern unsigned is_modifier(void);
extern unsigned is_type_word(void);
