 /*
 *	Nova instruction sim for compiler testing
 *
 *	This is not a full emulation. No device I/O emulation, interrupts,
 *	etc etc. Just the core user bits.
 *
 *	At this point we behave like a micronova, with no autoinc/dec locations
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

static uint16_t ram[32768];	/* 32KWord address space */
static unsigned flag_c;
static uint16_t reg_pc;
static uint16_t reg[4];
static uint16_t fp;
static uint16_t sp;
static uint16_t opcode;
static unsigned op_ac;
static unsigned op_code;
static unsigned op_index;
static unsigned op_indirect;
static int8_t op_disp;
static uint16_t ea;

/* TODO: autoinc and autodec on non micronova */
static uint16_t indirect(uint16_t baddr)
{
    unsigned n = 0;
    uint16_t addr = baddr;
    while (addr & 0x8000) {
        if (n++ == 64) {
            fprintf(stderr, "***indirection loop at %x\n", baddr);
            exit(1);
        }
        addr = ntohs(ram[addr & 0x7FFF]);
    }
    return addr;
}

static uint16_t read_mem(uint16_t addr)
{
    return ntohs(ram[addr & 0x7FFF]);
}

static void write_mem(uint16_t addr, int16_t v)
{
    if ((addr & 0x7FFF) < 86) {
        fprintf(stderr, "***WFAULT at %x\n", reg_pc);
        exit(1);
    }
    ram[addr & 0x7FFF] = htons(v);
}
    
static void modify_ea(int n)
{
    uint16_t v = read_mem(ea);
    v += n;
    write_mem(ea, v);
}

/* Decode base noac/oneac operations */
void opdecode(void)
{
    op_ac = (opcode >> 11) & 3;
    op_code = (opcode >> 13) & 3;
    op_index = (opcode >> 8) & 3;
    op_indirect = (opcode >> 10) & 1;
    op_disp = opcode & 0xFF;

    switch(op_index) {
    case 0:	/* Zero based 8bit add */
        ea = (uint8_t)op_disp;
        break;
    case 1:	/* PC rel signed */
        ea = reg_pc + op_disp;
        break;
    default:
        ea = reg[op_index] + op_disp;
        break;
    }
    if (op_indirect)
        ea = indirect(read_mem(ea));
}

/* 011 AC Op.3 F.2 Dev.6 */
void devio_op(void)
{
    unsigned ac = (opcode >> 11) & 3;
    unsigned dev = opcode & 0x3F;
    unsigned f = (opcode >> 6) & 3;

    /* Dev 0 access we use as our debug exit trap */
    if (dev == 0) {
        if (reg[1])
            printf("*** %u\n", reg[1]);
        exit(0);
    }
    /* Extended instructions are nailed to "device" 1 */
    /* We don't strictly check every bit here but we are just a test
       tool and what a real Nova does with all the other invaliud bit patterns
       is way more complicated */
    if (dev == 1) {
        switch((opcode >> 7) & 0x0F) {
        case 0:
            fp = reg[ac];		/* MTFP */
            return;
        case 1:
            reg[ac] = fp & 0x7FFF;	/* MFFP */
            return;
        case 2:				/* Nova 4 LDB */
        case 3:
            break;
        case 4:
            sp = reg[ac];		/* MTSP */
            return;
        case 5:
            reg[ac] = sp & 0x7FFF;	/* MFSP */
            return;
        case 6:
            write_mem(++sp, reg[ac]);
            return;
        case 7:
            reg[ac] = read_mem(sp--);
            return;
        case 8:				/* Nova 4 STB */
        case 9:
            break;
        case 10:			/* SAV */
            write_mem(++sp, reg[0]);
            write_mem(++sp, reg[1]);
            write_mem(++sp, reg[2]);
            write_mem(++sp, fp & 0x7FFF);
            write_mem(++sp, (reg[3] & 0x7FFF) | (flag_c ? 0x8000 : 0x0000));
            fp = sp;
            reg[3] = fp & 0x7FFF;
            return;
        case 11:			/* RET */
            reg_pc = read_mem(sp--);
            flag_c = reg_pc >> 15;
            reg_pc &= 0x7FFF;
            reg[3] = read_mem(sp--);
            fp = reg[3] & 0x7FFF;
            reg[2] = read_mem(sp--);
            reg[1] = read_mem(sp--);
            reg[0] = read_mem(sp--);
            reg_pc--;			/* Correct for caller changing it */
            return;
        case 12:
        case 13:			/* DIV option */
        case 14:
        case 15:			/* MUL option */
            break;
        }
    }
    fprintf(stderr, "Unknown I/O op %o (d %o f %o ac %o)\n", opcode, dev, f, ac);
    exit(1);
}

void noac_ea(void)
{
    /* the ac bits actually form the instruction */
    switch(op_ac) {
    case 0:	/* JMP */
        reg_pc = ea;
        return;
    case 1:	/* JSR */
        reg[3] = reg_pc + 1;
        reg_pc = ea;
        return;
    case 2:	/* ISZ */
        modify_ea(1);
        reg_pc++;
        return;
    case 3:	/* DSZ */
        modify_ea(-1);
        reg_pc++;
        return;
    }
}

void oneac_ea(void)
{
    opdecode();
    switch(op_code) {
    case 0:	/* Means it's a noac_ea */
        noac_ea();
        return;
    case 1:	/* LDA */
        reg[op_ac] = read_mem(ea);
        reg_pc++;
        return;
    case 2:	/* STA */
        write_mem(ea, reg[op_ac]);
        reg_pc++;
        return;
    case 3:	/* Other ops. We emulate a few special ones but not all
                   the general device I/O */
        devio_op();
        reg_pc++;
        return;
    }
}

void twoac_mo(void)
{
    unsigned acs = (opcode >> 13) & 3;
    unsigned acd = (opcode >> 11) & 3;
    unsigned sh = (opcode >> 6) & 3;
    unsigned c = (opcode >> 4) & 3;
    unsigned nostore = (opcode >> 3) & 1;
    unsigned skip = (opcode & 7);

    uint32_t iv;

    switch(c) {		/* Adjust carry as per instruction info */
    case 0:		/* No change */
        break;
    case 1:
        flag_c = 0;	/* Clear */
        break;
    case 2:
        flag_c = 1;	/* Set */
        break;
    case 3:
        flag_c = 1 -flag_c;	/* Complement */
        break;
    }

    iv = reg[acs];		/* Merge together carry and working reg */

    switch((opcode >> 8) & 7) {
    case 0:	/* COM */
        iv ^= 0xFFFF;
        break;
    case 1:	/* NEG */
        iv ^= 0xFFFF;
        iv++;
        break;
    case 2:	/* MOV */
        break;
    case 3:	/* INC */
        iv++;	/* FFFF causes a carry complement */
        break;
    case 4: 	/* ADC (wrongly listed as 101 in some Nova docs) */
        iv = reg[acd] + (iv ^ 0xFFFF);
        break;
    case 5:	/* SUB */
        iv = reg[acd] - iv;
        /* This is a borrow */
        iv ^= 0x10000;
        break;
    case 6:	/* ADD */
        iv += reg[acd];
        break;
    case 7:	/* AND - doesn't touch carry */
        iv &= reg[acd];
        break;
    }
    /* If the carry out is high complement the carry flag */
    if (iv & 0x10000)
        flag_c ^= 1;
    /* Merge then back together for shifting */
    iv = (iv & 0xFFFF) | (flag_c << 16);

    switch(sh) {
    case 0:
        break;
    case 1:
        /* Left rotate through carry */
        iv <<= 1;
        if (iv & 0x20000)	/* Old carry */
            iv |= 1;
        iv &= 0x1FFFF;
        break;
    case 2:
        /* Right rotate through carry */
        if (iv & 1)
            iv |= 0x20000;
        iv >>= 1;
        break;
    case 3:
        /* Keep carry swap bytes */
        iv = (iv & 0x10000) | ((iv & 0xFF) << 8) | ((iv >> 8) & 0xFF);
    }
    /* Extract final values */
    flag_c = !!(iv & 0x10000);
    iv &= 0xFFFF;
    if (nostore == 0)
        reg[acd] = iv;
    reg_pc++;
    /* Now do skip */
    switch(skip) {
        case 0:	/* never skip */
            break;
        case 1:	/* skp */
            reg_pc++;
            break;
        case 2:	/* szc */
            if (flag_c == 0)
                reg_pc++;
            break;
        case 3: /* snc */
            if (flag_c)
                reg_pc++;
            break;
        case 4: /* szr */
            if (iv == 0)
                reg_pc++;
            break;
        case 5:	/* snr */
            if (iv)
                reg_pc++;
            break;
        case 6:	/* sez */
            if (flag_c == 0 || iv == 0)
                reg_pc++;
            break;
        case 7: /* sbn */
            if (flag_c && iv)
                reg_pc++;
            break;
    }
}

static const char *oph[] = { "COM", "NEG", "MOV", "INC", "ADC", "SUB", "ADD", "AND"};
static const char *skipstr[] = { "    ", ",SKP", ",SZC", ",SNC", ",SZR", ",SNR", ",SEZ", ",SBN" };
static const char *opna[] = { "JMP", "JSR", "ISZ", "DSZ" };
static const char *opd[] = { "NIO", "DIA", "DOA", "DIB", "DOB", "DIC", "DOC", "SKP" };
static const char *opsf[] = { "BN", "BZ", "DN", "DZ" };

static void novaio_dis(uint16_t op)
{
    unsigned dop = (op >> 8) & 7;
    /* Special rules for Nova 3/4 ops */
    if ((op & 0x3F) == 1) {
        unsigned ac = (op >> 11) & 3;
        switch((op >> 7) & 0x0F) {
        case 0:
            fprintf(stderr, "MTFP %u\n", ac);
            return;
        case 1:
            fprintf(stderr, "MFFP %u\n", ac);
            return;
        case 4:
            fprintf(stderr, "MTSP %u\n", ac);
            return;
        case 5:
            fprintf(stderr, "MFSP %u\n", ac);
            return;
        case 6:
            fprintf(stderr, "PSHA %u\n", ac);
            return;
        case 7:
            fprintf(stderr, "POPA %u\n", ac);
            return;
        case 10:
            fprintf(stderr, "SAV\n");
            return;
        case 11:
            fprintf(stderr, "RET\n");
            return;
        }
    }
    if (op == 0) {
        fprintf(stderr, "NIO%c %u\n",
            " SCP"[(op >> 6) & 3],
            op & 0x3F);
        return;
    }
    if (op == 7) {
        fprintf(stderr, "SKP%s %u\n",
            opsf[(op >> 6) & 3],
            op & 0x3F);
        return;
    }
    fprintf(stderr, "%s%c %u,%u",
        opd[dop],
        " SCP"[(op >> 6) & 3],
        (op >> 11) & 3,
        op & 0x3F);
}

void nova_dis(uint16_t op)
{
    unsigned ac;
    if (op & 0x8000) {
        fprintf(stderr, "%s%c%c%c", oph[(op  >> 8) & 7],
                        " ZOC"[(op >> 4) & 3],
                        " LRS"[(op >> 6) & 3],
                        " #"[(op >> 3) & 1]);
        fprintf(stderr, " %u,%u",
            (op >> 13) & 3, (op >> 11) & 3);
        fprintf(stderr, "%s\n", skipstr[op & 7]);
        return;
    }
    ac = (opcode >> 11) & 3;
    switch((opcode >> 13) & 3) {
    case 0:
        /* No AC */
        fprintf(stderr, "%s", opna[ac]);
        break;
    case 1:
        fprintf(stderr, "LDA %u,", ac);
        break;
    case 2:
        fprintf(stderr, "STA %u,", ac);
        break;
    case 3:
        /* Device */
        novaio_dis(op);
        return;
    }
    fprintf(stderr, " %c", " @"[(opcode >> 10) & 1]);
    fprintf(stderr, " %u,%u\n", opcode & 0xFF, (opcode >> 8) & 3);
}
    
void nova_execute_one(unsigned debug)
{
    opcode = read_mem(reg_pc);
    if (debug) {
        fprintf(stderr, "%04X | %c %04X %04X %04X %04X | %04X %04X | %04X = ",
            reg_pc, " L"[flag_c], reg[0], reg[1], reg[2], reg[3], fp, sp, opcode);
        nova_dis(opcode);
    }            
    if (opcode & 0x8000)
        twoac_mo();
    else
        oneac_ea();
}

int main(int argc, char *argv[])
{
	int fd;
	unsigned debug = 0;

	if (argc == 4 && strcmp(argv[1], "-d") == 0) {
		argv++;
		argc--;
		debug = 1;
	}
	if (argc != 3) {
		fprintf(stderr, "nova: test map.\n");
		exit(1);
	}
	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror(argv[1]);
		exit(1);
	}
	if (read(fd, ram, sizeof(ram)) < 0x210) {
		fprintf(stderr, "nova: bad test.\n");
		perror(argv[1]);
		exit(1);
	}
	close(fd);

	reg_pc = 0x0100;

	while (1)
		nova_execute_one(debug);
	return 0;
}
