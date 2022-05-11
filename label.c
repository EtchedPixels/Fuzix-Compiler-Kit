/*
 *	Labels look like symbols except that they don't really behave
 *	like them in terms of scope. Handle them on their own
 */

#include <stdint.h>
#include "compiler.h"

#define L_DECLARED	0x8000

struct label {
    uint16_t name;
    uint16_t line;
};

struct label labels[MAXLABEL];
struct label *labelp;

void init_labels(void)
{
    labelp = labels;
}

static void new_label(unsigned n)
{
    if (labelp == &labels[MAXLABEL])
        error("too many goto labels");
    labelp->name = n;
    labelp->line = line_num;
    labelp++;
}

void use_label(unsigned n)
{
    struct label *p = labels;
    n &= 0x7FFF;
    while(p < labelp) {
        if (n == (p->name & 0x7FFF))
            return;
        p++;
    }
    new_label(n);
}

void add_label(unsigned n)
{
    new_label(n | L_DECLARED);
}

void check_labels(void)
{
    struct label *p = labels;
    while(p < labelp) {
        if (!(p->name & L_DECLARED))
            errorline(p->line, "unknown label");
        p++;
    }
}
