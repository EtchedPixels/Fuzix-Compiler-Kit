/*
 *	Fake mini machine to run compiler tests
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "intel_8085_emulator.h"

static uint8_t ram[65536];

uint8_t i8085_read(uint16_t addr)
{
    return ram[addr];
}

uint8_t i8085_debug_read(uint16_t addr)
{
    return ram[addr];
}

void i8085_write(uint16_t addr, uint8_t val)
{
    ram[addr] = val;
}

uint8_t i8085_inport(uint8_t port)
{
    return 0xFF;
}

void i8085_outport(uint8_t port, uint8_t value)
{
    switch(port) {
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

int i8085_get_input(void)
{
    return 0xFF;
}

void i8085_set_output(int value)
{
}

int main(int argc, char *argv[])
{
    int fd;
    if (argc == 4 && strcmp(argv[1], "-d") == 0) {
        argv++;
        argc--;
        i8085_log = stderr;
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
        fprintf(stderr, "emu85: bad test.\n");
        perror(argv[1]);
        exit(1);
    }
    close(fd);
    i8085_load_symbols(argv[2]);
    i8085_reset(0);
    while(1)
        i8085_exec(100000);
}
