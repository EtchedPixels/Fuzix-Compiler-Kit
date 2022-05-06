/*
 *	This is the main block for the code generator. It provides the
 *	basic parsing functions to make life easy for the target code
 *	generator. A target is not required to use this, it can work the
 *	tree/header mix any way it wants.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "compiler.h"
#include "backend.h"

static const char *argv0;

void error(const char *p)
{
	fprintf(stderr, "%s: error: %s\n", argv0, p);
	exit(1);
}

static void xread(int fd, void *buf, int len)
{
	if (read(fd, buf, len) != len)
		error("short read");
}

/*
 *	Name symbol table.
 */
#define NAMELEN 16
struct name {
	char name[NAMELEN];
	uint16_t id;
	struct symbol *next;
};

static struct name names[MAXSYM];

char *namestr(unsigned n)
{
	if (n < 0x8000)
		error("bad name");
	return names[n - 0x8000].name;
}

/*
 *	Expression tree nodes
 */
#define NUM_NODES 100

static struct node node_table[NUM_NODES];
static struct node *nodes;

struct node *new_node(void)
{
	struct node *n;
	if (nodes == NULL)
		error("Too many nodes");
	n = nodes;
	nodes = n->right;
	n->left = n->right = NULL;
	n->value = 0;
	n->flags = 0;
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

/* I/O buffering stuff can wait - as can switching to a block write method */
static struct node *load_tree(void)
{
	struct node *n = new_node();
	xread(0, n, sizeof(struct node));

	/* The values off disk are old pointers or NULL, that's good enough
	   to use as a load flag */
	if (n->left)
		n->left = load_tree();
	if (n->right)
		n->right = load_tree();
	return n;
}

static struct node *rewrite_tree(struct node *n)
{
	if (n->left)
		n->left = rewrite_tree(n->left);
	if (n->right)
		n->right = rewrite_tree(n->right);
	return gen_rewrite_node(n);
}

static unsigned process_expression(void)
{
	struct node *n = rewrite_tree(load_tree());
	gen_tree(n);
	return n->type;
}

static unsigned compile_expression(void)
{
	uint8_t h[2];
	xread(0, h, 2);
	if (h[0] != '%' || h[1] != '^')
		error("expression sync");
	return process_expression();
}

/*
 *	Process the header blocks. We call out to the target to let it
 *	handle the needs of the platform.
 */

static unsigned func_ret;
static unsigned frame_len;

static void process_header(void)
{
	struct header h;
	xread(0, &h, sizeof(struct header));

	switch (h.h_type) {
	case H_EXPORT:
		gen_export(namestr(h.h_name));
		break;
	case H_FUNCTION:
		gen_prologue(namestr(h.h_data));
		func_ret = h.h_name;
		break;
	case H_FRAME:
		gen_frame(h.h_name);
		break;
	case H_FUNCTION | H_FOOTER:
		gen_label("_r", h.h_name);
		gen_epilogue(frame_len);
		break;
	case H_FOR:
		compile_expression();
		gen_label("_c", h.h_data);
		compile_expression();
		gen_jfalse("_b", h.h_data);
		gen_jump("_n", h.h_data);
		compile_expression();
		break;
	case H_FOR | H_FOOTER:
		gen_label("_b", h.h_data);
		gen_jump("_c", h.h_data);
		break;
	case H_WHILE:
		gen_label("_c", h.h_data);
		compile_expression();
		gen_jfalse("_b", h.h_data);
		break;
	case H_WHILE | H_FOOTER:
		gen_jump("_c", h.h_data);
		gen_label("_b", h.h_data);
		break;
	case H_DO:
		gen_label("_c", h.h_data);
		break;
	case H_DOWHILE:
		compile_expression();
		gen_jtrue("_c", h.h_data);
		break;
	case H_DO | H_FOOTER:
		gen_jump("_c", h.h_data);
		gen_label("_b", h.h_data);
		break;
	case H_BREAK:
		gen_jump("_b", h.h_name);
		break;
	case H_CONTINUE:
		gen_jump("_c", h.h_name);
		break;
	case H_IF:
		compile_expression();
		gen_jfalse("_e", h.h_name);
		break;
	case H_ELSE:
		gen_jump("_f", h.h_name);
		gen_label("_e", h.h_name);
		break;
	case H_IF | H_FOOTER:
		/* If we have an else then _f is needed, if not _e is */
		if (h.h_data)
			gen_label("_f", h.h_name);
		else
			gen_label("_e", h.h_name);
		break;
	case H_RETURN:
		gen_jump("_r", func_ret);
		break;
	case H_LABEL:
		gen_label("", h.h_name);
		break;
	case H_GOTO:
		/* TODO - goto might be special ? */
		gen_jump("", h.h_name);
		break;
	case H_SWITCH:
		/* Will need to stack switch case tables somehow */
		/* FIXME */
		gen_switch_begin(h.h_name, compile_expression());	/* need the type of it back */
		break;
	case H_CASE:
		gen_case(compile_expression());	/* Will be a const expr */
		break;
	case H_SWITCH | H_FOOTER:
		gen_label("_b", h.h_name);
		gen_switch(h.h_name);
		break;
	case H_DATA:
		gen_data(namestr(h.h_name));
		break;
	case H_DATA | H_FOOTER:
		gen_code();
		break;
	case H_BSS:
		gen_bss(namestr(h.h_name));
		break;
	case H_BSS | H_FOOTER:
		gen_code();
		break;
	default:
		error("bad hdr");
		break;
	}
}

/* Each data node is a one node tree right now. We ought to trim this down
   to avoid bloating the intermediate file */

void process_data(void)
{
	struct node *n = load_tree();
	switch (n->op) {
	case T_PAD:
		gen_space(n->value);
		break;
	case T_LABEL:
		gen_text_label(n->value);
		break;
	default:
		gen_value(n->type, n->value);
		break;
	}
	free_node(n);
}

/*
 *	Load the symbol table from the front end
 */

static void load_symbols(const char *path)
{
	int fd = open(path, O_RDONLY);
	uint8_t n[2];
	if (fd == -1) {
		perror(path);
		exit(1);
	}
	xread(fd, n, 2);
	xread(fd, names, n[0] | (n[1] << 8));
	close(fd);
}

int main(int argc, char *argv[])
{
	int c;
	uint8_t h[2];

	argv0 = argv[0];

	/* We can make this better later */
	if (argc != 2)
		error("arguments");
	load_symbols(argv[1]);
	init_nodes();

	gen_start();
	while (read(0, &h, 2) > 0) {
		if (h[0] != '%')
			error("sync");
		/* We write a sequence of records starting %^ for an expression
		   %[ for data blocks and %H for a header. This helps us track any
		   errors and sync screwups when parsing */
		if (h[1] == '^')
			process_expression();
		else if (h[1] == 'H')
			process_header();
		else if (h[1] == '[')
			process_data();
		else
			error("unknown block");
	}
	gen_end();
}

/*
 *	Helpers for the targets
 */

/*
 *	Perform a simple left right walk of the tree and feed the code
 *	to the node generator.
 *
 *	TODO; constant spotting and helper
 */
void codegen_lr(struct node *n)
{
	if (n->left) {
		codegen_lr(n->left);
		gen_push(n->left->type);
	}
	if (n->right)
		codegen_lr(n->right);
	gen_node(n);
}
