/*
 *	Minimal EE200 test environment
 *
 *	In the SRAM bank the registers are laid out as
 *
 *	0x0E	H	(seems to hold PC on IPL changes but otherwise not)
 *			(possibly PC is cached in the CPU)
 *	0x0C	G
 *	0x0A	S
 *	0x08	Z
 *	0x06	Y
 *	0x04	X
 *	0x02	B
 *	0x00	A
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ee200.h"

static uint8_t cpu_ipl = 0;	/* IPL 0-15 */
static uint8_t cpu_mmu = 0;	/* MMU tag 0-7 */
static uint16_t pc;
static uint16_t exec_pc;	/* PC at instruction fetch */
static uint8_t op;
static uint8_t alu_out;
static uint8_t switches = 0xF0;
static uint8_t int_enable;
static unsigned halted;
static unsigned pending_ipl_mask = 0;

#define BS1	0x01
#define BS2	0x02
#define BS3	0x04
#define BS4	0x08

static void logic_flags16(unsigned r);

/*
 *	When packed into C, the flags live in the upper 4 bits of the low byte
 */

#define ALU_L	0x10
#define ALU_F	0x20
#define ALU_M	0x40
#define ALU_V	0x80


static uint16_t mem_read16(uint16_t addr)
{
	uint16_t r = mem_read8(addr) << 8;
	r |= mem_read8(addr + 1);
	return r;
}

static void mem_write16(uint16_t addr, uint16_t val)
{
	mem_write8(addr, val >> 8);
	mem_write8(addr + 1, val);
}

/*
 *	We know from the start address that the processor microsteps begin
 *	inc pc
 *	load opcode from (pc)
 *
 *	however it's not clear this is observable or matters for the moment
 *	because the cases that seem to matter like branches appear to do
 *
 *	inc pc
 *	load opcode
 *	inc pc
 *	load offset
 *	add offset
 *
 *	and then the inc of pc before the next instruction works. What
 *	does need a hard look here is the behaviour of X.
 */
uint8_t fetch(void)
{
	/* Do the pc++ after so that tracing is right */
	uint8_t r = mem_read8(pc);
	pc++;
	return r;
}

uint16_t fetch16(void)
{
	uint16_t r;
	r = mem_read8(pc) << 8;
	pc++;
	r |= mem_read8(pc);
	pc++;
	return r;
}

uint16_t fetch_literal(unsigned length)
{
	uint16_t addr = pc;
	pc += length;
	return addr;
}

static uint8_t reg_read(uint8_t r)
{
	return mem_read8((cpu_ipl << 4) | r);
}

static void reg_write(uint8_t r, uint8_t v)
{
	mem_write8((cpu_ipl << 4) | r, v);
}

/*
 *	On the EE200 a word register specified with an odd value gives
 *	you the upper byte twice.
 */

static uint16_t regpair_addr(uint8_t r)
{
	return r + (cpu_ipl << 4);
}

static uint16_t regpair_read(uint8_t r)
{
	if (r > 15) {
		fprintf(stderr, "Bad regpair encoding %02X %02X %04X\n",
			op, r, exec_pc);
		exit(1);
	}
	return (reg_read((r | 1) ^ 1) << 8) | reg_read((r ^ 1));
}

static void regpair_write(uint8_t r, uint16_t v)
{
	if (r > 15) {
		fprintf(stderr, "Bad regpair encoding %02X %04X\n", op,
			exec_pc);
		exit(1);
	}
	reg_write((r | 1) ^ 1, v >> 8);
	reg_write((r ^ 1), v);
}

/*
 *	Stack helpers
 */

void push(uint16_t val)
{
	uint16_t addr = regpair_read(S);
	addr -= 2;
	mem_write16(addr, val);
	regpair_write(S, addr);
}

uint16_t pop(void)
{
	uint16_t addr = regpair_read(S);
	uint16_t d = mem_read16(addr);
	regpair_write(S, addr + 2);
	return d;
}

void pushbyte(uint8_t val)
{
	uint16_t addr = regpair_read(S);
	addr -= 1;
	mem_write8(addr, val);
	regpair_write(S, addr);
}

uint8_t popbyte(void)
{
	uint16_t addr = regpair_read(S);
	uint8_t d = mem_read8(addr);
	regpair_write(S, addr + 1);
	return d;
}

/*
 *	Load flags
 *	F not touched
 *	L not touched
 *	M cleared then set if MSB of operand
 */
static void ldflags(unsigned r)
{
	alu_out &= ~(ALU_M | ALU_V);
	if (r & 0x80)
		alu_out |= ALU_M;
	if ((r & 0xFF) == 0)
		alu_out |= ALU_V;
}

/*
 *	The docs simply say that F is set if the sign of the destination
 *	register changes.
 *
 *	V - set according to value being zero or non zero
 *	M - set on result being negative
 *	F - set according to overflow rules
 *
 *	L is set only by add so done in add
 */
static void arith_flags(unsigned r, uint8_t a, uint8_t b)
{
	alu_out &= ~(ALU_F | ALU_M | ALU_V);
	if ((r & 0xFF) == 0)
		alu_out |= ALU_V;
	if (r & 0x80)
		alu_out |= ALU_M;
/*	if ((r ^ d) & 0x80)
		alu_out |= ALU_F; */

	/* Overflow for addition is (!r & x & m) | (r & !x & !m) */
	if (r & 0x80) {
		if (!((a | b) & 0x80))
			alu_out |= ALU_F;
	} else {
		if (a & b & 0x80)
			alu_out |= ALU_F;
	}
}

/*
 *	Subtract is similar but the overflow rule probably differs and
 *	L is a borrow not a carry
 */
static void sub_flags(uint8_t r, uint8_t a, uint8_t b)
{
	alu_out &= ~(ALU_F | ALU_M | ALU_V);
	if ((r & 0xFF) == 0)
		alu_out |= ALU_V;
	if (r & 0x80)
		alu_out |= ALU_M;
	if (a & 0x80) {
		if (!((b | r) & 0x80))
			alu_out |= ALU_F;
       	} else {
       		if (b & r & 0x80)
			alu_out |= ALU_F;
	}
}

/*
 *	Logical operations
 *	M is set if there is a 1 in the MSB of the source register
 *	   (or dest register for double register ops)
 *		TODO: before or after operation ?
 */
static void logic_flags(unsigned r)
{
	alu_out &= ~(ALU_M | ALU_V);
	if (r & 0x80)
		alu_out |= ALU_M;
	if (!(r & 0xFF))
		alu_out |= ALU_V;
}

/*
 *	Shift
 *	L is the bit shfited out
 *	M is set as with logic
 *	V is set if result is zero
 *	Left shift/rotate: F is xor of L and M after shift
 *
 */
static void shift_flags(unsigned c, unsigned r)
{
	alu_out &= ~(ALU_L | ALU_M | ALU_V);
	if ((r & 0xFF) == 0)
		alu_out |= ALU_V;
	if (c)
		alu_out |= ALU_L;
	if (r & 0x80)
		alu_out |= ALU_M;
}

/*
 *	Load flags
 *	F not touched
 *	L not touched
 *	M cleared then set if MSB of operand
 */
static void ldflags16(unsigned r)
{
	alu_out &= ~(ALU_M | ALU_V);
	if (r & 0x8000)
		alu_out |= ALU_M;
	if ((r & 0xFFFF) == 0)
		alu_out |= ALU_V;
}

/*
 *	The docs simply say that F is set if the sign of the destination
 *	register changes.
 *
 *	V - set according to value being zero or non zero
 *	M - set on result being negative
 *	F - set according to overflow rules
 *
 *	L is set only by add so done in add
 */
static void arith_flags16(unsigned r, uint16_t a, uint16_t b)
{
	alu_out &= ~(ALU_F | ALU_M | ALU_V);
	if ((r & 0xFFFF) == 0)
		alu_out |= ALU_V;
	if (r & 0x8000)
		alu_out |= ALU_M;
	/* If the result is negative but both inputs were positive then
	   we overflowed */
/* 	if ((r ^ d) & 0x8000)
		alu_out |= ALU_F; */
	/* Overflow for addition is (!r & x & m) | (r & !x & !m) */
	if (r & 0x8000) {
		if (!((a | b) & 0x8000))
			alu_out |= ALU_F;
	} else {
		if (a & b & 0x8000)
			alu_out |= ALU_F;
	}
}

/*
 *	Subtract is similar but the overflow rule probably differs and
 *	L is a borrow not a carry
 */
static void sub_flags16(uint16_t r, uint16_t a, uint16_t b)
{
	alu_out &= ~(ALU_F | ALU_M | ALU_V);
	if ((r & 0xFFFF) == 0)
		alu_out |= ALU_V;
	if (r & 0x8000)
		alu_out |= ALU_M;
	if (a & 0x8000) {
		if (!((b | r) & 0x8000))
        		alu_out |= ALU_F;;
       	} else {
       		if (b & r & 0x8000)
       			alu_out |= ALU_F;;
	}
}

/*
 *	Logical operations
 *	M is set if there is a 1 in the MSB of the source register
 *	   (or dest register for double register ops)
 *		TODO: before or after operation ?
 */
static void logic_flags16(unsigned r)
{
	alu_out &= ~(ALU_M | ALU_V);
	if (r & 0x8000)
		alu_out |= ALU_M;
	if (!(r & 0xFFFF))
		alu_out |= ALU_V;
}

/*
 *	Shift
 *	C is the bit shfited out
 *	M is set as with logic
 *	V is set if result is zero
 *	Left shift/rotate: F is xor of L and M after shift
 *
 */
static void shift_flags16(unsigned c, unsigned r)
{
	alu_out &= ~(ALU_L | ALU_M | ALU_V);
	if ((r & 0xFFFF) == 0)
		alu_out |= ALU_V;
	if (c)
		alu_out |= ALU_L;
	if (r & 0x8000)
		alu_out |= ALU_M;
}


/*
 *	INC/DEC are not full maths ops it seems
 */
static int inc(unsigned reg)
{
	uint8_t r = reg_read(reg);
	reg_write(reg, r + 1);
	arith_flags(r + 1, r, 1);
	return 0;
}

/*
 *	This one is a bit strange
 *
 *	FF->0 expects !F !L M
 *
 */
static int dec(unsigned reg)
{
	uint8_t r = reg_read(reg) - 1;
	reg_write(reg, r);
	alu_out &= ~(ALU_L | ALU_V | ALU_M | ALU_F);
	if (r == 0)
		alu_out |= ALU_V;
	if (r & 0x80)
		alu_out |= ALU_M;
	return 0;
}

static int clr(unsigned reg)
{
	reg_write(reg, 0);
	alu_out &= ~(ALU_F | ALU_L | ALU_M);
	alu_out |= ALU_V;
	return 0;
}

/*
 *	Sets all the flags but rule not clear
 */
static int not(unsigned reg)
{
	uint8_t r = ~reg_read(reg);
	reg_write(reg, r);
	logic_flags(r);
	return 0;
}

/*
 *	The CPU test checks that SRL FF sets C and expects N to be clear
 */
static int sra(unsigned reg)
{
	uint8_t v;
	uint8_t r = reg_read(reg);

	v = r >> 1;
	if (v & 0x40)
		v |= 0x80;
	shift_flags(r & 1, v);
	r = v;
	reg_write(reg, v);
	return 0;
}

/*
 *	Left shifts also play with F
 */
static int sll(unsigned reg)
{
	uint8_t r = reg_read(reg);
	uint8_t v;

	v = r << 1;
	shift_flags((r & 0x80), v);
	alu_out &= ~ALU_F;
	/* So annoying C lacks a ^^ operator */
	switch (alu_out & (ALU_L | ALU_M)) {
	case ALU_L:
	case ALU_M:
		alu_out |= ALU_F;
		break;
	}
	r = v;
	reg_write(reg, v);
	return 0;
}

/* The CPU test checks that an RR with the low bit set propogates carry. It
   also checks that FF shift to 7F leaves n clear. The hex digit conversion
   confirms that the rotates are 9bit rotate through carry */
static int rrc(unsigned reg)
{
	uint8_t r = reg_read(reg);
	uint8_t c;

	c = r & 1;

	r >>= 1;
	r |= (alu_out & ALU_L) ? 0x80 : 0;

	shift_flags(c, r);

	reg_write(reg, r);
	return 0;
}

/* An RL of FF sets C but either clears N or leaves it clear */
static int rlc(unsigned reg)
{
	uint8_t r = reg_read(reg);
	uint8_t c;

	c = r & 0x80;
	r <<= 1;
	r |= (alu_out & ALU_L) ? 1 : 0;

	shift_flags(c, r);
	alu_out &= ~ALU_F;
	/* So annoying C lacks a ^^ operator */
	switch (alu_out & (ALU_L | ALU_M)) {
	case ALU_L:
	case ALU_M:
		alu_out |= ALU_F;
		break;
	}
	reg_write(reg, r);
	return 0;
}

/*
 *	Add changes all the flags so fix up L
 */
static int add(unsigned dst, unsigned src)
{
	uint16_t d = reg_read(dst);
	uint16_t s = reg_read(src);
	reg_write(dst, d + s);
	arith_flags(d + s, d, s);
	alu_out &= ~ALU_L;
	if ((s + d) & 0x100)
		alu_out |= ALU_L;
	return 0;
}

/*
 *	Subtract changes all the flags
 */
static int sub(unsigned dst, unsigned src)
{
	unsigned s = reg_read(src);
	unsigned d = reg_read(dst);
	unsigned r =  s - d;
	reg_write(dst, r);
	sub_flags(r, s, d);
	alu_out &= ~ALU_L;
	if (d <= s)
		alu_out |= ALU_L;
	return 0;
}

/*
 *	Logic operations
 */
static int and(unsigned dst, unsigned src)
{
	uint8_t r = reg_read(dst) & reg_read(src);
	reg_write(dst, r);
	logic_flags(r);
	return 0;
}

static int or(unsigned dst, unsigned src)
{
	uint8_t r = reg_read(dst) | reg_read(src);
	reg_write(dst, r);
	logic_flags(r);
	return 0;
}

static int xor(unsigned dst, unsigned src)
{
	uint8_t r = reg_read(dst) ^ reg_read(src);
	reg_write(dst, r);
	logic_flags(r);
	return 0;
}

static int mov(unsigned dst, unsigned src)
{
	uint8_t r = reg_read(src);
	reg_write(dst, r);
	logic_flags(r);
	return 0;
}


/* 16bit versions */

static uint16_t inc16(uint16_t a)
{
	arith_flags(a + 1, a, 1);
	return a + 1;
}

static uint16_t dec16(uint16_t a)
{
	uint16_t r = a - 1;
	alu_out &= ~(ALU_L | ALU_V | ALU_M | ALU_F);
	if ((r & 0xFFFF) == 0)
		alu_out |= ALU_V;
	if (r & 0x8000)
		alu_out |= ALU_M;
	return r;
}

static uint16_t clr16(uint16_t a)
{
	alu_out &= ~(ALU_F | ALU_L | ALU_M);
	alu_out |= ALU_V;
	return 0;
}

static uint16_t not16(uint16_t a)
{
	uint16_t r = ~a;
	logic_flags16(r);
	return r;
}

static uint16_t sra16(uint16_t a)
{
	uint16_t v;
	uint16_t r = a;

	v = r >> 1;
	if (v & 0x4000)
		v |= 0x8000;
	shift_flags16(r & 1, v);
	r = v;

	return r;
}

static uint16_t sll16(uint16_t a)
{
	uint16_t v;
	uint16_t r = a;

	v = r << 1;
	shift_flags16((r & 0x8000), v);
	alu_out &= ~ALU_F;
	/* So annoying C lacks a ^^ operator */
	switch (alu_out & (ALU_L | ALU_M)) {
	case ALU_L:
	case ALU_M:
		alu_out |= ALU_F;
		break;
	}
	r = v;
	return r;
}

static uint16_t rrc16(uint16_t a)
{
	uint16_t r = a;
	uint16_t c;

	c = r & 1;

	r >>= 1;
	r |= (alu_out & ALU_L) ? 0x8000 : 0;

	shift_flags16(c, r);

	return r;
}

static uint16_t rlc16(uint16_t a)
{
	uint16_t r = a;
	uint16_t c;

	c = r & 0x8000;

	r <<= 1;
	r |= (alu_out & ALU_L) ? 1 : 0;

	shift_flags16(c, r);
	alu_out &= ~ALU_F;
	/* So annoying C lacks a ^^ operator */
	switch (alu_out & (ALU_L | ALU_M)) {
	case ALU_L:
	case ALU_M:
		alu_out |= ALU_F;
		break;
	}
	return r;
}

static int add16(unsigned dsta, unsigned a, unsigned b)
{
	mem_write16(dsta, a + b);
	arith_flags16(a + b, a, b);
	alu_out &= ~ALU_L;
	if ((b + a) & 0x10000)
		alu_out |= ALU_L;
	return 0;
}

static int sub16(unsigned dsta, unsigned a, unsigned b)
{
	unsigned r = b - a;
	mem_write16(dsta, r);
	sub_flags16(r, b, a);
	alu_out &= ~ALU_L;
	if (a <= b)
		alu_out |= ALU_L;
	return 0;
}

static int and16(unsigned dsta, unsigned a, unsigned b)
{
	uint16_t r = a & b;
	mem_write16(dsta, r);
	logic_flags16(r);
	return 0;
}

static int or16(unsigned dsta, unsigned a, unsigned b)
{
	uint16_t r = a | b;
	mem_write16(dsta, r);
	logic_flags16(r);
	return 0;
}

static int xor16(unsigned dsta, unsigned a, unsigned b)
{
	uint16_t r = a ^ b;
	mem_write16(dsta, r);
	logic_flags16(r);
	return 0;
}

static int mov16(unsigned dsta, unsigned srcv)
{
	mem_write16(dsta, srcv);
	logic_flags16(srcv);
	return 0;
}

/*
 *	Address generator (with side effects). We don't know when the
 *	pre-dec/post-inc hits the register
 */

static uint16_t indexed_address(unsigned size)
{
	uint8_t idx = fetch();
	unsigned r = idx >> 4;
	unsigned addr;
	int8_t offset = 0;	/* Signed or not ? */

	if (idx & 0x08)
		offset = fetch();
	switch (idx & 0x03) {
	case 0:
		addr = regpair_read(r) + offset;
		break;
	case 1:
		addr = regpair_read(r);
		regpair_write(r, addr + size);
		addr = addr + offset;
		break;
	case 2:
		addr = regpair_read(r);
		addr -= size;
		regpair_write(r, addr);
		addr = addr + offset;
		break;
	default:
		fprintf(stderr, "Unknown indexing mode %02X at %04X\n",
			idx, exec_pc);
		exit(1);
	}
	if (idx & 0x04)
		addr = mem_read16(addr);
	return addr;
}

static uint16_t decode_address(unsigned size, unsigned mode)
{
	uint16_t addr;
	uint16_t indir = 0;

	switch (mode) {
	case 0:
		addr = pc;
		pc += size;
		indir = 0;
		break;
	case 1:
		addr = pc;
		pc += 2;
		indir = 1;
		break;
	case 2:
		addr = pc;
		pc += 2;
		indir = 2;
		break;
	case 3:
		addr = (int8_t) fetch();
		addr += pc;
		indir = 0;
		break;
	case 4:
		addr = (int8_t) fetch();
		addr += pc;
		indir = 1;
		break;
	case 5:
		/* Indexed modes */
		addr = indexed_address(size);
		indir = 0;
		break;
	case 6:
	case 7:
		fprintf(stderr, "unknown address indexing %X at %04X\n",
			mode, exec_pc);
		exit(1);
	default:
		/* indexed off a register */
		addr = regpair_read((mode & 7) << 1);
		indir = 0;
		break;
	}
	while (indir--)
		addr = mem_read16(addr);
	return addr;
}

/*
 *	Branch instructions
 */

static int branch_op(void)
{
	unsigned t;
	int8_t off;

	switch (op & 0x0F) {
	case 0:		/* BL   Branch if link is set */
		t = (alu_out & ALU_L);
		break;
	case 1:		/* BNL  Branch if link is not set */
		t = !(alu_out & ALU_L);
		break;
	case 2:		/* BF   Branch if fault is set */
		t = (alu_out & ALU_F);
		break;
	case 3:		/* BNF  Branch if fault is not set */
		t = !(alu_out & ALU_F);
		break;
	case 4:		/* BZ   Branch if zero */
		t = (alu_out & ALU_V);
		break;
	case 5:		/* BNZ  Branch if non zero */
		t = !(alu_out & ALU_V);
		break;
	case 6:		/* BM   Branch if minus */
		t = alu_out & ALU_M;
		break;
	case 7:		/* BP   Branch if plus */
		t = !(alu_out & ALU_M);
		break;
	case 8:		/* BGZ  Branch if greater than zero */
		/* Branch if both M and V are zero */
		t = !(alu_out & (ALU_M | ALU_V));
		break;
	case 9:		/* BLE  Branch if less than or equal to zero */
		t = alu_out & (ALU_M | ALU_V);
		break;
	case 10:		/* BS1  */
		t = (switches & BS1);
		break;
	case 11:		/* BS2 */
		t = (switches & BS2);
		break;
	case 12:		/* BS3 */
		t = (switches & BS3);
		break;
	case 13:		/* BS4 */
		t = (switches & BS4);
		break;
	case 14:		/* BTM - branch on teletype mark - CPU4 only ? */
		t = 0;
		break;
	case 15:		/* Branch on even parity (BRA if option not fitted) */
		t = 1;
		break;
	}
	/* We'll keep pc and reg separate until we know if/how it fits memory */
	off = fetch();
	/* Offset is applied after fetch leaves PC at next instruction */
	if (t) {
		pc += off;
		return 18;
	}
	return 9;
}

#define SWITCH_IPL_RETURN  1
#define SWITCH_IPL_RETURN_MODIFIED 2
#define SWITCH_IPL_INTERRUPT 3

static void switch_ipl(unsigned new_ipl, unsigned mode)
{
	/*  C register layout:
	 *
	 *  15-12   Previous IPL
	 *  11-8    Unknown, not used?
	 *  7       Value (Zero)
	 *  6       Minus (Sign)
	 *  5       Fault (Overflow)
	 *  4       Link (Carry)
	 *  3-0     Memory MAP aka MMU aka Page Table Base
	 */

	unsigned old_ipl = cpu_ipl;
	if (mode != SWITCH_IPL_RETURN_MODIFIED) {
		// Save pc
		regpair_write(P, pc);

		// Save flags and MAP
		reg_write(CL, alu_out | cpu_mmu);
	}
	cpu_ipl = new_ipl;

	// We are now on the new level

	// restore pc
	pc = regpair_read(P);

	if (mode == SWITCH_IPL_INTERRUPT) {
		// Save previous IPL, so we can return later
		reg_write(CH, old_ipl << 4);
	}

	uint8_t cl = reg_read(CL);
	// Restore flags
	alu_out = cl & (ALU_L | ALU_F | ALU_M | ALU_V);

	// Restore memory MAP
	cpu_mmu = cl & 0x7;
}

/* Low operations */
static int low_op(void)
{
	switch (op) {
	case 0x00:		/* HALT */
		halted = 1;
		break;
	case 0x01:		/* NOP */
		return 4;
	case 0x02:		/* SF   Set Fault */
		alu_out |= ALU_F;
		break;
	case 0x03:		/* RF   Reset Fault */
		alu_out &= ~ALU_F;
		break;
	case 0x04:		/* EI   Enable Interrupts */
		int_enable = 1;
		break;
	case 0x05:		/* DI   Disable Interrupts */
		int_enable = 0;
		return 8;
	case 0x06:		/* SL   Set Link */
		alu_out |= ALU_L;
		break;
	case 0x07:		/* RL   Clear Link */
		alu_out &= ~ALU_L;
		break;
	case 0x08:		/* CL   Complement Link */
		alu_out ^= ALU_L;
		break;
	case 0x09:		/* RSR  Return from subroutine */
		pc = regpair_read(X);
		regpair_write(X, pop());
		break;
	case 0x0A:		/* RI   Return from interrupt */
		switch_ipl(reg_read(CH) >> 4, SWITCH_IPL_RETURN);
		break;
	case 0x0B:		/* RIM  Return from interrupt modified */
		switch_ipl(reg_read(CH) >> 4, SWITCH_IPL_RETURN_MODIFIED);
		break;
	case 0x0C:
		/* Enable link to teletype */
		break;
	case 0x0D:
		/* No flag effects */
		regpair_write(X, pc);
		break;
		/*
		 * "..0x0E ought to be a long (but not infinite) loop.
		 *  The delay opcode 0x0E should 4.5ms long.
		 */
	case 0x0E:		/* DELAY */
		break;
	case 0x0F:
		/* Unused */
		break;
	}
	return 0;
}

/*
 *	Jump doesn't quite match the op decodes for the load/store strangely
 *
 *	0 would be nonsense
 *	1,2,3 seem to match
 *	5 is used for what we would expect to be indexed modes but we only
 *	see it used as if the indexing byte was implicitly 8 (A + offset)
 *
 *	4,6,7 we don't know but 6,7 are not used for load/store and 4 is
 *	an indirect so is perhaps mem_read16(mem_read16(fetch16()));
 *
 *	7E and 7F are repurposed as multi register push/pop on CPU6
 */
static int jump_op(void)
{
	uint16_t new_pc;
	if (op == 0x76) {	/* syscall is a mystery */
		uint8_t old_ipl = cpu_ipl;
		unsigned old_s = regpair_read(S);
		cpu_ipl = 15;
		/* Unclear if this also occurs */
		/* Also seems to propogate S but can't be sure */
		regpair_write(S, old_s);
		reg_write(CH, old_ipl);
		return 0;
	}
	if (op == 0x7E) {
		/* Push a block of registers given the last register to push
		   and the count */
		uint8_t r = fetch();
		uint8_t c = (r & 0x0F);
		unsigned addr = regpair_read(S);
		r >>= 4;
		/* We push the highest one first */
		r += c;
		r &= 0x0F;
		c++;
		/* A push of S will use the original S before the push insn. */
		while(c--) {
			mem_write8(--addr, reg_read(r));
			r--;
			r &= 0x0F;
		}
		regpair_write(S, addr);
		return 0;
	}
	if (op == 0x7F) {
		/* Pop a block of registers given the first register and count */
		uint8_t r = fetch();
		uint8_t c = (r & 0x0F) + 1;
		unsigned addr = regpair_read(S);
		r >>= 4;
		/* A pop of S will always update S at the end */
		while(c--) {
			reg_write(r, mem_read8(addr++));
			r++;
			r = r & 0x0F;
		}
		regpair_write(S, addr);
		return 0;
	}
	/* We don't know what 0x70 does (it's invalid but I'd guess it jumps
	   to the following byte */
	new_pc = decode_address(2, op & 0x07);
	if (op & 0x08) {
		/* Subroutine calls are a hybrid of the classic call/ret and
		   branch/link. The old X is stacked, X is set to the new
		   return address and then we jump */
		push(regpair_read(X));
		regpair_write(X, pc);
		/* This is specifically stated in the EE200 manual */
		regpair_write(P, new_pc);
	}
	pc = new_pc;
	return 0;
}

/*
 *	This appears to work like the other loads and not affect C
 */
static int x_op(void)
{
	/* Valid modes 0-5 */
	uint16_t addr = decode_address(2, op & 7);
	uint16_t r;
	if (op & 0x08) {
		r = regpair_read(X);
		mem_write16(addr, r);
		ldflags16(r);
	} else {
		r = mem_read16(addr);
		regpair_write(X, r);
		ldflags16(r);
	}
	return 0;
}

static int loadbyte_op(void)
{
	uint16_t addr = decode_address(1, op & 0x0F);
	uint8_t r = mem_read8(addr);

	if (op & 0x40)
		reg_write(BL, r);
	else
		reg_write(AL, r);
	ldflags(r);
	return 0;
}

static int loadword_op(void)
{
	uint16_t addr = decode_address(2, op & 0x0F);
	uint16_t r = mem_read16(addr);

	if (op & 0x40)
		regpair_write(B, r);
	else
		regpair_write(A, r);
	ldflags16(r);
	return 0;
}

static int storebyte_op(void)
{
	uint16_t addr = decode_address(1, op & 0x0F);
	uint8_t r;

	if (op & 0x40)
		r = reg_read(BL);
	else
		r = reg_read(AL);

	mem_write8(addr, r);
	ldflags(r);
	return 0;
}

static int storeword_op(void)
{
	uint16_t addr = decode_address(2, op & 0x0F);
	uint16_t r;

	if (op & 0x40)
		r = regpair_read(B);
	else
		r = regpair_read(A);

	mem_write16(addr, r);
	ldflags16(r);

	return 0;
}

static int loadstore_op(void)
{
	switch (op & 0x30) {
	case 0x00:
		return loadbyte_op();
	case 0x10:
		return loadword_op();
	case 0x20:
		return storebyte_op();
	case 0x30:
		return storeword_op();
	default:
		fprintf(stderr, "internal error loadstore\n");
		exit(1);
	}
}

static int misc2x_op(void)
{
	unsigned reg = AL;
	if (!(op & 8)) {
		reg = fetch();
		reg >>= 4;
	}

	switch (op) {
	case 0x20:
		return inc(reg);
	case 0x21:
		return dec(reg);
	case 0x22:
		return clr(reg);
	case 0x23:
		return not(reg);
	case 0x24:
		return sra(reg);
	case 0x25:
		return sll(reg);
	case 0x26:
		return rrc(reg);
	case 0x27:
		return rlc(reg);
	case 0x28:
		return inc(AL);
	case 0x29:
		return dec(AL);
	case 0x2A:
		return clr(AL);
	case 0x2B:
		return not(AL);
	case 0x2C:
		return sra(AL);
	case 0x2D:
		return sll(AL);
	/* On the EE200 these would be inc XL/dec XL but they are not present and
	   X is almost always handled as a 16bit register only */
	case 0x2E:
	case 0x2F:
		return 0;
	default:
		fprintf(stderr, "internal error misc2\n");
		exit(1);
	}
}

static uint16_t misc3x_op_impl(unsigned op, uint16_t val)
{
	switch (op) {
	case 0x30:
		return inc16(val);
	case 0x31:
		return dec16(val);
	case 0x32:
		return clr16(val);
	case 0x33:
		return not16(val);
	case 0x34:
		return sra16(val);
	case 0x35:
		return sll16(val);
	case 0x36:
		return rrc16(val);
	case 0x37:
		return rlc16(val);
	case 0x38:
		return inc16(val);
	case 0x39:
		return dec16(val);
	case 0x3A:
		return clr16(val);
	case 0x3B:
		return not16(val);
	case 0x3C:
		return sra16(val);
	case 0x3D:
		return sll16(val);
	default:
		fprintf(stderr, "internal error misc3 %x\n", op);
		exit(1);
	}
}

/* Like misc2x but word
 * If the explicit register is odd, it operates on memory
*/
static int misc3x_op(void)
{
	unsigned reg;
	// Special cases that don't fit general pattern
	if (op == 0x3E) {
		regpair_write(X, inc16(regpair_read(X)));
		return 0;
	}
	if (op == 0x3F) {
		regpair_write(X, dec16(regpair_read(X)));
		return 0;
	}

	if (op & 8) {
		// Implicit ops that work on A
		regpair_write(A, misc3x_op_impl(op, regpair_read(A)));
		return 0;
	}
	reg = (fetch() >> 4) & 0x0F;
	regpair_write(reg,
		misc3x_op_impl(op, regpair_read(reg)));
	return 0;
}

/* Mostly ALU operations on AL */
static int alu4x_op(void)
{
	unsigned src = BL, dst = AL;
	if ((!(op & 0x08))) {
		dst = fetch();
		src = dst >> 4;
		dst &= 0x0F;
	}
	switch (op) {
	case 0x40:		/* add */
		return add(dst, src);
	case 0x41:		/* sub */
		return sub(dst, src);
	case 0x42:		/* and */
		return and(dst, src);
	case 0x43:		/* or */
		return or(dst, src);
	case 0x44:		/* xor */
		return xor(dst, src);
	case 0x45:		/* mov */
		return mov(dst, src);
	case 0x48:
		return add(BL, AL);
	case 0x49:
		return sub(BL, AL);
	case 0x4A:
		return and(BL, AL);
	case 0x4B:
		return mov(XL, AL);
	case 0x4C:
		return mov(YL, AL);
	case 0x4D:
		return mov(BL, AL);
	case 0x46:
	case 0x47:
	case 0x4E:		/* unused */
	case 0x4F:		/* unused */
		fprintf(stderr, "Unknown ALU4 op %02X at %04X\n", op,
			exec_pc);
		return 0;
	default:
		fprintf(stderr, "internal error alu4\n");
		exit(1);
	}
}

/* Much like ALU4x but word */

static int alu5x_op(void)
{
	unsigned src, dst;
	uint16_t a; // First argument isn't always dst anymore.
	uint16_t b;
	uint16_t dsta;
	uint16_t movv; // move value is usually source.
	               // But when is a choice of a memory operand, mov ignores everything else.
	if (!(op & 0x08)) {
		dst = fetch();
		src = dst >> 4;
		movv = b = regpair_read(src & 0x0E);
		dsta = regpair_addr(dst & 0x0E);
		a = regpair_read(dst & 0XE);
	} else {
		a = regpair_read(B);
		movv = b = regpair_read(A);
		dsta = regpair_addr(B);
	}
	switch (op) {
	case 0x50:		/* add */
		return add16(dsta, a, b);
	case 0x51:		/* sub */
		return sub16(dsta, a, b);
	case 0x52:		/* and */
		return and16(dsta, a, b);
	case 0x53:		/* or */
		return or16(dsta, a, b);
	case 0x54:		/* xor */
		return xor16(dsta, a, b);
	case 0x55:		/* mov */
		return mov16(dsta, movv);
	case 0x56:		/* unused */
	case 0x57:		/* unused */
		fprintf(stderr, "Unknown ALU5 op %02X at %04X\n", op,
			exec_pc);
		return 0;
	case 0x58:
		return add16(dsta, a, b);
	case 0x59:
		return sub16(dsta, a, b);
	case 0x5A:
		return and16(dsta, a, b);
	/* These are borrowed for moves */
	case 0x5B:
		return mov16(regpair_addr(X), movv);
	case 0x5C:
		return mov16(regpair_addr(Y), movv);
	case 0x5D:
		return mov16(regpair_addr(B), movv);
	case 0x5E:
		return mov16(regpair_addr(Z), movv);
	case 0x5F:
		return mov16(regpair_addr(S), movv);
	default:
		fprintf(stderr, "internal error alu5\n");
		exit(1);
	}
}

/*
 *	There are explicit flags for Minus, Link (carry), Value (zero),
 *	Fault (Signed overflow tracking).
 */
static char *flagcode(void)
{
	static char buf[5];
	strcpy(buf, "----");
	if (alu_out & ALU_F)
		*buf = 'F';
	if (alu_out & ALU_L)
		buf[1] = 'L';
	if (alu_out & ALU_M)
		buf[2] = 'M';
	if (alu_out & ALU_V)
		buf[3] = 'V';
	return buf;
}

void ee200_interrupt(unsigned trace)
{
	unsigned old_ipl = cpu_ipl;
	unsigned pending_ipl;

	if (int_enable == 0)
		return;

	pending_ipl = pending_ipl_mask == 0 ? 0 : 31 - __builtin_clz(pending_ipl_mask);

	if (pending_ipl > cpu_ipl) {
		halted = 0;
		switch_ipl(pending_ipl, SWITCH_IPL_RETURN);

		if (trace)
			fprintf(stderr,
				"Interrupt %X: New PC = %04X, previous IPL %X\n",
				cpu_ipl, pc, old_ipl);
	}
}

// Not quite accurate to real hardware, but hopefully close enough
void cpu_assert_irq(unsigned ipl) {
	pending_ipl_mask |= 1 << ipl;
}

void cpu_deassert_irq(unsigned ipl) {
	pending_ipl_mask &= ~(1 << ipl);
}

unsigned ee200_execute_one(unsigned trace)
{
	exec_pc = pc;

	ee200_interrupt(trace);
	if (trace)
		fprintf(stderr, "CPU %04X: ", pc);
	op = fetch();
	if (trace == 2) {
		fprintf(stderr,
			"%02X %s A:%04X B:%04X X:%04X Y:%04X Z:%04X S:%04X C:%04X LVL:%x | ",
			op, flagcode(), regpair_read(A), regpair_read(B),
			regpair_read(X), regpair_read(Y), regpair_read(Z),
			regpair_read(S), regpair_read(C), cpu_ipl);
		disassemble(op);
	} else if (trace) {
		fprintf(stderr,
			"%02X %s A:%04X B:%04X X:%04X Y:%04X Z:%04X S:%04X | ",
			op, flagcode(), regpair_read(A), regpair_read(B),
			regpair_read(X), regpair_read(Y), regpair_read(Z),
			regpair_read(S));
		disassemble(op);
	}
	if (op < 0x10)
		return low_op();
	if (op < 0x20)
		return branch_op();
	/* 20-5F is sort of ALU stuff but other things seem to have been shoved
	   into the same space */
	if (op < 0x30)
		return misc2x_op();
	if (op < 0x40)
		return misc3x_op();
	if (op < 0x50)
		return alu4x_op();
	if (op < 0x60)
		return alu5x_op();
	if (op < 0x70)
		return x_op();
	if (op < 0x80)
		return jump_op();
	return loadstore_op();
}

uint16_t ee200_pc(void)
{
	return exec_pc;
}

void set_pc_debug(uint16_t new_pc) {
	pc = new_pc;
}

void reg_write_debug(uint8_t r, uint8_t v) {
	reg_write(r, v);
}

void regpair_write_debug(uint8_t r, uint16_t v) {
	regpair_write(r, v);
}

void ee200_set_switches(unsigned v)
{
	switches = v;
}

unsigned ee200_halted(void)
{
	return halted;
}

void ee200_init(void)
{
}


/*
 *	Mini EE200 debug machine
 */

#include <unistd.h>
#include <fcntl.h>
static uint8_t mem[0xE000];

uint8_t mem_read8(uint16_t addr)
{
	if (addr < sizeof(mem))
		return mem[addr];
	else
		return 0xFF;
}

uint8_t mem_read8_debug(uint16_t addr)
{
	if (addr < sizeof(mem))
		return mem[addr];
	else
		return 0xFF;
}

void mem_write8(uint16_t addr, uint8_t val)
{
	if (addr == 0xFFFF) {
		if (val)
			printf("%d\n", val);
		exit(!!val);
	}
	if (addr < sizeof(mem))
		mem[addr] = val;
}

int main(int argc, char * argv[])
{
	int fd;
	unsigned debug = 0;

	if (argc == 4 && strcmp(argv[1], "-d") == 0) {
		argv++;
		argc--;
		debug = 1;
	}
	if (argc != 3) {
		fprintf(stderr, "ee200: test map.\n");
		exit(1);
	}
	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror(argv[1]);
		exit(1);
	}
	/* 0100-0xDFFF */
	if (read(fd, mem, 0xE000) < 0x110) {
		fprintf(stderr, "ee200: bad test.\n");
		perror(argv[1]);
		exit(1);
	}
	close(fd);

	set_pc_debug(0x0100);

	while (1)
		ee200_execute_one(debug);
	return 0;
}
