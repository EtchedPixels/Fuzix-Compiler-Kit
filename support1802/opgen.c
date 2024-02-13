#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned num;	/* 0 is reserved in each for op bank switch */
static unsigned num2;

static const char *opname[256];

static const char *set_opname(unsigned n, const char *p, const char *s, const char *t)
{
    char buf[32];
    snprintf(buf, 32, "%s%s%s", p, s, t);
    const char *x = strdup(buf);
    if (n > 510) {
        fprintf(stderr, "Error: too many symbols.\n");
        exit(1);
    }
    n >>= 1;
    if (x) {
        if (opname[n]) {
            fprintf(stderr, "Error: two symbols at %u\n", n);
            exit(1);
        }
        opname[n] = x;
    } else {
        fprintf(stderr, "Error: out of memory.\n");
        exit(1);
    }
    return x;
}

static void make_ops(FILE *h, const char *t, const char *o, unsigned s, const char *end)
{
    const char *p = set_opname(num, t, "", end);
    fprintf(h, "#define op_%-16.16s\t0x00%02X\n", p, num);
    num += 2;
    if (s) {
        p = set_opname(num, t, "u", end);
        fprintf(h, "#define op_%-16.16s\t0x00%02X\n", p, num);
        num += 2;
    }
}

static void make_ops2(FILE *h, const char *t, const char *o, unsigned s, const char *end)
{
    const char *p = set_opname(num2 + 256, t, "", end);
    fprintf(h, "#define op_%-16.16s\t0x01%02X\n", p, num2);
    num2 += 2;
    if (s) {
        p = set_opname(num2 + 256, t, "u", end);
        fprintf(h, "#define op_%-16.16s\t0x01%02X\n", p, num2);
        num2 += 2;
    }
}

static void process_op(FILE *h, char *buf)
{
    char *t, *o;
    unsigned sign = 0, withc = 0, nolong = 0, withf = 0, novar = 0, reg = 0;
    buf++;
    t = strtok(buf, " \t\n");
    if (t == NULL)
        return;
    while((o = strtok(NULL, " \t\n")) != NULL) {
        if (*o == 'T')
            break;
        while(*o) {
            switch(*o) {
                case 'r':	
                    reg = 1;
                    break;
                case 's':
                    sign = 1;
                    break;
                case 'c':
                    withc = 1;
                    break;
                case 'i':
                    nolong = 1;
                    break;
                case 'f':
                    withf = 1;
                    break;
                case '-':
                    novar = 1;
                    break;
                case '2':
                    novar = 2;
                    break;
                default:
                    fprintf(stderr, "token '%s' bad option '%c'\n", t, *o);
                    exit(1);
            }
            o++;
        }
    }
    if (novar) {
        if (novar == 1)
            make_ops(h, t, o, 0, "");
        else
            make_ops2(h, t, o, 0, "");
        return;
    }
    if (withc) {
        if (reg)
            make_ops2(h, t, o, sign, "c");
        else
            make_ops(h, t, o, sign, "c");
    }
    if (withf)
        make_ops2(h, t, o, 0, "f");
    if (!nolong)
        make_ops2(h, t, o, sign, "l");
    if (reg)
        make_ops2(h, t, o, sign, "");
    else
        make_ops(h, t, o, sign, "");
}

int main(int argc, char *argv[])
{
    unsigned i;
    char buf[512];
    FILE *o = fopen("1802ops.h", "w");
    if (o == NULL) {
        perror("1802ops.h");
        exit(1);
    }
    while(fgets(buf, 512, stdin)) {
        if (*buf == '#')
            continue;
        else if (*buf == '%')
            process_op(o,buf);
        else if (*buf != '\n')
            fprintf(stderr, "?? %s", buf);
    }
    fclose(o);
    o = fopen("1802debug.h", "w");
    if (o == NULL) {
        perror("1802debug.h");
        exit(1);
    }
    fprintf(o, "const char *opnames[] = {\n");
    for (i = 0; i < 256; i++) {
        if (opname[i])
            fprintf(o, "\t\"%s\",\n", opname[i]);
        else
            fprintf(o, "\tNULL,\n");
    }
    fprintf(o, "};\n");
    fclose(o);
    o = fopen("1802tab.s", "w");
    if (o == NULL) {
        perror("1802tab.s");
        exit(1);
    }
    fprintf(o, "byteop:	; Must be page aligned\n");
    for (i = 0; i < 256; i++) {
        if (opname[i])
            fprintf(o, "\t.word op_%s\n", opname[i]);
        else
            fprintf(o, "\t.word op_invalid\n");
    }
    fclose(o);
    return 0;
}
