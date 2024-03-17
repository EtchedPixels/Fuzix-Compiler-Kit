/*
 *	Fake 6809 mini machine to run compiler tests
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "d6809.h"
#include "e6809.h"

static uint8_t ram[65536];
int log_6809 = 0;

unsigned char e6809_read8(unsigned addr)
{
	return ram[addr];
}

unsigned char e6809_read8_debug(unsigned addr)
{
	if (addr < 0xFE00 || addr >= 0xFF00)
		return ram[addr];
	else
		return 0xFF;
}

void e6809_write8(unsigned addr, unsigned char val)
{
	if (addr == 0xFEFF) {
		if (val == 0)
			exit(0);
		fprintf(stderr, "***FAIL %u\n", val);
		exit(1);
	}
	ram[addr] = val;
}

static const char *make_flags(uint8_t cc)
{
	static char buf[9];
	char *p = "EFHINZVC";
	char *d = buf;

	while (*p) {
		if (cc & 0x80)
			*d++ = *p;
		else
			*d++ = '-';
		cc <<= 1;
		p++;
	}
	*d = 0;
	return buf;
}

/* Called each new instruction issue */
void e6809_instruction(unsigned pc)
{
	char buf[80];
	struct reg6809 *r = e6809_get_regs();
	if (log_6809) {
		d6809_disassemble(buf, pc);
		fprintf(stderr, "%04X: %-16.16s | ", pc, buf);
		fprintf(stderr, "%s %02X:%02X %04X %04X %04X %04X\n", make_flags(r->cc), r->a, r->b, r->x, r->y, r->u, r->s);
	}
}

int main(int argc, char *argv[])
{
	int fd;

	if (argc == 4 && strcmp(argv[1], "-d") == 0) {
		argv++;
		argc--;
		log_6809 = 1;
	}
	if (argc != 3) {
		fprintf(stderr, "emu6809: test map.\n");
		exit(1);
	}
	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror(argv[1]);
		exit(1);
	}
	/* 0100-0xFDFF */
	if (read(fd, ram, 0xFC00) < 10) {
		fprintf(stderr, "emu6809: bad test.\n");
		perror(argv[1]);
		exit(1);
	}
	close(fd);

	ram[0xFFFE] = 0x01;
	ram[0xFFFF] = 0x00;

	e6809_reset(log_6809);
	while (1)
		e6809_sstep(0, 0);
	return 0;
}
