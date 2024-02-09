/*
 *	Fake mini machine to run compiler tests
 *
 *	For the 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "z8.h"

static uint8_t ram[65536];

uint8_t z8_read_data(struct z8 *cpu, uint16_t addr)
{
    fprintf(stderr, "R%04X->%02X\n", addr, ram[addr]);
    return ram[addr];
}

uint8_t z8_read_code(struct z8 *cpu, uint16_t addr)
{
    return ram[addr];
}

uint8_t z8_read_code_debug(struct z8 *cpu, uint16_t addr)
{
    return ram[addr];
}

void z8_write_data(struct z8 *cpu, uint16_t addr, uint8_t val)
{
    fprintf(stderr, "W%04X<-%02X\n", addr, val);
    switch(addr) {
    case 0xFFFE:
        if (val < 32 || val > 127)
            printf("\\x%02X", val);
        else
            putchar(val);
        fflush(stdout);
        return;
    case 0xFFFF:
        if (val)
            fprintf(stderr, "***FAIL %d\n", val);
        exit(val);
    }
    ram[addr] = val;
}

void z8_tx(struct z8 *cpu, uint8_t ch)
{
}

void z8_port_write(struct z8 *z8, uint8_t port, uint8_t val)
{
}

uint8_t z8_port_read(struct z8 *z8, uint8_t port)
{
    return 0xFF;
}

void z8_write_code(struct z8 *cpu, uint16_t addr, uint8_t val)
{
    ram[addr] = val;
}

int main(int argc, char *argv[])
{
    struct z8 *cpu;
    unsigned log = 0;
    int fd;

    if (argc == 4 && strcmp(argv[1], "-d") == 0) {
        argv++;
        argc--;
        log = 1;
    }
    if (argc != 3) {
        fprintf(stderr, "emuz8: test map.\n");
        exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    /* 0000-0xFFFF */
    if (read(fd, ram, sizeof(ram)) < 520) {
        fprintf(stderr, "emuz8: bad test.\n");
        perror(argv[1]);
        exit(1);
    }
    close(fd);

    cpu = z8_create();
    z8_reset(cpu);
    z8_set_trace(cpu, log);
    while(1)
        z8_execute(cpu);
}
