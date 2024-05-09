#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"
#include "backend-6800.h"

/*
 *	Start with some simple X versus S tracking
 *	to clean up the local accesses
 */

uint16_t x_fpoff;
unsigned x_fprel;
uint8_t a_val;
uint8_t b_val;
unsigned a_valid;
unsigned b_valid;
unsigned d_valid;
static struct node d_node;

void invalidate_all(void)
{
	x_fprel = 0;
	a_valid = 0;
	b_valid = 0;
	d_valid = 0;
}

void invalidate_x(void)
{
	x_fprel = 0;
}

void invalidate_a(void)
{
	a_valid = 0;
}

void invalidate_b(void)
{
	b_valid = 0;
}

void invalidate_d(void)
{
	d_valid = 0;
}

void invalidate_work(void)
{
	a_valid = 0;
	b_valid = 0;
	d_valid = 0;
}

void invalidate_mem(void)
{
	/* If memory changes it might be an alias to the value cached in AB */
	switch(d_node.op) {
	case T_LREF:
	case T_LBREF:
	case T_NREF:
		d_valid = 0;
	}
}

void set_d_node(struct node *n)
{
	memcpy(&d_node, n, sizeof(struct node));
	switch(d_node.op) {
	case T_LSTORE:
		d_node.op = T_LREF;
		break;
	case T_LBSTORE:
		d_node.op = T_LBREF;
		break;
	case T_NSTORE:
		d_node.op = T_NREF;
		break;
	case T_RSTORE:
		d_node.op = T_RREF;
		break;
	case T_NREF:
	case T_LBREF:
	case T_LREF:
	case T_NAME:
	case T_LABEL:
	case T_LOCAL:
	case T_ARGUMENT:
		break;
	default:
		d_valid = 0;
		return;
	}
	d_valid = 1;
}

/* D holds the content pointed to by n */
void set_d_node_ptr(struct node *n)
{
	memcpy(&d_node, n, sizeof(struct node));
	switch(d_node.op) {
	case T_NAME:
		d_node.op = T_NREF;
		break;
	case T_LABEL:
		d_node.op = T_LBREF;
		break;
	case T_ARGUMENT:
		d_node.value += argbase + frame_len;
		/* Fall through */
	case T_LOCAL:
		d_node.op = T_LREF;
		break;
	default:
		d_valid = 0;
		return;
	}
	d_valid = 1;
}

/* Do we need to check fields by type or will the default filling
   be sufficient ? */
unsigned d_holds_node(struct node *n)
{
	if (d_valid == 0)
		return 0;
	if (d_node.op != n->op)
		return 0;
	if (d_node.val2 != n->val2)
		return 0;
	if (d_node.snum != n->snum)
		return 0;
	if (d_node.value != n->value)
		return 0;
	return 1;
}

void modify_a(uint8_t val)
{
	a_val = val;
	d_valid = 0;
}

void modify_b(uint8_t val)
{
	b_val = val;
	d_valid = 0;
}
