#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tms7k.h"

static FILE *tms_log;

static uint8_t st;
static uint8_t sp;
static uint16_t pc;
static uint16_t pc0;
static uint8_t reg[256];
static unsigned clocks;
static unsigned mode;

#define ST_C	0x80
#define ST_N	0x40
#define ST_Z	0x20
#define ST_I	0x10

#define MODE_RUN	0
#define MODE_IDLE	1


static const char *itab[256] = {
/* 0x00 */
    "NOP",
    "IDLE",
    NULL,
    NULL,
    NULL,
    "EINT",
    "DINT",
    "SETC",
    "POP ST",
    "STSP",
    "RETS",
    "RETI",
    NULL,
    "LDSP",
    "PUSH ST",
    NULL,
/* 0x10 */
    NULL,
    NULL,
    "MOV R#,A",
    "AND R#,A",
    "OR R#,A",
    "XOR R#,A",
    "BTJO R#,A,#",
    "BTJZ R#,A,#",
    "ADD R#,A",
    "ADC R#,A",
    "SUB R#,A",
    "SBB R#,A",
    "MPY R#,A",
    "CMP R#,A",
    "DAC R#,A",
    "DSB R#,A",
/* 0x20 */
    NULL,
    NULL,
    "MOV %#,A",
    "AND %#,A",
    "OR %#,A",
    "XOR %#,A",
    "BTJO %#,A,#",
    "BTJZ %#,A,#",
    "ADD %#,A",
    "ADC %#,A",
    "SUB %#,A",
    "SBB %#,A",
    "MPY %#,A",
    "CMP %#,A",
    "DAC %#,A",
    "DSB %#,A",
/* 0x30 */
    NULL,
    NULL,
    "MOV R#,B",
    "AND R#,B",
    "OR R#,B",
    "XOR R#,B",
    "BTJO R#,B,#",
    "BTJZ R#,B,#",
    "ADD R#,B",
    "ADC R#,B",
    "SUB R#,B",
    "SBB R#,B",
    "MPY R#,B",
    "CMP R#,B",
    "DAC R#,B",
    "DSB R#,B",
/* 0x40 */
    NULL,
    NULL,
    "MOV R#,R#",
    "AND R#,R#",
    "OR R#,R#",
    "XOR R#,R#",
    "BTJO R#,R#,#",
    "BTJZ R#,R#,#",
    "ADD R#,R#",
    "ADC R#,R#",
    "SUB R#,R#",
    "SBB R#,R#",
    "MPY R#,R#",
    "CMP R#,R#",
    "DAC R#,R#",
    "DSB R#,R#",
/* 0x50 */
    NULL,
    NULL,
    "MOV %#,B",
    "AND %#,B",
    "OR %#,B",
    "XOR %#,B",
    "BTJO %#,B,#",
    "BTJZ %#,B,#",
    "ADD %#,B",
    "ADC %#,B",
    "SUB %#,B",
    "SBB %#,B",
    "MPY %#,B",
    "CMP %#,B",
    "DAC %#,B",
    "DSB %#,B",
/* 0x60 */
    NULL,
    NULL,
    "MOV B,A",
    "AND B,A",
    "OR B,A",
    "XOR B,A",
    "BTJO B,A,#",
    "BTJZ B,A,#",
    "ADD B,A",
    "ADC B,A",
    "SUB B,A",
    "SBB B,A",
    "MPY B,A",
    "CMP B,A",
    "DAC B,A",
    "DSB B,A",
/* 0x70 */
    NULL,
    NULL,
    "MOV %#,R#",
    "AND %#,R#",
    "OR %#,R#",
    "XOR %#,R#",
    "BTJO %#,R#,#",
    "BTJZ %#,R#,#",
    "ADD %#,R#",
    "ADC %#,R#",
    "SUB %#,R#",
    "SBB %#,R#",
    "MPY %#,R#",
    "CMP %#,R#",
    "DAC %#,R#",
    "DSB %#,R#",
/* 0x80 */
    "MOVP P#,A",
    NULL,
    "MOVP A,P#",
    "ANDP A,P#",
    "ORP A,P#",
    "XORP A,P#",
    "BTJOP A,P#",
    "BTJZP A,P#",
    "MOVD ##,R#",
    NULL,
    "LDA @##",
    "STA @##",
    "BR @##",
    "CMPA @##",
    "CALL @##",
    NULL,
/* 0x90 */
    NULL,
    "MOVP P#,B",
    "MOVP B,P#",
    "ANDP B,P#",
    "ORP B,P#",
    "XORP B,P#",
    "BTJOP B,P#",
    "BTJZP B,P#",
    "MOVD R#,R#",
    NULL,
    "LDA *R#",
    "STA *R#",
    "BR *R#",
    "CMPA *R#",
    "CALL *R#",
    NULL,
/* 0xA0 */
    NULL,
    NULL,
    "MOVP %#,P#",
    "ANDP %#,P#",
    "ORP %#,P#",
    "XORP %#,P#",
    "BTJOP %#,P#",
    "BTJZP %#,P#",
    "MOVD %##(B),R#",
    NULL,
    "LDA %##(B)",
    "STA %##(B)",
    "BR %##(B)",
    "CMPA %##(B)",
    "CALL %##(B)",
    NULL,
/* 0xB0 */
    "TSTA",
    NULL,
    "DEC A",
    "INC A",
    "INV A",
    "CLR A",
    "XCHB A",
    "SWAP A",
    "PUSH A",
    "POP A",
    "DJNZ A,#",
    "DECD A",
    "RR A",
    "RRC A",
    "RL A",
    "RLC A",
/* 0xC0 */
    "MOV A,B",
    "TSTB",
    "DEC B",
    "INC B",
    "INV B",
    "CLR B",
    "XCHB B",
    "SWAP B",
    "PUSH B",
    "POP B",
    "DJNZ B,#",
    "DECD B",
    "RR B",
    "RRC B",
    "RL B",
    "RLC B",
/* 0xD0 */
    "MOV A,R#",
    "MOV B,R#",
    "DEC R#",
    "INC R#",
    "INV R#",
    "CLR R#",
    "XCHB R#",
    "SWAP R#",
    "PUSH R#",
    "POP R#",
    "DJNZ R#,#",
    "DECD R#",
    "RR R#",
    "RRC R#",
    "RL R#",
    "RLC R#",
/* 0xE0 */
    "JMP #",
    "JN #",
    "JZ #",
    "JC #",
    "JP #",
    "JPZ #",
    "JNZ #",
    "JNC #",
    "TRAP 23",
    "TRAP 22",
    "TRAP 21",
    "TRAP 20",
    "TRAP 19",
    "TRAP 18",
    "TRAP 17",
    "TRAP 16",
/* 0xF0 */
    "TRAP 15",
    "TRAP 14",
    "TRAP 13",
    "TRAP 12",
    "TRAP 11",
    "TRAP 10",
    "TRAP 9",
    "TRAP 8",
    "TRAP 7",
    "TRAP 6",
    "TRAP 5",
    "TRAP 4",
    "TRAP 3",
    "TRAP 2",
    "TRAP 1",
    "TRAP 0"
};

static const char *tms7k_status(void)
{
    static char buf[5];
    strcpy(buf, "----");
    if (st & ST_C)
        *buf = 'C';
    if (st & ST_N)
        buf[1] = 'N';
    if (st & ST_Z)
        buf[2] = 'Z';
    if (st & ST_I)
        buf[3] = 'I';
    return buf;
}

static const char *tms7k_decode(uint16_t addr)
{
    static char buf[256];
    uint8_t op = mem_read8_debug(addr);
    const char *p = itab[op];
    char *bp;
    unsigned i;

    bp = buf + sprintf(buf, "%04X: | ", addr++);
    for (i = 0; i < 16; i++)
        bp += sprintf(bp, "%02X ",reg[i]);
    bp += sprintf(bp, " | %s %02X | ", tms7k_status(), sp);
    if (p == NULL) {
        sprintf(bp, "Invalid (%02X)", op);
        return buf;
    }
    while(*p) {
        if (*p == '#') {
            bp += sprintf(bp, "%02X", mem_read8_debug(addr++));
            p++;
        } else
            *bp++ = *p++;
    }
    *bp = 0;
    return buf;
}


static void invalid(unsigned op)
{
    fprintf(stderr, "tms7k: halt at %04X - invalid instruction %02X\n", pc0, op);
    exit(1);
}

/* Ports; depends on the device but we need to know about a
   few */

static uint8_t iocnt0;
static uint8_t iocnt1;
static uint8_t iocnt2;
static uint8_t bport;
static uint8_t addr;

/*
 * I/O: very crude approximation for the moment. Unused ports map through
 * to the space behind. For now emulate a 256 byte reg space device with
 * 70Cx2 I/O
 */

static uint8_t dev_read8(uint16_t addr)
{
    if (addr < 256)
        return reg[addr];
    switch(addr) {
        case 0x0100:
            return iocnt0;
        case 0x0101:
            return iocnt2;
        case 0x0102:
            return iocnt1;
        case 0x0103:
            return 0xFF;
        case 0x0104:	/* APORT */
        case 0x0105:	/* ADDR */
        case 0x0106:	/* BPORT */
        case 0x0107:	/* Reserved */
            return 0xFF;
        case 0x010C:	/* T1MSDATA */
        case 0x010D:	/* T1LSDATA */
        case 0x010E:	/* T1CTL1 */
        case 0x010F:	/* T1CTL0 */
        case 0x0110:	/* T2MSDATA */
        case 0x0111:	/* T2LSDATA */
        case 0x0112:	/* T2CTL1 */
        case 0x0113:	/* T2CTL0 */
        case 0x0114:	/* SMODE */
        case 0x0115:	/* SCTL0 */
        case 0x0116:	/* SSTAT */
        case 0x0117:	/* T3DATA */
        case 0x0118:	/* SCTL1 */
        case 0x0119:	/* RXBUF */
        case 0x011A:	/* TXBUF */
        default:
            return mem_read8(addr);
    }
}

static void dev_write8(uint16_t addr, uint8_t val)
{
    if (addr < 256)
        reg[addr] = val;
    else switch(addr) {
        case 0x0100:	/* IOCNT0 */
        case 0x0101:	/* IOCNT2 */
        case 0x0102:	/* IOCNT1 */
        case 0x0103:	/* Reserved */
            break;
        case 0x0104:	/* APORT */
        case 0x0105:	/* ADDR */
        case 0x0106:	/* BPORT */
        case 0x0107:	/* Reserved */
            break;
        case 0x010C:	/* T1MSDATA */
        case 0x010D:	/* T1LSDATA */
        case 0x010E:	/* T1CTL1 */
        case 0x010F:	/* T1CTL0 */
        case 0x0110:	/* T2MSDATA */
        case 0x0111:	/* T2LSDATA */
        case 0x0112:	/* T2CTL1 */
        case 0x0113:	/* T2CTL0 */
        case 0x0114:	/* SMODE */
        case 0x0115:	/* SCTL0 */
        case 0x0116:	/* SSTAT */
        case 0x0117:	/* T3DATA */
        case 0x0118:	/* SCTL1 */
        case 0x0119:	/* RXBUF */
        case 0x011A:	/* TXBUF */
        default:
            mem_write8(addr, val);
    }
}

static unsigned get_byte(void)
{
    return dev_read8(pc++);
}

static unsigned get_reg(void)
{
    return reg[dev_read8(pc++)];
}

static uint8_t *get_regp(void)
{
    return reg + dev_read8(pc++);
}

static unsigned get_port(void)
{
    return dev_read8(pc++) + 0x100;
}

static int8_t get_offset(void)
{
    return dev_read8(pc++);
}

static unsigned get_addr(void)
{
    unsigned addr = dev_read8(pc++) << 8;
    addr |= dev_read8(pc++);
    return addr;
}

static unsigned get_regpair(void)
{
    unsigned rl = dev_read8(pc++);
    if (rl == 0)
        invalid(0);
    return (reg[rl - 1] << 8) | reg[rl];
}

static unsigned carry(void)
{
    if (st & ST_C)
        return 1;
    return 0;
}

/*
 *	Set up the flags from the result. The TMS7000 flags
 *	are fairly basic so this is quite easy
 */
static void setflags(unsigned res)
{
    st &= ~(ST_C|ST_Z|ST_N);
    if (res & 0x100)
        st |= ST_C;
    if ((res & 0xFF) == 0)
        st |= ST_Z;
    if (res & 0x80)
        st |= ST_N;
}

/*
 *	op src,dst. Also variants that then take an
 *	offset for a branch.
 */
static void twoop(unsigned op, uint8_t src, uint8_t *dst)
{
    unsigned res;
    switch(op & 0x0F) {
    case 0x00:	/* Invalid */
    case 0x01:	/* Invalid */
        invalid(op);
        break;
    case 0x02:	/* MOV */
        res = src;
        *dst = res;
        break;
    case 0x03:	/* AND */
        res = src & *dst;
        *dst = res;
        break;
    case 0x04:	/* OR */
        res = src | *dst;
        *dst = res;
        break;
    case 0x05:	/* XOR */
        res = src ^ *dst;
        *dst = res;
        break;
    case 0x06:	/* BTJO off */
        res = src & *dst;
        clocks += 2;
        if (res) {
            clocks += 2;
            pc += get_offset();
        } else
            get_offset();
        break;
    case 0x07:	/* BTJZ off */
        res = src & *dst;
        clocks += 2;
        if (res == 0) {
            clocks += 2;
            pc += get_offset();
        } else
            get_offset();
        break;
    case 0x08:	/* ADD */
        res = src + *dst;
        *dst = res;
        break;
    case 0x09:	/* ADC */
        res = src + *dst + carry();
        *dst = res;
        break;
    case 0x0A:	/* SUB */
        res = *dst - src;
        res ^= 0x0100;	/* Borrow not carry */
        *dst = res;
        break;
    case 0x0B:	/* SBB */
        res = *dst - src - 1 + carry();
        res ^= 0x0100;	/* Borrow not carry */
        *dst = res;
        break;
    case 0x0C:	/* MPY */
        res = src * *dst;
        reg[0] = res >> 8;
        reg[1] = res;
        clocks += 39;
        return;
    case 0x0D:	/* CMP */
        res = *dst - src;
        /* N/Z from the result, C is set if d is >= s (unsigned) */
        res ^= 0x0100;
        setflags(res);
        return;
    case 0x0E:	/* DAC */
        /* TODO: review decimal ops */
        res = *dst + src + carry();
        if ((res & 0x0F) > 0x09)
            res += 0x06;	/* Carry the BCD digit */
        if ((res & 0xF0) > 0x90)
            res += 0x60;	/* Carry the upper BCD digit */
        *dst = res;
        /* TODO: add carry */
        break;
    case 0x0F:	/* DSB */
        res = *dst + src + 1 - carry();
        if ((res & 0x0F) > 0x09)
            res -= 0x16;
        if ((res & 0xF0) > 0x90)
            res -= 0x160;
        break;
    }
    *dst = res;
    setflags(res);
}

static unsigned opx8(unsigned op, uint16_t addr)
{
    unsigned res;
    switch(op & 0x07) {
    case 0x00:	/* MOVD handled elsewhere */
    case 0x01:	/* Invalid */
        invalid(op);
        return 0;
    case 0x02:	/* LDA */
        reg[0] = dev_read8(addr);
        setflags(reg[0]);
        return 10;
    case 0x03:	/* STA */
        dev_write8(addr, reg[0]);
        setflags(reg[0]);
        return 10;
    case 0x04:	/* BR */
        pc = addr;
        return 9;
    case 0x05:	/* CMPA */
        res = reg[0] - dev_read8(addr);
        res ^= 0x0100;	/* Borrow */
        setflags(res);
        return 11;
    case 0x06:	/* CALL */
        reg[++sp] = pc >> 8;
        reg[++sp] = pc;
        pc = addr;
        return 13;
    default:	/* Invalid */
        invalid(op);
        return 0;
    }
}

static void opx0(unsigned op, unsigned data, unsigned port)
{
    unsigned res;

    switch(op & 0x07) {
    case 0x00:	/* Invalid */
    case 0x01:	/* Invalid */
        invalid(op);
        break;
    case 0x02:	/* MOVP */
        dev_read8(port);	/* A move to a port causes a read first */
        res = data;
        break;
    case 0x03:	/* ANDP */
        res = dev_read8(port) & data;
        break;
    case 0x04:	/* ORP */
        res = dev_read8(port) | data;
        break;
    case 0x05:	/* XORP */
        res = dev_read8(port) ^ data;
        break;
    case 0x06:	/* BJTOP off */
        res = dev_read8(port) & data;
        clocks++;
        if (res) {
            pc += get_offset();
            clocks += 2;
        } else
            get_offset();
        setflags(res);
        return;
    case 0x07:	/* BJTZP off */
        clocks++;
        res = dev_read8(port) & data;
        if (res == 0) {
            pc += get_offset();
            clocks += 2;
        } else
            get_offset();
        setflags(res);
        return;
    }
    setflags(res);
    dev_write8(port, res);
}

static void single_op(unsigned op, uint8_t r)
{
    unsigned res;
    switch(op & 0x0F) {
    case 0x00: 	/* Invalid */
    case 0x01:	/* Invalid */
        invalid(op);
        return;
    case 0x02:	/* DEC */
        res = reg[r] - 1;
        break;
    case 0x03:	/* INC */
        res = reg[r] + 1;
        break;
    case 0x04:	/* INV */
        res = reg[r] ^ 0xFF;
        break;
    case 0x05:	/* CLR */
        res = 0;
        break;
    case 0x06:	/* XCHB */
        /* TODO : which of the two gets the flag values ? */
        res = reg[1];
        reg[1] = reg[r];
        reg[r] = res;
        break;
    case 0x07:	/* SWAP */
        res = (reg[r] << 4) & 0x0F;
        res |= (reg[r] >> 4);
        break;
    case 0x08:	/* PUSH */
        reg[sp++] = reg[r];
        res = reg[r];
        break;
    case 0x09:	/* POP */
        res = reg[--sp];
        return;
    case 0x0A:	/* DJNZ off */
        res = reg[r] - 1;
        clocks += 2;
        if (res) {
            pc += get_offset();
            clocks += 2;
        } else
            get_offset();
        break;
    case 0x0B:	/* DECD */
        if (r == 0)
            invalid(op);
        reg[r]--;
        res = reg[r - 1];
        if (reg[r] == 0xFF)
            res--;
        setflags(res);
        reg[r - 1] = res;
        clocks += 4;
        return;
    case 0x0C:	/* RR */
        res = reg[r] >> 1;
        if (reg[r] & 0x01)
            res |= 0x180;
        break;
    case 0x0D:	/* RRC */
        res = reg[r] >> 1;
        if (st & ST_C)
            res |= 0x80;
        if (reg[r] & 0x01)
            res |= 0x0100;
        break;
    case 0x0E:	/* RL */
        res = reg[r] << 1;
        if (reg[r] & 0x80)
            res |= 0x101;
        break;
    case 0x0F:	/* RLC */
        res = reg[r] << 1;
        if (st & ST_C)
            res |= 0x01;
        if (reg[r] & 0x80)
            res |= 0x100;
        break;
    }
    setflags(res);
    reg[r] = res;
}

static unsigned immed0(unsigned op)
{
    switch(op) {
    case 0x00:	/* NOP */
        return 4;
    case 0x01:	/* IDLE */
        mode = MODE_IDLE;
        return 6;
    case 0x02:	/* Illegal */
    case 0x03:	/* Illegal */
    case 0x04:	/* Illegal */
        invalid(op);
        return 0;
    case 0x05:	/* EINT */
        st |= ST_I|ST_C|ST_Z|ST_N;
        return 5;
    case 0x06:	/* DINT */
        st = 0;
        return 5;
    case 0x07:	/* SETC */
        st |= ST_C|ST_Z;
        st &= ~ST_N;
        return 5;
    case 0x08:	/* POP ST */
        st = reg[sp--];
        return 6;
    case 0x09:	/* STSP */
        sp = reg[1];
        return 6;
    case 0x0A:	/* RETS */
        pc = reg[sp--];
        pc |= reg[sp--] << 8;
        return 7;
    case 0x0B:	/* RETI */
        pc = reg[sp--];
        pc |= reg[sp--] << 8;
        st = reg[sp--];
        return 9;
    case 0x0C:	/* Illegal */
        invalid(op);
        return 0;
    case 0x0D:	/* LDSP */
        sp = reg[1];
        return 5;
    case 0x0E:	/* PUSH ST */
        reg[++sp] = st;
        return 6;
    default:	/* Illegal */
        invalid(op);
        return 0;
    }
}

static void trap(unsigned op)
{
    unsigned addr = 0xFFFE - 2 * op;
    reg[++sp] = pc >> 8;
    reg[++sp] = pc;
    pc = dev_read8(addr) << 8;
    pc |= dev_read8(addr + 1);
}

static unsigned branch(unsigned op)
{
    unsigned r = 0;
    switch(op){
    case 0xE0:	/* JMP */
        r = 1;
        break;
    case 0xE1:	/* JN */
        r = st & ST_N;
        break;
    case 0xE2:	/* JEQ / JZ */
        r = st & ST_Z;
        break;
    case 0xE3:	/* JHS / JC */
        r = st & ST_C;
        break;
    case 0xE4:	/* JP */
        if (!(st & ST_Z))
            r = !(st & ST_N);
        break;
    case 0xE5:	/* JPZ */
        r = !(st & ST_N);
        break;
    case 0xE6:	/* JNE / JNZ */
        r = !(st & ST_Z);
        break;
    case 0xE7:	/* JL / JNC */
        r = !(st & ST_C);
        break;
    }
    if (r) {
        pc += get_offset();
        return 7;
    }
    get_offset();
    return 5;
}

static unsigned execute_op(unsigned op)
{
    uint8_t r, r2;
    unsigned addr;

    if (tms_log)
        fprintf(tms_log, "%s\n", tms7k_decode(pc0));

    /* TODO: specials at C0 etc */
    /* Simple op classes */
    switch(op & 0xF8) {
    case 0x00:
    case 0x08:
        return immed0(op);
    case 0x10:
    case 0x18:
        twoop(op, get_reg(), reg);
        return 8;
    case 0x20:
    case 0x28:
        twoop(op, get_byte(), reg);
        return 7;
    case 0x30:
    case 0x38:
        twoop(op, get_reg(), reg + 1);
        return 8;
    case 0x40:
    case 0x48:
        r = get_reg();
        twoop(op, r, get_regp());
        return 10;
    case 0x50:
    case 0x58:
        twoop(op, get_byte(), reg + 1);
        return 7;
    case 0x60:
    case 0x68:
        twoop(op, reg[1], reg);
        return 5;
    case 0x70:
    case 0x78:
        r = get_byte();
        twoop(op, r, get_regp());
        return 9;
    case 0x80:
        /* 0x80/91 are special */
        if (op == 0x80)	{ /* MOVP Ps,A */
            reg[0] = dev_read8(get_port());
            setflags(reg[0]);
            return 9;
        }
        opx0(op, reg[0], get_port());
        return 10;
    case 0x88:
        if (op == 0x88) {	/* MOVD const16,rr */
            addr = get_addr();
            r = get_byte();
            if (r == 0)
                invalid(op);
            reg[r - 1] = addr >> 8;
            reg[r] = addr & 0xFF;
            setflags(reg[r]);
            return 15;
        }
        return 1 + opx8(op, get_addr());
    case 0x90:
        if (op == 0x91)	{ /* MOVP Ps,B */
            reg[1] = dev_read8(get_port());
            setflags(reg[1]);
            return 8;
        }
        opx0(op, reg[1], get_port());
        return 9;
    case 0x98:
        if (op == 0x98) {	/* MOVD r,r is a bit odd so handle here */
            r = get_byte();
            r2 = get_byte();
            if (r == 0 || r2 == 0)
                invalid(op);
            reg[r2 - 1] = reg[r - 1];
            reg[r2] = reg[r];
            setflags(reg[r2]);	/* TODO: is this the byte that's flagged ? */
            return 14;
        }
        return opx8(op, get_regpair());
    case 0xA0:
        r = get_byte();
        opx0(op, r, get_port());
        return 11;
    case 0xA8:
        if (op == 0xA8) {
            addr = get_addr() + reg[1];
            r = get_byte();
            if (r == 0)
                invalid(op);
            reg[r - 1] = addr & 0xFF;
            reg[r] = addr >> 8;
            setflags(reg[r]);
            return 17;
        }
        return 3 + opx8(op, get_addr() + reg[1]);
    case 0xB0:
        if (op == 0xB0) {	/* CLRC / TSTA */
            setflags(reg[0]);
            return 6;
        }
    case 0xB8:
        single_op(op, 0);
        return 5;
    case 0xC0:
        if (op == 0xC0) {	/* MOV A,B */
            reg[1] = reg[0];
            setflags(reg[1]);
            return 6;
        }
        if (op == 0xC1) {	/* TSTB */
            setflags(reg[1]);
            return 6;
        }
    case 0xC8: 
        single_op(op, 1);
        return 5;
    case 0xD0:
        /* D0 and D1 are special */
        if (op == 0xD0) {	/* MOV Rn,A */
            r = get_byte();
            reg[r] = reg[0];
            setflags(reg[r]);
            return 8;
        }
        if (op == 0xD1) {	/* MOV B,Rn */
            reg[get_byte()] = reg[1];
            setflags(reg[1]);
            return 7;
        }
    case 0xD8:
        single_op(op, get_byte());
        return 7;
    case 0xE0:
        return branch(op);
    case 0xE8:
    case 0xF0:
    case 0xF8:
        trap(op - 0xE8);
        return 14;
    default:
        invalid(op);
        return 0;
    }
}

/* Dispatch a given IRQ */
static void tms7k_irq_dispatch(unsigned irq)
{
    if (st & ST_I) {
        mode = MODE_RUN;	/* TODO check idle v masked */
        reg[++sp] = st;
        /* The trap mechanism is the same for the rest of this */
        trap(irq);
    }
}

static void tms7k_irq(void)
{
    if ((iocnt0 & 0x03) == 0x03)
        tms7k_irq_dispatch(1);
    else if ((iocnt0 & 0x0C) == 0x0C)
        tms7k_irq_dispatch(2);
    else if ((iocnt0 & 0x30) == 0x30)
        tms7k_irq_dispatch(3);
    else if ((iocnt1 & 0x03) == 0x03)
        tms7k_irq_dispatch(4);
    else if ((iocnt1 & 0x0C) == 0x0C)
        tms7k_irq_dispatch(5);
}

/* TODO: ints, timers */
static void tms7k_tick(unsigned c)
{
}

int tms7k_execute(int c)
{
    unsigned op;
    while(c > 0) {
        clocks = 0;
        tms7k_irq();            
        if (mode == MODE_IDLE)
            return c;
        pc0 = pc;
        op = get_byte();
        clocks += execute_op(op);
        tms7k_tick(clocks);
        c -= clocks;
        clocks = 0;
    }
    /* Report number of excess clocks we ran */
    return -c;
}


void tms7k_reset(void)
{
    reg[0] = pc >> 8;
    reg[1] = pc;
    pc = mem_read8(0xFFFE) << 8;
    pc |= mem_read8(0xFFFF);
    st = 0;
    sp = 1;
    bport = 0xFF;
    addr = 0x00;
    iocnt0 = 0;
    iocnt1 = 0;
}

void tms7k_trace(FILE *f)
{
    if (tms_log)
        fclose(tms_log);
    tms_log = f;
}
