/*
 *	Fake mini machine to run compiler tests
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "tms7k.h"

static uint8_t ram[65536];

uint8_t mem_read8(uint16_t addr)
{
    return ram[addr];
}

uint8_t mem_read8_debug(uint16_t addr)
{
    return ram[addr];
}

void mem_write8(uint16_t addr, uint8_t val)
{
    ram[addr] = val;
    if (addr == 0x1FF) {
        if (val) {
            fprintf(stderr, "*** %u\n", val);
            exit(1);
        }
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    int fd;
    if (argc == 4 && strcmp(argv[1], "-d") == 0) {
        argv++;
        argc--;
        tms7k_trace(stderr);
    }
    if (argc != 3) {
        fprintf(stderr, "emu7k: test map.\n");
        exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    if (read(fd, ram + 0x0200, sizeof(ram) - 0x0200) < 8) {
        fprintf(stderr, "emu7k: bad test.\n");
        perror(argv[1]);
        exit(1);
    }
    close(fd);
    ram[0xFFFE] = 0x02;
    ram[0xFFFF] = 0x00;
    tms7k_reset();
    while(1)
        tms7k_execute(100000);
}
