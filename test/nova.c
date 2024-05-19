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
static int op_disp;
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
        addr = ram[addr];
    }
    return addr;
}

static uint16_t read_mem(uint16_t addr)
{
    return ram[addr & 0x7FFF];
}

static void write_mem(uint16_t addr, int16_t v)
{
    ram[addr & 0x7FFF] = v;
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
    op_indirect = !!(opcode >> 10);
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

    /* Extended instructions are nailed to "device" 1 */
    /* We don't strictly check every bit here but we are just a test
       tool and what a real Nova does with all the other invaliud bit patterns
       is way more complicated */
    if (dev == 1) {
        switch((opcode >> 6) & 0x0F) {
        case 0:
            fp = reg[ac];		/* MTFP */
            return;
        case 1:
            reg[ac] = fp;		/* MFFP */
            return;
        case 2:				/* Nova 4 LDB */
        case 3:
            break;
        case 4:
            sp = reg[ac];		/* MTSP */
            return;
        case 5:
            reg[ac] = sp;		/* MFSP */
            return;
        case 6:
            write_mem(sp++, reg[ac]);
            return;
        case 7:
            reg[ac] = read_mem(--sp);
            return;
        case 8:				/* Nova 4 STB */
        case 9:
            break;
        case 10:			/* SAV */
            write_mem(sp++, reg[0]);
            write_mem(sp++, reg[1]);
            write_mem(sp++, reg[2]);
            write_mem(sp++, fp);
            write_mem(sp++, (reg[3] & 0x7FFF) | (flag_c ? 0x8000 : 0x0000));
            return;
        case 11:			/* RET */
            reg_pc = read_mem(--sp);
            flag_c = reg_pc >> 15;
            reg_pc &= 0x7FFF;
            reg[3] = read_mem(--sp);
            fp = reg[3] & 0x7FFF;
            reg[2] = read_mem(--sp);
            reg[1] = read_mem(--sp);
            reg[0] = read_mem(--sp);
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
    if (flag_c)
        iv |= (1 << 16);

    switch((opcode >> 8) & 7) {
    case 0:	/* COM */
        iv ^= 0xFFFF;
        break;
    case 1:	/* NEG */
        iv ^= 0xFFFF;
        iv++;
        break;
    case 2:	/* MOV */
        iv &= 0xFFFF;	/* So we don't cause a carry */
        break;
    case 3:	/* INC */
        iv++;
        break;
    case 4: 	/* ADC (wrongly listed as 101 in some Nova docs) */
        iv = reg[acd] + ~iv;
        break;
    case 5:	/* SUB */
        iv = reg[acd] - iv;
        break;
    case 6:	/* ADD */
        iv += reg[acd];
        break;
    case 7:	/* AND - doesn't touch carry so mask carry */
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

void nova_execute(void)
{
    opcode = ram[reg_pc];
    if (opcode & 0x8000)
        twoac_mo();
    else
        oneac_ea();
}

