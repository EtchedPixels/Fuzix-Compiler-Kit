/*
 *	Fake mini machine to run compiler tests
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "libz80/z80.h"
#include "z80dis.h"

static uint8_t ram[65536];
static Z80Context cpu_z80;
static unsigned trace;

static uint8_t mem_read(int unused, uint16_t addr)
{
    return ram[addr];
}

static void mem_write(int unused, uint16_t addr, uint8_t val)
{
    ram[addr] = val;
}

static uint8_t io_read(int unused, uint16_t port)
{
    return 0xFF;
}

static void io_write(int unused, uint16_t port, uint8_t value)
{
    switch(port & 0xFF) {
    case 0xFD:
        if (value < 32 || value > 127)
            printf("\\x%02X", value);
        else
            putchar(value);
        fflush(stdout);
        return;
    case 0xFF:
        if (value)
            fprintf(stderr, "***FAIL %d\n", value);
        exit(value);
    default:
        fprintf(stderr, "***BAD PORT %d\n", port);
        exit(1);
    }
}

static unsigned int nbytes;

uint8_t z80dis_byte(uint16_t addr)
{
	uint8_t r = mem_read(0, addr);
	fprintf(stderr, "%02X ", r);
	nbytes++;
	return r;
}

uint8_t z80dis_byte_quiet(uint16_t addr)
{
	return mem_read(0, addr);
}

static void z80_trace(unsigned unused)
{
	static uint32_t lastpc = -1;
	char buf[256];

	if (!trace)
		return;
	nbytes = 0;
	/* Spot XXXR repeating instructions and squash the trace */
	if (cpu_z80.M1PC == lastpc && z80dis_byte_quiet(lastpc) == 0xED &&
		(z80dis_byte_quiet(lastpc + 1) & 0xF4) == 0xB0) {
		return;
	}
	lastpc = cpu_z80.M1PC;
	fprintf(stderr, "%04X: ", lastpc);
	z80_disasm(buf, lastpc);
	while(nbytes++ < 6)
		fprintf(stderr, "   ");
	fprintf(stderr, "%-16s ", buf);
	fprintf(stderr, "[ %02X:%02X %04X %04X %04X %04X %04X %04X ]\n",
		cpu_z80.R1.br.A, cpu_z80.R1.br.F,
		cpu_z80.R1.wr.BC, cpu_z80.R1.wr.DE, cpu_z80.R1.wr.HL,
		cpu_z80.R1.wr.IX, cpu_z80.R1.wr.IY, cpu_z80.R1.wr.SP);
}


int main(int argc, char *argv[])
{
    int fd;
    if (argc == 4 && strcmp(argv[1], "-d") == 0) {
        argv++;
        argc--;
        trace = 1;
    }
    if (argc != 3) {
        fprintf(stderr, "emu85: test map.\n");
        exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    if (read(fd, ram, 65536) < 8) {
        fprintf(stderr, "emuz80: bad test.\n");
        perror(argv[1]);
        exit(1);
    }
    close(fd);
    Z80RESET(&cpu_z80);
    cpu_z80.ioRead = io_read;
    cpu_z80.ioWrite = io_write;
    cpu_z80.memRead = mem_read;
    cpu_z80.memWrite = mem_write;
    cpu_z80.trace = z80_trace;

    while(1)
        Z80ExecuteTStates(&cpu_z80, 1000);
}
