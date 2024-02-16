#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../support1802/1802ops.h"
#include "../support1802/1802debug.h"

/*
 *	We operate big endian because the 1805 forces the issue for later
 *	processors. It's not the ideal way around for us and we will need
 *	to rework the 1802 engine in various places accordingly. In particular
 *	it makes math ops more painful but compares less so.
 */

static uint8_t mem[65536];

static uint16_t sp;
unsigned debug;

static void mwc(uint16_t addr, uint8_t c)
{
	mem[addr] = c;
}

static void mw(uint16_t addr, unsigned c)
{
	mem[addr] = c >> 8;
	mem[addr + 1] = c;
}

static void mwl(uint16_t addr, uint32_t c)
{
	mem[addr] = c >> 24;
	mem[addr + 1] = c >> 16;
	mem[addr + 2] = c >> 8;
	mem[addr + 3] = c;
}

static uint8_t mrc(uint16_t addr)
{
	return mem[addr];
}

static uint16_t mr(uint16_t addr)
{
	return (mem[addr] << 8)| mem[addr + 1];
}

static uint32_t mrl(uint16_t addr)
{
	uint32_t r = mr(addr + 2);
	r |= ((uint32_t)mr(addr)) << 16;
	return r;
}

static void pushc(uint8_t c)
{
	mwc(sp, c);
	if (debug)
		fprintf(stderr, "pushed %uB at %x\n", c, sp);
	sp--;
}

static void push(unsigned u)
{
	sp -= 2;
	mw(sp + 1, u);
	if (debug)
		fprintf(stderr, "pushed %u at %x\n", u, sp + 1);
}

static void pushl(uint32_t l)
{
	sp -= 4;
	mwl(sp + 1, l);
	if (debug)
		fprintf(stderr, "pushed %uL at %x\n", l, sp + 1);
}

static uint8_t popc(void)
{
	sp++;
	return mrc(sp);
}

static unsigned pop(void)
{
	unsigned r = mr(sp + 1);
	if (debug)
		fprintf(stderr, "popped %u at %x\n", r, sp + 1);
	sp += 2;
	return r;
}

static unsigned popl(void)
{
	sp += 4;
	return mrl(sp - 3);
}

static uint8_t byte(unsigned x)
{
	return (uint8_t)x;
}

static uint16_t word(unsigned x)
{
	return(uint16_t)x;
}

void error(const char *p)
{
	puts(p);
	exit(1);
}

static uint16_t do_switchc(uint16_t pc, uint8_t c)
{	
	unsigned len = mr(pc);
	pc += 2;
	while(len--) {
		if (mrc(pc) == c)
			return mr(pc + 1);
		pc += 3;
	}
	return mr(pc);
}

static uint16_t do_switch(uint16_t pc, uint16_t u)
{	
	unsigned len = mr(pc);
	pc += 2;
	while(len--) {
		if (mr(pc) == u)
			return mr(pc + 2);
		pc += 4;
	}
	return mr(pc);
}

static uint16_t do_switchl(uint16_t pc, uint32_t l)
{	
	unsigned len = mr(pc);
	pc += 2;
	while(len--) {
		if (mrl(pc) == l)
			return mr(pc + 4);
		pc += 6;
	}
	return mr(pc);
}

unsigned execute(unsigned initpc, unsigned initsp)
{
	uint16_t pc = initpc;
	uint16_t fp = 0;
	unsigned shift = 0;
	uint32_t ac = 0;
	uint16_t addr;
	uint16_t r0 = 0, r1 = 0, r2 = 0, r3 = 0;

	sp = initsp;

	while(1) {
		uint16_t op = mrc(pc) + shift;

		if (debug) {
			const char *s = opnames[op >> 1];
			if (s == NULL)
				s = "illegal";
			fprintf(stderr, "%04X: %08X %04X %04X %04X %04X %04X %04X: %02X %s\n",
				pc, ac, fp, sp, r0, r1, r2, r3, op, s);
		}
		pc++;
		
		switch(op) {
		case op_shift1:
			shift = 0x100;
			break;
		case op_shift0:
			shift = 0;
			break;
		case op_pushc:
			pushc(ac);
			break;
		case op_pushl:
			pushl(ac);
			break;
		case op_push:
			push(ac);
			break;
		case op_popc:
			popc();
			break;
		case op_popl:
			popl();
			break;
		case op_pop:
			pop();
			break;
		case op_shrl:
			ac = (int32_t)popl() >> word(ac);
			break;
		case op_shrul:
			ac = (uint32_t)popl() >> word(ac);
			break;
		case op_shr:
			ac = (int16_t)pop() >> word(ac);
			break;
		case op_shru:
			ac = (uint16_t)pop() >> word(ac);
			break;
		case op_shll:
			ac = popl() << word(ac);
			break;
		case op_shl:
			ac = pop() << word(ac);
			break;
		case op_plusf:
		case op_plusl:
			ac += popl();
			break;
		case op_plus:
			ac = word(ac + pop());
			break;
		case op_minusf:
		case op_minusl:
			ac = popl() - ac;
			break;
		case op_minus:
			ac = word(pop() - ac);
			break;
		case op_mulf:
		case op_mull:
			ac *= popl();
			break;
		case op_mul:
			ac *= pop();
			ac = word(ac);
			break;
		case op_divf:
		case op_divl:
			ac = ((int32_t)popl()) / ac;
			break;
		case op_divul:
			ac = ((uint32_t)popl()) / ac;
			break;
		case op_div:
			ac = ((int16_t)pop()) / word(ac);
			ac = word(ac);
			break;
		case op_divu:
			ac = ((uint16_t)pop()) / word(ac);
			ac = word(ac);
			break;
		case op_remf:
		case op_reml:
			ac = ((int32_t)popl()) % ac;
			break;
		case op_remul:
			ac = ((uint32_t)popl()) % ac;
			break;
		case op_rem:
			ac = ((int16_t)pop()) % ac;
			ac = word(ac);
			break;
		case op_remu:
			ac = ((uint16_t)pop()) % ac;
			ac = word(ac);
			break;
		case op_negatef:
		case op_negatel:
			ac = -ac;
			break;
		case op_negate:
			ac = -ac;
			ac = word(ac);
			break;
		case op_bandl:
			ac &= popl();
			break;
		case op_band:
			ac &= pop();
			break;
		case op_orl:
			ac |= popl();
			break;
		case op_or:
			ac |= pop();
			break;
		case op_xorl:
			ac ^= popl();
			break;
		case op_xor:
			ac ^= pop();
			break;
		case op_cpll:
			ac ^= 0xFFFFFFFFUL;
			break;
		case op_cpl:
			ac ^= 0xFFFFU;
			break;
		case op_assignc:
			mwc(pop(), ac);
			break;
		case op_assignl:
			mwl(pop(), ac);
			break;
		case op_assign:
			mw(pop(), ac);
			break;
		case op_derefc:
			ac = mrc(ac);
			break;
		case op_derefl:
			ac = mrl(ac);
			break;
		case op_deref:
			ac = mr(ac);
			break;
		case op_constc:
			ac = mrc(pc);
			pc ++;
			break;
		case op_constl:
			ac = mrl(pc);
			pc += 4;
			break;
		case op_const:
			ac = mr(pc);
			pc += 2;
			break;
		case op_notc:
			ac = !byte(ac);
			break;
		case op_notl:
			ac = !ac;
			break;
		case op_not:
			ac = !word(ac);
			break;
		case op_boolc:
			ac = !!byte(ac);
			break;
		case op_booll:
			ac = !!ac;
			break;
		case op_bool:
			ac = !!word(ac);
			break;
		case op_extc:
			ac = (signed long)(signed char)byte(ac);
			break;
		case op_extuc:
			ac &= 0xFF;
			break;
		case op_ext:
			ac = (signed long)(signed int)word(ac);
			break;
		case op_extu:
			ac &= 0xFFFF;
			break;
		case op_f2l:
		case op_l2f:
		case op_f2ul:
		case op_ul2f:
		case op_xxeq:
			addr = pop();
			push(addr);
			push(addr);
			break;
		case op_xxeqpostc:
			mwc(pop(), ac);
			break;
		case op_xxeqpostl:
			mwl(pop(), ac);
			break;
		case op_xxeqpost:
			mw(pop(), ac);
			break;
		case op_postincc:
			addr = ac;
			ac = mrc(addr);
			mw(addr, ac + mrc(pc));
			pc++;
			break;
		case op_postincf:
		case op_postincl:
			addr = ac;
			ac = mrl(addr);
			mwl(addr, ac + mrl(pc));
			pc += 4;
			break;
		case op_postinc:
			addr = ac;
			ac = mr(addr);
			mw(addr, ac + mrl(pc));
			pc += 2;
			break;
		case op_callfname:
			push(pc + 2);
			push(fp);
			pc = mr(pc);
			break;
		case op_callfunc:
			/* Q: check 1802 and compiler side. is AC or pop the addr */
			push(pc + 2);
			push(fp);
			pc = ac;
			break;
		case op_jfalse:
			addr = mr(pc);
			if (byte(ac) == 0)
				pc = addr;
			else
				pc += 2;
			break;
		case op_jtrue:
			addr = mr(pc);
			if (byte(ac) != 0)
				pc = addr;
			else
				pc += 2;
			break;
		case op_jump:
			pc = mr(pc);
			break;
		case op_switchc:
			pc = do_switchc(ac, pc);
			break;
		case op_switchl:
			pc = do_switchl(ac, pc);
			break;
		case op_switch:
			pc = do_switch(ac, pc);
			break;
		case op_cceqf:
		case op_cceql:
			ac = !!(popl() == ac);
			break;
		case op_cceq:
			ac = !!(pop() == word(ac));
			break;
		case op_ccltf:
		case op_ccltl:
			ac = !!((signed long)popl() < (signed long)ac);
			break;
		case op_ccltul:
			ac = !!((uint32_t)popl() < (uint32_t)ac);
			break;
		case op_cclt:
			ac = !!((signed)pop() < (signed)word(ac));
			break;
		case op_ccltu:
			ac = !!((unsigned)pop() < (unsigned)word(ac));
			break;
		case op_cclteqf:
		case op_cclteql:
			ac = !!((signed long)popl() <= (signed long)ac);
			break;
		case op_ccltequl:
			ac = !!((uint32_t)popl() <= (uint32_t)ac);
			break;
		case op_cclteq:
			ac = !!((signed)pop() <= (signed)word(ac));
			break;
		case op_ccltequ:
			ac = !!((unsigned)pop() <= (unsigned)word(ac));
			break;
		case op_nrefc:
			ac = mrc(mr(pc));
			pc += 2;
			break;
		case op_nrefl:
			ac = mrl(mr(pc));
			pc += 2;
			break;
		case op_nref:
			ac = mr(mr(pc));
			pc += 2;
			break;
		case op_lrefc:
			if (debug)
				fprintf(stderr, "lrefc %04X\n", fp + mr(pc) + 1);
			ac = mrc(fp + mr(pc) + 1);	/* FIXME: do the adjust in the compiler */
			pc += 2;
			break;
		case op_lrefl:
			if (debug)
				fprintf(stderr, "lrefl %04X\n", fp + mr(pc) + 1);
			ac = mrl(fp + mr(pc) + 1);
			pc += 2;
			break;
		case op_lref:
			if (debug)
				fprintf(stderr, "lref %04X\n", fp + mr(pc) + 1);
			ac = mr(fp + mr(pc) + 1);
			pc += 2;
			break;
		case op_nstorec:
			mwc(mr(pc), ac);
			pc += 2;
			break;
		case op_nstorel:
			mwl(mr(pc), ac);
			pc += 2;
			break;
		case op_nstore:
			mw(mr(pc), ac);
			pc += 2;
			break;
		case op_lstorec:
			mwc(fp + mr(pc) + 1, ac);
			pc += 2;
			break;
		case op_lstorel:
			mwl(fp + mr(pc) + 1, ac);
			pc += 2;
			break;
		case op_lstore:
			mw(fp + mr(pc) + 1, ac);
			pc += 2;
			break;
		case op_local:
			ac = fp + mr(pc) + 1;
			pc += 2;
			break;
		case op_plusconst:
			ac = word(ac) + mr(pc);
			pc += 2;
			break;
		case op_plus4:
			ac = word(ac + 4);
			break;
		case op_plus3:
			ac = word(ac + 3);
			break;
		case op_plus2:
			ac = word(ac + 2);
			break;
		case op_plus1:
			ac = word(ac + 1);
			break;
		case op_minus4:
			ac = word(ac - 4);
			break;
		case op_minus3:
			ac = word(ac - 3);
			break;
		case op_minus2:
			ac = word(ac - 2);
			break;
		case op_minus1:
			ac = word(ac - 1);
			break;
		case op_fnenter:
			if (debug)
				fprintf(stderr, ";fnenter sp by %x\n", word(-mr(pc)));
			sp += mr(pc);	/* Will be negative so can add not sub */
			fp = sp;
			pc += 2;
			break;
		case op_fnexit:
			if (debug)
				fprintf(stderr, ";fnenxit sp by %x\n", mr(pc));
			sp += mr(pc);
			fp = pop();
			pc = pop();
			break;
		case op_cleanup:
			sp += mr(pc);
			pc += 2;
			break;
		case op_native:
			/* Hack for now for testing FIXME */
			return ac;
		case op_byte:
			ac = byte(ac);
		case op_r0refc:
			ac = byte(r0);
			break;
		case op_r0ref:
			ac = r0;
			break;
		case op_r0storec:
			r0 = byte(ac);
			break;
		case op_r0store:
			r0 = ac;
			break;
		case op_r0derefc:
			ac = mrc(r0);
			break;
		case op_r0deref:
			ac = mr(r0);
			break;
		case op_r0inc1:
			r0++;
			break;
		case op_r0inc2:
			r0 += 2;
			break;
		case op_r0dec:
			r0--;
			break;
		case op_r0dec2:
			r0 -= 2;
			break;
		case op_r0drfpost:
			ac = mr(r0);
			r0 += 2;
			break;
		case op_r0drfpre:
			r0 += 2;
			ac = mr(r0);
			break;

		case op_r1refc:
			ac = byte(r1);
			break;
		case op_r1ref:
			ac = r1;
			break;
		case op_r1storec:
			r1 = byte(ac);
			break;
		case op_r1store:
			r1 = ac;
			break;
		case op_r1derefc:
			ac = mrc(r1);
			break;
		case op_r1deref:
			ac = mr(r1);
			break;
		case op_r1inc1:
			r1++;
			break;
		case op_r1inc2:
			r1 += 2;
			break;
		case op_r1dec:
			r1--;
			break;
		case op_r1dec2:
			r1 -= 2;
			break;
		case op_r1drfpost:
			ac = mr(r1);
			r1 += 2;
			break;
		case op_r1drfpre:
			r1 += 2;
			ac = mr(r1);
			break;
		case op_r2refc:
			ac = byte(r2);
			break;
		case op_r2ref:
			ac = r2;
			break;
		case op_r2storec:
			r2 = byte(ac);
			break;
		case op_r2store:
			r2 = ac;
			break;
		case op_r2derefc:
			ac = mrc(r2);
			break;
		case op_r2deref:
			ac = mr(r2);
			break;
		case op_r2inc1:
			r2++;
			break;
		case op_r2inc2:
			r2 += 2;
			break;
		case op_r2dec:
			r2--;
			break;
		case op_r2dec2:
			r2 -= 2;
			break;
		case op_r2drfpost:
			ac = mr(r2);
			r2 += 2;
			break;
		case op_r2drfpre:
			r2 += 2;
			ac = mr(r2);
			break;

		case op_r3refc:
			ac = byte(r3);
			break;
		case op_r3ref:
			ac = r3;
			break;
		case op_r3storec:
			r3 = byte(ac);
			break;
		case op_r3store:
			r3 = ac;
			break;
		case op_r3derefc:
			ac = mrc(r3);
			break;
		case op_r3deref:
			ac = mr(r3);
			break;
		case op_r3inc1:
			r3++;
			break;
		case op_r3inc2:
			r3 += 2;
			break;
		case op_r3dec:
			r3--;
			break;
		case op_r3dec2:
			r3 -= 2;
			break;
		case op_r3drfpost:
			ac = mr(r3);
			r3 += 2;
			break;
		case op_r3drfpre:
			r3 += 2;
			ac = mr(r3);
			break;

		default:
			fprintf(stderr, "op %x AC %x\n", op, ac);
			error("unknown op\n");
		}
	}
}

int main(int argc, char *argv[])
{
    int fd;

    if (argc == 4 && strcmp(argv[1], "-d") == 0) {
        argv++;
        argc--;
        debug = 1;
    }
    if (argc != 3) {
        fprintf(stderr, "byte1802: test map.\n");
        exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    /* 0000-0xFFFF */
    if (read(fd, mem, sizeof(mem)) < 2) {
        fprintf(stderr, "byte1802: bad test.\n");
        perror(argv[1]);
        exit(1);
    }
    close(fd);

    execute(0, 0xFF00);
}
