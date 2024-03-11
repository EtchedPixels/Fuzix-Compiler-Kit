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

#include "6502.h"

static uint8_t ram[65536];

uint8_t read6502(uint16_t addr)
{
    return ram[addr];
}

uint8_t read6502_debug(uint16_t addr)
{
    return ram[addr];
}

void write6502(uint16_t addr, uint8_t val)
{
    switch(addr) {
    case 0xFEFD:
        if (val < 32 || val > 127)
            printf("\\x%02X", val);
        else
            putchar(val);
        fflush(stdout);
        return;
    case 0xFFFF:	/* Exit cleanly on write to $FFFF */
        exit(0);
    case 0xFEFF:
        if (val)
            fprintf(stderr, "***FAIL %d\n", val);
        exit(val);
    default:
        ram[addr] = val;
        return;
    }
}

int main(int argc, char *argv[])
{
    int fd;
    if (argc == 4 && strcmp(argv[1], "-d") == 0) {
        argv++;
        argc--;
        log_6502 = 1;
    }
    if (argc != 3) {
        fprintf(stderr, "emu6502: test map.\n");
        exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    /* 0200-0xFDFF */
    if (read(fd, ram, 0xFC00) < 520) {
        fprintf(stderr, "emu6502: bad test.\n");
        perror(argv[1]);
        exit(1);
    }
    close(fd);

    /* Run from 0x200 */
    ram[0xFFFC] = 0x00;
    ram[0xFFFD] = 0x02;

    disassembler_init(argv[2]);
    init6502();
    reset6502();

    while(1)
        exec6502(100000);
}
