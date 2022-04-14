/*      File code8080.c: 2.2 (84/08/31,10:05:09) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

/*
 *      Some predefinitions:
 *
 *      INTSIZE is the size of an integer in the target machine
 *      BYTEOFF is the offset of an byte within an integer on the
 *              target machine. (ie: 8080,pdp11 = 0, 6809 = 1,
 *              360 = 3)
 *      This compiler assumes that an integer is the SAME length as
 *      a pointer - in fact, the compiler uses INTSIZE for both.
 */

/**
 * prints new line
 * @return 
 */
void newline (void) {
    output_byte (LF);
}

/**
 * Output internal generated label prefix
 */
void output_label_prefix(void) {
    output_byte('L');
}

/**
 * Output a label definition terminator
 */
void output_label_terminator (void) {
    output_byte (':');
}

/**
 * Output a C label with leading _
 */
void output_label_name(unsigned sym)
{
    output_byte('_');
    output_name(sym);
}

/**
 * begin a comment line for the assembler
 */
void gen_comment(void) {
    output_byte (';');
}

/**
 * print any assembler stuff needed after all code
 */
void trailer(void) {
    output_line (";\t.end");
}

/**
 * text (code) segment
 */
void code_segment_gtext(void) {
    output_line ("\t.code");
}

/**
 * data segment
 */
void data_segment_gdata(void) {
    output_line ("\t.data");
}

/**
 * Output the variable symbol at scptr as an extrn or a public
 * @param scptr
 */
void ppubext(SYMBOL *scptr)  {
    if (symbol_table[current_symbol_table_idx].storage == STATIC) return;
    if (scptr->storage != EXTERN) {
        output_with_tab(".globl\t");
        output_name (scptr->name);
        newline();
    }
}

/**
 * Output the function symbol at scptr as an extrn or a public
 * @param scptr
 */
void fpubext(SYMBOL *scptr) {
    if (scptr->storage == STATIC) return;
    if (scptr->offset == FUNCTION) {
        output_with_tab (".globl\t");
        output_name (scptr->name);
        newline ();
    }
}

/**
 * Output a decimal number to the assembler file, with # prefix
 * @param num
 */
void output_number(int num) {
    output_decimal(num);
}

/**
 * platform level analysis of whether a symbol access needs to be
 * direct or indirect (globals are always direct)
 */
int gen_indirected(SYMBOL *s)
{
    if (s->storage == LSTATIC)
        return 0;
    return 1;
}

/**
 * declare entry point
 */
void declare_entry_point(unsigned symbol_name) {
    output_name(symbol_name);
    output_label_terminator();
    //newline();
}

/**
 * print pseudo-op  to define a byte
 */
void gen_def_byte(void) {
    output_with_tab (".db\t");
}

/**
 * print pseudo-op to define storage
 */
void gen_def_storage(void) {
    output_with_tab (".ds\t");
}

/**
 * print pseudo-op to define a word
 */
void gen_def_word(void) {
    output_with_tab (".dw\t");
}

int gen_register(int size, int typ)
{
    /* For the moment */
    return -1;
}

