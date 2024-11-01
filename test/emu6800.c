#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "6800.h"

static uint8_t ram[65536];

struct m6800 cpu;

void m6800_sci_change(struct m6800 *cpu)
{
	/* SCI changed status - could add debug here FIXME */
}

void m6800_tx_byte(struct m6800 *cpu, uint8_t byte)
{
	write(1, &byte, 1);
}

/* I/O ports: nothing for now */

void m6800_port_output(struct m6800 *cpu, int port)
{
}

uint8_t m6800_port_input(struct m6800 *cpu, int port)
{
	return 0xFF;
}

uint8_t m6800_inport(uint8_t addr)
{
	return 0xFF;
}

#if	0
void m6800_outport(uint8_t addr, uint8_t val)
{
	if (addr == 0xFF) {
		if (val)
			fprintf(stderr, "***FAIL %d\n", val);
		exit(1);
	}
}
#endif

void m68hc11_spi_begin(struct m6800 *cpu, uint8_t val)
{
}

uint8_t m68hc11_spi_done(struct m6800 *cpu)
{
	return 0xFF;
}

void m68hc11_port_direction(struct m6800 *cpu, int port)
{
}


uint8_t m6800_read_op(struct m6800 *cpu, uint16_t addr, int debug)
{
	if (addr >> 8 == 0xFE)
		return m6800_inport(addr & 0xFF);
	return ram[addr];
}

uint8_t m6800_debug_read(struct m6800 *cpu, uint16_t addr)
{
	return m6800_read_op(cpu, addr, 1);
}

uint8_t m6800_read(struct m6800 *cpu, uint16_t addr)
{
	return m6800_read_op(cpu, addr, 0);
}

static unsigned char fefcval=0;

void m6800_write(struct m6800 *cpu, uint16_t addr, uint8_t val)
{
	int x;

	/* Writes to certain addresses act like system calls */
	/* 0xFEFF:  exit() with the val as the exit value */
	/* 0xFEFE:  putchar(val) */
	/* 0xFEFC/D: print out the 16-bit value as a decimal */

	switch(addr) {
	    case 0xFEFF:
		if (val == 0)
			exit(0);
		fprintf(stderr, "***FAIL %u\n", val);
		exit(1);
	    case 0xFEFE:
		putchar(val);
		break;
	    case 0xFEFD:
		/* Make the value signed */
		x= (fefcval << 8) | val;
		if (x>0x8000) x-= 0x10000;
		printf("%d\n", x);
		break;
	    case 0xFEFC:
		fefcval= val;	/* Save high byte for now */
		break;
	    default:
		ram[addr & 0xFFFF] = val;
	}
}

/* TODO: CPU setting option */
int main(int argc, char *argv[])
{
	int fd;
	unsigned debug = 0;

	if (argc == 5 && strcmp(argv[1], "-d") == 0) {
		debug = 1;
		argv++;
		argc--;
	}
	if (argc != 4) {
		fprintf(stderr, "emu6800: cpu test map.\n");
		exit(1);
	}
	fd = open(argv[2], O_RDONLY);
	if (fd == -1) {
		perror(argv[2]);
		exit(1);
	}
	/* 0100-0xFDFF */
	if (read(fd, ram, 0xFD00) < 4) {
		fprintf(stderr, "emu6800: bad test.\n");
		perror(argv[2]);
		exit(1);
	}
	close(fd);

	/* Run from 0x100 */
	ram[0xFFFE] = 0x01;
	ram[0xFFFF] = 0x00;
	switch(atoi(argv[1])) {
	case 6303:
		m6800_reset(&cpu, CPU_6303, INTIO_6803, 3);
		break;
	case 6800:
		m6800_reset(&cpu, CPU_6800, INTIO_NONE, 3);
		break;
	case 6803:
		m6800_reset(&cpu, CPU_6803, INTIO_6803, 3);
		break;
	case 6811:
		/* Run from 32K for 68HC11 */
		ram[0xFFFE] = 0x80;
		m68hc11a_reset(&cpu, 0, 0, NULL, NULL);
		if (debug)
			cpu.debug = 1;
		while(1)
			m68hc11_execute(&cpu);
		break;
	default:
		fprintf(stderr, "Unknown cpu type '%s'\n", argv[1]);
		exit(1);
	}
	if (debug)
		cpu.debug = 1;

	while (1)
		m6800_execute(&cpu);
}
