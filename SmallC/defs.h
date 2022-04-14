/*
 * File defs.h: 2.1 (83/03/21,02:07:20)
 */

// Intel 8080 / Z80 architecture defs
#define INTSIZE 2

// miscellaneous
#define FOREVER for(;;)
#define FALSE   0
#define TRUE    1
#define NO      0
#define YES     1

#define EOS     0
#define LF      10
#define BKSP    8
#define CR      13
#define FFEED   12
#define TAB     9

struct symbol {
	unsigned name;		// Name is now a token value
	unsigned char storage;        // public, auto, extern, static, lstatic, defauto
	unsigned type;               // char, int, uchar, unit
	int offset;             // offset
	int tagidx;             // index of struct in tag table
};
#define SYMBOL struct symbol

#define NUMBER_OF_GLOBALS 100
#define NUMBER_OF_LOCALS 20

// Define the structure tag table parameters
#define NUMTAG		10

struct tag_symbol {
	unsigned name;		// structure tag name (token code)
	int size;               // size of struct in bytes
	int member_idx;         // index of first member
	int number_of_members;  // number of tag members
};
#define TAG_SYMBOL struct tag_symbol

#ifdef SMALL_C
#define NULL_TAG 0
#else
#define NULL_TAG (TAG_SYMBOL *)0
#endif

// Define the structure member table parameters
#define NUMMEMB		30

// possible entries for "ident"
#define VARIABLE        1
#define ARRAY           2
#define POINTER         3
#define FUNCTION        4

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

/* possible entries for storage: to change */

#define PUBLIC  1
#define AUTO    2
#define EXTERN  3

#define STATIC  4
#define LSTATIC 5
#define DEFAUTO 6

#define REGISTER 7

// "do"/"for"/"while"/"switch" statement stack
#define WSTABSZ 20

struct while_rec {
	int symbol_idx;		// symbol table address
	int stack_pointer;	// stack pointer
	int type;           // type
	int case_test;		// case or test
	int incr_def;		// continue label ?
	int body_tab;		// body of loop, switch ?
	int while_exit;     // exit label
};
#define WHILE struct while_rec

/* possible entries for "wstyp" */
#define WSWHILE 0
#define WSFOR   1
#define WSDO    2
#define WSSWITCH        3

/* "switch" label stack */
#define SWSTSZ  100

/* statement types (tokens) */
#define STIF        1
#define STWHILE     2
#define STRETURN    3
#define STBREAK     4
#define STCONT      5
#define STASM       6
#define STEXP       7
#define STDO        8
#define STFOR       9
#define STSWITCH    10

/**
 * Output the variable symbol at scptr as an extrn or a public
 * @param scptr
 */
void ppubext(SYMBOL *scptr);

/**
 * Output the function symbol at scptr as an extrn or a public
 * @param scptr
 */
void fpubext(SYMBOL *scptr);

/**
 * fetch a static memory cell into the primary register
 * @param sym
 */
void gen_get_memory(SYMBOL *sym);

/**
 * fetch the specified object type indirect through the primary
 * register into the primary register
 * @param typeobj object type
 */
void gen_get_indirect(char typeobj, int reg);

/**
 * asm - store the primary register into the specified static memory cell
 * @param sym
 */
void gen_put_memory(SYMBOL *sym);

// initialisation of global variables
#define INIT_TYPE    NAMESIZE
#define INIT_LENGTH  NAMESIZE+1
#define INITIALS_SIZE 5*1024

/* For arrays we need to use the type and point the type at the object
   info so we can do multi-dimensional arrays properly */
struct initials_table {
	unsigned name;		// symbol name
	int type;               // type
	int dim;                // length of data (possibly an array)
    int data_len;               // index of tag or zero
};
#define INITIALS struct initials_table

#include "prototype.h"

#include "tokens.h"

extern unsigned token;
extern unsigned token_value;

extern unsigned line_num;

#include "tree.h"
#include "header.h"
#include "type.h"