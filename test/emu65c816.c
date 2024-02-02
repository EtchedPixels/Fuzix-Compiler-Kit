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

#include <lib65816/cpu.h>
#include <lib65816/cpuevent.h>

static uint8_t ram[65536];

uint8_t read65c816(uint32_t addr, uint8_t mode)
{
    if (addr & 0xFFFF0000) {
        fprintf(stderr, "Read access outside of low 64K\n");
        exit(1);
    }
    return ram[addr];
}

void write65c816(uint32_t addr, uint8_t val)
{
    if (addr & 0xFFFF0000) {
        fprintf(stderr, "Read access outside of low 64K\n");
        exit(1);
    }
    switch(addr) {
    case 0xFEFD:
        if (val < 32 || val > 127)
            printf("\\x%02X", val);
        else
            putchar(val);
        fflush(stdout);
        return;
    case 0xFEFF:
        if (val)
            fprintf(stderr, "***FAIL %d\n", val);
        exit(val);
    default:
        ram[addr] = val;
        return;
    }
}

void wdm(void)
{
}

void system_process(void)
{
}


int main(int argc, char *argv[])
{
    int fd;
    if (argc == 4 && strcmp(argv[1], "-d") == 0) {
        argv++;
        argc--;
        CPU_setTrace(1);
    }
    if (argc != 3) {
        fprintf(stderr, "emu65c816: test map.\n");
        exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    /* 0200-0xFDFF for code proper */
    if (read(fd, ram, 0xFC00) < 520) {
        fprintf(stderr, "emu65c816: bad test.\n");
        perror(argv[1]);
        exit(1);
    }
    close(fd);

    /* Run from 0x200 */
    ram[0xFFFC] = 0x00;
    ram[0xFFFD] = 0x02;

    CPUEvent_initialize();
    CPU_setUpdatePeriod(100000);
    CPU_reset();
    CPU_run();
}
