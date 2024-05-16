#include <stdio.h>

#include "ee200.h"

/* Disassembler */

static const char *r8map[16] = {
	"AH", "AL",
	"BH", "BL",
	"XH", "XL",
	"YH", "YL",
	"ZH", "ZL",
	"SH", "SL",
	"CH", "CL",
	"PH", "PL"
};

static const char *r16map[16] = {
	"A", "AHH",
	"B", "BHH",
	"X", "XHH",
	"Y", "YHH",
	"Z", "ZHH",
	"S", "SHH",
	"C", "CHH",
	"P", "PHH"
};

static const char *r8name(unsigned n)
{
	n &= 0x0F;
	return r8map[n];
}

static const char *r16name(unsigned n)
{
	n &= 0x0F;
	return r16map[n];
}

static uint16_t get8d(unsigned rpc)
{
	return mem_read8_debug(rpc);
}
static uint16_t get16d(unsigned rpc)
{
	uint16_t n = mem_read8_debug(rpc) << 8;
	n |= mem_read8_debug(rpc + 1);
	return n;
}

static void dis16d(unsigned rpc)
{
	uint16_t n = get16d(rpc);
	fprintf(stderr, "%04X", n);
}

static void disindexed(unsigned rpc)
{
	unsigned r = mem_read8_debug(rpc);
	if (r & 4)
		fputc('@', stderr);
	if (r & 8)
		fprintf(stderr, "%d", mem_read8_debug(rpc + 1));
	switch (r & 3) {
	case 0:
		fprintf(stderr, "(%s)", r16name(r >> 4));
		break;
	case 1:
		fprintf(stderr, "(%s+)", r16name(r >> 4));
		break;
	case 2:
		fprintf(stderr, "(-%s)", r16name(r >> 4));
		break;
	case 3:
		fprintf(stderr, "Bad indexing mode.");
		break;
	}
}

static void disaddr(unsigned rpc, unsigned size, unsigned op,
		    unsigned isjump)
{
	switch (op) {
	case 0:
		if (size == 1)
			fprintf(stderr, "%02X", mem_read8_debug(rpc));
		else
			dis16d(rpc);
		break;
	case 1:
		if (!isjump)
			fputc('(', stderr);
		dis16d(rpc);
		if (!isjump)
			fputc(')', stderr);
		break;
	case 2:
		if (!isjump)
			fputc('@', stderr);
		fputc('(', stderr);
		dis16d(rpc);
		fputc(')', stderr);
		break;
	case 3:
		fprintf(stderr, "(PC+%d)", (int8_t) mem_read8_debug(rpc));
		break;
	case 4:
		fprintf(stderr, "@(PC+%d)", (int8_t) mem_read8_debug(rpc));
		break;
	case 5:
		disindexed(rpc);
		break;
	case 6:
	case 7:
		fprintf(stderr, "invalid address decode.");
		break;
	default:
		fprintf(stderr, "(%s)", r16name((op & 0x07) << 1));
		break;
	}
	fputc('\n', stderr);
}

static const char *op0name[] = {
	"HLT", "NOP", "SF", "RF", "EI", "DI", "SL", "RL",
	"CL", "RSR", "RI", "RIM", "ELO", "PCX", "DLY", "RSYS"
};

static const char *braname[] = {
	"BL", "BNL", "BF", "BNF", "BZ", "BNZ", "BM", "BP",
	"BGZ", "BLE", "BS1", "BS2", "BS3", "BS4", "BTM", "BEP"
};

static const char *alu1name[] = {
	"INR", "DCR", "CLR", "IVR", "SRR", "SLR", "RRR", "RLR"
};

static const char *alu2name[] = {
	"ADD", "SUB", "AND", "ORI", "ORE", "XFR"
};

static const char *ldst[] = {
	"LDAB ", "LDA ",
	"STAB ", "STA ",
	"LDBB ", "LDB ",
	"STBB ", "STB "
};

static void stack_op(const char *op, unsigned rpc)
{
        uint8_t byte2 = mem_read8_debug(rpc);
        uint8_t r = byte2 >> 4;
        uint8_t end = r + (byte2 & 0x0F) + 1;
        const char* s = "";

        fprintf(stderr, "%s {", op);

        if (r & 1) {
                fputs(r8map[r], stderr);
                s = ",";
        }
        while (r + 1 < end) {
                fprintf(stderr, "%s%s", s, r16map[r]);
                s = ",";
                r += 2;
        }
        if (r < end) {
                fprintf(stderr, "%s%s", s, r8map[r]);
        }
        fputs("}\n", stderr);
}

void disassemble(unsigned op)
{
	unsigned rpc = ee200_pc() + 1;
	if (op < 0x10) {
		fprintf(stderr, "%s\n", op0name[op]);
		return;
	}
	if (op < 0x20) {
		fprintf(stderr, "%s %d\n", braname[op & 0x0F],
			mem_read8_debug(rpc));
		return;
	}
	if (op < 0x28) {
		uint8_t v = mem_read8_debug(rpc);
		fprintf(stderr, "%sB %s", alu1name[op & 7],
			r8name(v >> 4));
		if (v & 0x0F)
			fprintf(stderr, ", %d", v & 0x0F);
		fprintf(stderr, "\n");
		return;
	}
	if (op < 0x2E) {
		fprintf(stderr, "%s AL\n", alu1name[op & 7]);
		return;
	}
	if (op < 0x38) {
		uint8_t v = mem_read8_debug(rpc);
		fprintf(stderr, "%s %s", alu1name[op & 7],
			r16name(v >> 4));
		if (v & 0x0F)
			fprintf(stderr, ", %d", v & 0x0F);
		fprintf(stderr, "\n");
		return;
	}
	if (op < 0x3E) {
		fprintf(stderr, "%s A\n", alu1name[op & 7]);
		return;
	}
	if (op == 0x3E) {
		fputs("INX\n", stderr);
		return;
	}
	if (op == 0x3F) {
		fputs("DCX\n", stderr);
		return;
	}
	if (op < 0x46) {
		uint8_t v = mem_read8_debug(rpc);
		fprintf(stderr, "%sB %s, %s\n", alu2name[op & 7],
			r8name(v >> 4), r8name(v));
		return;
	}
	if (op < 0x4E) {
		fprintf(stderr, "%sB AL,BL\n", alu2name[op & 7]);
		return;
	}
	if (op < 0x56) {
		uint8_t v = mem_read8_debug(rpc);
		uint8_t f = v & 0x11;
		v &= 0xEE;
		switch(f) {
		case 0x00:
			fprintf(stderr, "%s %s, %s\n", alu2name[op & 7],
				r16name(v >> 4), r16name(v));
			break;
		case 0x01:
			fprintf(stderr, "%s %s, (%X)\n", alu2name[op & 7],
				r16name(v >> 4), get16d(rpc + 1));
			break;
		case 0x10:
			fprintf(stderr, "%s %s, %X\n", alu2name[op & 7],
				r16name(v >> 4), get16d(rpc + 1));
			break;
		case 0x11:
			fprintf(stderr, "%s (%X), %s\n", alu2name[op & 7],
				get16d(rpc + 1), r16name(v));
			break;
		}
		return;
	}
	if (op < 0x5B) {
		fprintf(stderr, "%s A,B\n", alu2name[op & 7]);
		return;
	}
	if (op < 0x60) {
		fprintf(stderr, "XA%c\n", "XYBZS"[op - 0x5B]);
		return;
	}
	if (op == 0x66) {
		fprintf(stderr, "JSYS %02X\n", get8d(rpc));
		return;
	}
	if (op < 0x70) {
		/* X ops */
		if (op & 0x08)
			fputs("STX ", stderr);
		else
			fputs("LDX ", stderr);
		disaddr(rpc, 2, op & 7, 1);
		return;
	}
        if (op == 0x7e) {
                stack_op("PUSH", rpc);
                return;
        }
        if (op == 0x7f) {
                stack_op("POP", rpc);
                return;
        }
	if (op < 0x80) {
		if (op == 0x76) {
			fputs("SYSCALL?\n", stderr);
			return;
		}
		if (op & 0x08)
			fputs("JSR ", stderr);
		else
			fputs("JMP ", stderr);
		disaddr(rpc, 2, op & 7, 0);
		return;
	}
	fputs(ldst[(op & 0x7F) >> 4], stderr);
	disaddr(rpc, (op & 0x10) ? 2 : 1, op & 15, 0);
}
