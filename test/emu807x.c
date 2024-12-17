#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ns807x.h"

static uint8_t ram[65536];

struct ns8070 *cpu;
unsigned long cycles;

uint8_t mem_read(struct ns8070 *cpu, uint16_t addr)
{
	return ram[addr];
}

static uint8_t fefcval=0;

void mem_write(struct ns8070 *cpu, uint16_t addr, uint8_t val)
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
		x=  (fefcval << 8) | val;
		if (x >= 0x8000)
			x-= 0x10000;
		printf("%d\n", x);
		break;
	    case 0xFEFC:
		fefcval= val;	/* Save high byte for now */
		break;
	    case 0xFEFB:
	    	printf("CPU cycles = %lu\n", cycles);
	    	break;
	    default:
		ram[addr & 0xFFFF] = val;
	}
}

void flag_change(struct ns8070 *cpu, uint8_t fbits)
{
}

int main(int argc, char *argv[])
{
	int fd;
	unsigned debug = 0;

	if (argc == 4 && strcmp(argv[1], "-d") == 0) {
		debug = 1;
		argv++;
		argc--;
	}
	if (argc != 3) {
		fprintf(stderr, "emu8070: test map.\n");
		exit(1);
	}
	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror(argv[1]);
		exit(1);
	}
	/* 0000-0xFDFF */
	if (read(fd, ram, 0xFE00) < 4) {
		fprintf(stderr, "emu8070: bad test.\n");
		perror(argv[2]);
		exit(1);
	}
	close(fd);

	/* Will begin execution at address 1 */
	cpu = ns8070_create(NULL);
	ns8070_trace(cpu, debug);
	while (1)
		cycles += ns8070_execute_one(cpu);
}
