/* Pass 1 values */
#define MAXSYM		512
#define	NAMELEN		16		/* Matches linker */

/* Pass 2 values */
#define NUMGLOBAL	384
#define NUMLOCAL	128
#define FUNCSIZE	512

/* Tokens */
#define T_SYMBOL	0x8000	/* Upwards */

/* Special control symbols */
#define T_EOF		0x7F00
#define T_INVALID	0x7F01
#define T_POT		0x7F02

/* Special case symbols */
#define T_SHLEQ		0x0010
#define T_SHREQ		0x0011
#define T_POINTSTO	0x0012

/* Two repeats of a symbol */
#define T_DOUBLESYM	0x0020
#define T_PLUSPLUS	0x0020
#define T_MINUSMINUS	0x0021
#define T_EQEQ		0x0022
#define T_LTLT		0x0023
#define T_GTGT		0x0024
#define T_OROR		0x0025
#define T_ANDAND	0x0026

/* Symbol followed by equals */
#define T_SYMEQ		0x0100
#define T_PLUSEQ	0x0100
#define T_MINUSEQ	0x0101
#define T_SLASHEQ	0x0102
#define T_STAREQ	0x0103
#define T_HATEQ		0x0104
#define T_BANGEQ	0x0105
#define T_OREQ		0x0106
#define T_ANDEQ		0x0107
#define T_PERCENTEQ	0x0108
#define T_LTEQ		0x0109
#define T_GTEQ		0x010A

/* Symbols with a semantic meaning */
#define T_UNI		0x0200
#define T_LPAREN	0x0200
#define T_RPAREN	0x0201
#define T_LSQUARE	0x0202
#define T_RSQUARE	0x0203
#define T_LCURLY	0x0204
#define T_RCURLY	0x0205
#define T_AND		0x0206
#define T_STAR		0x0207	/* We change this to T_DEREF for uni form */
#define T_SLASH		0x0208
#define T_PERCENT	0x0209
#define T_PLUS		0x020A
#define T_MINUS		0x020B
#define T_QUESTION	0x020C
#define T_COLON		0x020D
#define T_HAT		0x020E
#define T_LT		0x020F
#define T_GT		0x0210
#define T_OR		0x0211
#define T_TILDE		0x0212
#define T_BANG		0x0213
#define T_EQ		0x0214
#define T_SEMICOLON	0x0215
#define T_DOT		0x0216
#define T_COMMA		0x0217
/* We process strings and quoting of strings so " ' and \ are not seen */

/* The C language keywords */		
#define T_KEYWORD	0x1000

/* Type keywords */
#define T_CHAR		0x1000
#define T_DOUBLE	0x1001
#define T_ENUM		0x1002
#define T_FLOAT		0x1003
#define T_INT		0x1004
#define T_LONG		0x1005
#define T_SHORT		0x1006
#define T_SIGNED	0x1007
#define T_STRUCT	0x1008
#define T_UNION		0x1009
#define T_UNSIGNED	0x100A
#define T_VOID		0x100B
/* Storage classes */
#define T_AUTO		0x100C
#define T_EXTERN	0x100D
#define T_REGISTER	0x100E
#define	T_STATIC	0x100F
/* Modifiers */
#define T_CONST		0x1010
#define T_VOLATILE	0x1011
/* Other keywords */
#define T_BREAK		0x1012
#define T_CASE		0x1013
#define T_CONTINUE	0x1014
#define T_DEFAULT	0x1015
#define T_DO		0x1016
#define T_ELSE		0x1017
#define T_FOR		0x1018
#define T_GOTO		0x1019
#define T_IF		0x101A
#define T_RETURN	0x101B
#define T_SIZEOF	0x101C
#define T_SWITCH	0x101D
#define T_TYPEDEF	0x101E
#define T_WHILE		0x101F

/* Encodings for tokenized constants */
/* These are followed by a 4 byte little endian value */
#define T_INTVAL	0x1100
#define T_UINTVAL	0x1101
#define T_LONGVAL	0x1102
#define T_ULONGVAL	0x1103
/* This is followed by a literal bytestream terminated by a 0. 0 and 255 in the
   stream are encoded as 255,0 and 255,255. */
#define T_STRING	0x1104
/* End marker for strings (mostly a dummy for convenience) */
#define T_STRING_END	0x1105

/* Encodings for disambiguation and internal use */
#define T_ADDROF	0x1200
#define T_DEREF		0x1201
#define T_NEGATE	0x1202
#define T_POSTINC	0x1203
#define T_POSTDEC	0x1204
#define T_FUNCCALL	0x1205
#define T_NAME		0x1206
#define T_LABEL		0x1207
#define T_NULL		0x1208	/* expression not present */

#define T_LINE		0x3FFF	/* Line number encoding scheme */
