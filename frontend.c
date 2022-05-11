/*
 *	Tokenizer
 *
 *	It might be nicer to switch to an algorithm with less meta-data
 *	but we have to balance code size/data size/speed
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "token.h"
#include "target.h"

#define MAXSYM		512
#define	NAMELEN		16	/* Matches linker */

static int isoctal(unsigned char c)
{
	if (c >= '0' && c <= '7')
		return 1;
	return 0;
}

static int iscsymbol(unsigned char c)
{
	if (c == '_' || isalnum(c))
		return 1;
	return 0;
}

static int iscsymstart(unsigned char c)
{
	if (c == '_' || isalpha(c))
		return 1;
	return 0;
}

/*
 *	Glue for now
 */

static unsigned err;
static unsigned line = 1;
static unsigned oldline = 0;

void error(const char *p)
{
	fprintf(stderr, "%d: E: %s\n", line, p);
	err++;
}

void warning(const char *p)
{
	fprintf(stderr, "%d: W: %s\n", line, p);
}

void fatal(const char *p)
{
	error(p);
	exit(1);
}

static unsigned pushback;
static unsigned pbstack[2];
static unsigned isnl = 1;
static unsigned lastbslash;

static void directive(void);

unsigned get(void)
{
	int c;
	if (pushback) {
		c = pbstack[--pushback];
		pushback = 0;
		if (c == '\n') {
			isnl = 1;
			line++;
		}
		return c;
	}
	do {
		c = getchar();
		if (c == '#' && isnl) {
			fprintf(stderr, "directive\n");
			directive();
			c = getchar();
		}
	} while(c == '#');
	isnl = 0;
	if (c == '\n') {
		line++;
		isnl = 1;
	}
	/* backslash newline continuation */
	if (lastbslash && c == '\n')
		c = getchar();

	if (c == '\\')
		lastbslash = 1;
	else
		lastbslash = 0;

	if (c == EOF)
		return 0;
	return c;
}

unsigned get_nb(void)
{
	unsigned c;
	do {
		c = get();
	} while (c && isspace(c));
	return c;
}

void unget(unsigned c)
{
	if (pushback > 2)
		fatal("double pushback");
	pbstack[pushback++] = c;
	if (c == '\n')
		line--;
}

void required(unsigned cr)
{
	unsigned c = get();
	if (c != cr) {
		error("expected quote");
		unget(c);
	}
}

#define NHASH	64

/* We could infer the symbol number from the table position in theory */

struct symbol {
	char name[NAMELEN];
	uint16_t id;
	struct symbol *next;
};

static struct symbol symbols[MAXSYM];
static struct symbol *nextsym = symbols;
static struct symbol *symbase;	/* Base of post keyword symbols */
static struct symbol *symhash[NHASH];
/* Start of symbol range */
static unsigned symnum = T_SYMBOL;

/*
 *	Add a symbol to our symbol tables as we discover it. Log the
 *	fact if tracing.
 */
static struct symbol *new_symbol(const char *name, unsigned hash,
				 unsigned id)
{
	struct symbol *s;
	if (nextsym == symbols + MAXSYM)
		fatal("too many sybmols");
	s = nextsym++;
	strncpy(s->name, name, NAMELEN);
	s->next = symhash[hash];
	s->id = id;
	symhash[hash] = s;
	return s;
}

/*
 *	Find a symbol in a given has table	
 */
static struct symbol *find_symbol(const char *name, unsigned hash)
{
	struct symbol *s = symhash[hash];
	while (s) {
		if (strncmp(s->name, name, NAMELEN) == 0)
			return s;
		s = s->next;
	}
	return NULL;
}

/*
 *	A simple but adequate hashing algorithm. A better one would
 *	be worth it for performance.
 */
static unsigned hash_symbol(const char *name)
{
	int hash = 0;
	uint8_t n = 0;

	while (*name && n++ < NAMELEN)
		hash += *name++;
	return (hash & (NHASH - 1));
}

static void write_symbol_table(void)
{
	unsigned len = (uint8_t *) nextsym - (uint8_t *) symbase;
	uint8_t n[2];

	/* FIXME: proper temporary file! */
	int fd = open(".symtmp", O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd == -1) {
		perror(".symtmp");
		exit(1);
	}
	n[0] = len;
	n[1] = len  >> 8;
	if (write (fd, n, 2) != 2 || write(fd, symbase, len) != len)
		error("symbol I/O");
	close(fd);
}

/*
 *	Token stream writing. We have a single special case to handle
 *	which is strings.
 */


/* TODO: buffer this sensibly for speed */
static void outbyte(unsigned char c)
{
	if (write(1, &c, 1) != 1)
		error("I/O");
}

static void outbyte_quoted(unsigned char c)
{
	if (c == 0 || c == 0xFF)
		outbyte(0xFF);
	if (c == 0)
		outbyte(0xFE);
	else
		outbyte(c);
}

static unsigned char tokdata[8];
static unsigned char *tokptr = tokdata;

static void encode_byte(unsigned c)
{
	*tokptr++ = c;
}

static void write_token(unsigned c)
{
	unsigned char *tp;
	if (oldline != line) {
		oldline = line;
		write_token(T_LINE);
		outbyte(line);
		outbyte(line << 8);
	}
	/* Write the token, then any data for it */
	outbyte(c);
	outbyte(c >> 8);
	tp = tokdata;
	while (tp < tokptr)
		outbyte(*tp++);
	/* Reset the data pointer */
	tokptr = tokdata;
}

/* C keywords, ignoring all the modern crap */

static const char *keytab[] = {
	/* Types */
	"char",
	"double",
	"enum",
	"float",
	"int",
	"long",
	"short",
	"signed",
	"struct",
	"union",
	"unsigned",
	"void",
	/* Storage classes */
	"auto",
	"extern",
	"register",
	"static",
	/* Modifiers */
	"const",
	"volatile",
	/* Then the rest */
	"break",
	"case",
	"continue",
	"default",
	"do",
	"else",
	"for",
	"goto",
	"if",
	"return",
	"sizeof",
	"switch",
	"typedef",
	"while",
	NULL
};

/* Add keywords. These get added first so they head the hash lists */
static void keywords(void)
{
	const char **p = keytab;
	int i = T_KEYWORD;
	while (*p) {
		new_symbol(*p, hash_symbol(*p), i++);
		p++;
	}
	symbase = nextsym;
}

/* Read up to 14 more bytes into the symbol name, plus a terminator */
static void get_symbol_tail(char *p)
{
	unsigned n = 14;
	unsigned c;
	while ((c = get()) != 0) {
		if (!iscsymbol(c))
			break;
		if (n) {
			n--;
			*p++ = c;
		}
	}
	*p = 0;
	unget(c);
}

/* Also does keywords */
static unsigned tokenize_symbol(unsigned c)
{
	char symstr[16];
	unsigned h;
	struct symbol *s;
	*symstr = c;
	get_symbol_tail(symstr + 1);
	/* We can't do cunning tricks to spot labels in this pass because
	   foo: is ambiguous between a label and a ?: */
	h = hash_symbol(symstr);
	s = find_symbol(symstr, h);
	if (s)
		return s->id;
	return new_symbol(symstr, h, symnum++)->id;
}


static unsigned long decimal(unsigned char c)
{
	unsigned long val = c - '0';
	unsigned long prev;

	for (;;) {
		c = get();
		if (!isdigit(c)) {
			unget(c);
			break;
		}
		prev = val;
		val *= 10;
		val += c - '0';
		if (val < prev) {
			error("overflow");
			return 0;
		}
	}
	return val;
}

static unsigned unhex(unsigned c)
{
	if (c <= '9')
		return c - '0';
	else
		return tolower(c) - 'a' + 10;
}

static unsigned long hexadecimal(void)
{
	unsigned long val = 0;
	unsigned long prev;
	unsigned c;

	for (;;) {
		c = get();
		if (!isxdigit(c)) {
			unget(c);
			break;
		}
		prev = val;
		val *= 10;
		val += unhex(c);
		if (val < prev) {
			error("overflow");
			return 0;
		}
	}
	return val;
}

static unsigned long octal(void)
{
	unsigned long val = 0;
	unsigned long prev;
	unsigned c;

	c = get();
	if (c == 'x' || c == 'X')
		return hexadecimal();
	unget(c);

	for (;;) {
		c = get();
		if (!isoctal(c)) {
			unget(c);
			break;
		}
		prev = val;
		val <<= 3;
		val += c - '0';
		if (val < prev) {
			error("overflow");
			return 0;
		}
	}
	return val;
}

/* # directive from cpp # line file - # line "file" */
/* TODO file name saving */
static void directive(void)
{
	unsigned c = get_nb();
	if (!isdigit(c))
		fatal("bad cpp");
	line = decimal(c);
	/* don't yet look for file name */
	while ((c = get()) != 0) {
		fputc(c, stderr);
		if (c == '\n') {
			isnl = 1;
			return;
		}
	}
	fatal("bad cpp");
}


/*
 *	TODO
 *	float, double, longlong
 */
static unsigned tokenize_numeric(int sign, unsigned c)
{
	unsigned long val;
	unsigned signmod = 0;
	unsigned force_unsigned = 0;
	unsigned force_long = 0;
	unsigned type;
	/* Two's complement assumed so -(n+1) <= x <= n */
	if (sign)
		signmod = 1;

	if (c == '0')
		val = octal();
	else
		val = decimal(c);

	while (1) {
		c = get();
		if ((c == 'U' || c == 'u') && !force_unsigned)
			force_unsigned = 1;
		else if ((c == 'L' || c == 'l') && !force_long)
			force_long = 1;
		else {
			unget(c);
			break;
		}
	}
	/* FIXME: deal with 32bit + sign overflow */
	if (sign == -1)
		val = -val;
	/* Anything can be shoved in a ulong */
	type = T_ULONGVAL;
	/* Will it fit in a uint ? */
	if (!force_long && val < TARGET_MAX_UINT)
		type = T_UINTVAL;
	if (!force_unsigned) {
		/* Maybe a signed long then ? */
		if (val < TARGET_MAX_LONG + signmod)
			type = T_LONGVAL;
		/* Will it fit in a signed integer ? */
		if (!force_long && val < TARGET_MAX_INT + signmod)
			type = T_INTVAL;
	}
	if (sign == -1)
		val = -val;
	/* Order really doesn't matter here so stick to LE. We will worry about 
	   actual byte order in the code generation */
	encode_byte(val);
	encode_byte(val >> 8);
	encode_byte(val >> 16);
	encode_byte(val >> 24);
	return type;
}

static unsigned tokenize_number(unsigned c)
{
	return tokenize_numeric(1, c);
}

static unsigned hexpair(void)
{
	unsigned c, c2;
	c = get();
	if (!isxdigit(c)) {
		warning("invalid hexadecimal escape");
		return T_INVALID;
	}
	c2 = get();
	if (!isxdigit(c2)) {
		warning("invalid hexadecimal escape");
		return T_INVALID;
	}
	return (unhex(c2) << 4) | unhex(c);
}

static unsigned octalset(unsigned c)
{
	unsigned int n = c - '0';
	int ct = 0;
	while (ct++ < 3) {
		c = get();
		if (!isoctal(c)) {
			unget(c);
			return n;
		}
		n <<= 3;
		n |= c - '0';
	}
	return n;
}

static unsigned escaped(unsigned c)
{
	/* Simple cases first */
	switch (c) {
	case 'a':
		return 0x07;
	case 'b':
		return 0x08;
	case 'e':
		return 0x1B;	/* Non standard but common */
	case 'f':
		return 0x0C;
	case 'n':
		return 0x0A;
	case 'r':
		return 0x0D;
	case 't':
		return 0x09;
	case 'v':
		return 0x0B;
	case '\\':
		return '\\';
	case '\'':
		return '\'';
	case '"':
		return '"';
	case '?':
		return '?';	/* Not that we suport the trigraph nonsense */
	}
	/* Now the numerics */
	if (c == 'x')
		return hexpair();
	if (isdigit(c))
		return octalset(c);
	warning("invalid escape code");
	return T_INVALID;
}

static unsigned tokenize_char(void)
{
	unsigned c = get();
	unsigned c2;
	if (c != '\\') {
		/* Encode as a value */
		encode_byte(c);
		encode_byte(0);
		encode_byte(0);
		encode_byte(0);
		c = get();
		if (c != '`') {
			unget(c);
			required('\'');
		}
		return T_INTVAL;
	}
	c2 = get();
	required('\'');
	c = escaped(c2);
	if (c == T_INVALID)
		/* Not a valid escape */
		encode_byte(c2);
	else
		encode_byte(c);
	encode_byte(0);
	encode_byte(0);
	encode_byte(0);
	return T_INTVAL;
}

static unsigned tokenize_string(void)
{
	/* We escape any internal \0 or \FF so we can parse this without
	   buffers, and likewise write it to data the other end the same way */
	unsigned c, c2;
	write_token(T_STRING);

	/* This is slightly odd because we do the string catenation here too */
	do {
		while ((c = get()) != '"') {
			if (c != '\\') {
				outbyte_quoted(c);
			} else {
				c2 = get();
				c = escaped(c2);
				if (c == T_INVALID)
					outbyte_quoted(c2);
				else
					outbyte_quoted(c);
			}
		}
		c = get_nb();
	} while (c == '"');
	unget(c);
	outbyte(0);
	return T_STRING_END;
}

static char *doublesym = "+-=<>|&";
static char *symeq = "+-/*^!|&%<>";
static char *unibyte = "()[]{}&*/%+-?:^<>|~!=;.,";

static unsigned tokenize(void)
{
	unsigned c, c2, c3;
	char *p;

	c = get_nb();
	if (c == 0)
		return T_EOF;
	if (iscsymstart(c))
		return tokenize_symbol(c);
	if (isdigit(c))
		return tokenize_number(c);
	if (c == '\'')
		return tokenize_char();
	if (c == '"')
		return tokenize_string();
	/* Look for things like ++ and the special case of -n for constants */
	c2 = get();
/*	if (c == '-' && isdigit(c2))
		return tokenize_neg(c2); */
	if (c2 == c) {
		p = strchr(doublesym, c);
		if (p) {
			if (c == '<' || c == '>') {
				c3 = get();
				if (c3 == '=') {
					if (c == '<')
						return T_SHLEQ;
					return T_SHREQ;
				}
				unget(c3);
			}
			/* Double sym */
			return T_DOUBLESYM + p - doublesym;
		}
	}
	/* Now deal with the other double symbol cases */
	if (c == '-' && c2 == '>')
		return T_POINTSTO;
	if (c == '.' && c2 == '.') {
		c3 = get();
		if (c3 == '.')
			return T_ELLIPSIS;
		unget(c3);
	}
	/* The '=' cases */
	if (c2 == '=') {
		p = strchr(symeq, c);
		if (p)
			return T_SYMEQ + p - symeq;
	}
	unget(c2);
	/* Symbols that only have a 1 byte form */
	p = strchr(unibyte, c);
	if (p)
		return c;	/* Map to self */
	/* Not valid C */
	error("nonsense in C");
	/* I'm a teapot */
	return T_POT;
}

/* Tokenizer as a standalone pass */
int main(int argc, char *argv[])
{
	unsigned t;
	keywords();
	do {
		t = tokenize();
		write_token(t);
	} while (t != T_EOF);
	write_symbol_table();
	return err;
}
